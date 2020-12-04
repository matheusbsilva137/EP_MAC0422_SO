#include <sched.h>
#include <iostream>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <vector>
#include <sys/stat.h>
#include <time.h>

using namespace std;
vector<bool> bitmap;
vector<int> fat;

string formatarData(int dia, int mes, int ano, int hora, int min){
    return (to_string(dia)+" "+to_string(mes)+" "+to_string(ano)+" "+to_string(hora)+" "+to_string(min));
}

/*
FLAG = 1: retorna a data de modificação do arquivo de nome arq;
FLAG = 2: retorna a data do último acesso do arquivo de nome arq;
FLAG = 3: retorna a data de criação do arquivo de nome arq;
*/
string obterData(char* nomeArq, int FLAG){
    struct tm *timeArq;
    struct stat attrib;

    stat(nomeArq, &attrib);

    if (FLAG == 1) timeArq = gmtime(&(attrib.st_mtime));
    else if (FLAG == 2) timeArq = gmtime(&(attrib.st_atime));
    else timeArq = gmtime(&(attrib.st_ctime));

    return formatarData(timeArq->tm_mday, timeArq->tm_mon + 1, timeArq->tm_year + 1900, timeArq->tm_hour - 3, timeArq->tm_min);
}

int reservarEspacoDisco(){
    int i;
    for (i = 0; i < bitmap.size() && !bitmap[i]; i++);

    if (i >= bitmap.size()) return -1;
    bitmap[i] = 0;
    return i;
}

class Arquivo{
    public:
        string nome, dataModificacao, dataAcesso, dataCriacao;
        vector<string> blocos;
        int tamanho_bytes;

        Arquivo(string nomeDir, string dataModDir, string dataVisDir, string dataCriDir, int tam_bytes){
            nome = nomeDir;
            dataModificacao = dataModDir;
            dataAcesso = dataVisDir;
            dataCriacao = dataCriDir;
            tamanho_bytes = tam_bytes;
        }

        string obterMetadados(){
            return (nome+" "+to_string(tamanho_bytes)+" "+dataCriacao+" "+dataModificacao+" "+dataAcesso);
        }
};

class Diretorio{
    public:
        string nome, dataModificacao, dataAcesso, dataCriacao;

        //vetor de pares do tipo (primeiroBloco, ponteiro para o objeto do arquivo/diretório iniciado em primeiroBloco)
        vector<pair<int, Arquivo*>> filhosArq;
        vector<pair<int, Diretorio*>> filhosDir;

        Diretorio(string nomeDir, string dataModDir, string dataVisDir, string dataCriDir){
            nome = nomeDir;
            dataModificacao = dataModDir;
            dataAcesso = dataVisDir;
            dataCriacao = dataCriDir;
        }

        string obterMetadados(){
            return (nome+" "+dataCriacao+" "+dataModificacao+" "+dataAcesso);
        }
};
vector<Diretorio*> dirs;

void imprimirDiretorio(Diretorio* dir, int bloco, vector<string> &v){
    string s = "";

    for (int i = 0; i < dir->filhosDir.size(); i++){
        s += to_string(dir->filhosDir[i].first) + " " + dir->filhosDir[i].second->obterMetadados()+" ";
        imprimirDiretorio(dir->filhosDir[i].second, dir->filhosDir[i].first, v);
    }

    for (int i = 0; i < dir->filhosArq.size(); i++)
        s += to_string(dir->filhosArq[i].first) + " " + dir->filhosArq[i].second->obterMetadados()+" ";

    if (s.size() > 0) s.pop_back();
    s += "\n";

    v[bloco] = s;
}

int main(){
    FILE *arq, *arqUsr;
    bool sai = false;
    char c[] = "a";
    char* line, *command, *op1, *op2, *nomeArq, *dir;
    string prompt, livre, ocupado, nextLine, lineArq = "";

    prompt = "[ep3]:";
    livre = "1";
    ocupado = "0";
    nextLine = "\n";

    while (!sai){
        line = readline(prompt.c_str());
        add_history(line);
        
        command = strtok(line, " ");
        
        if (strcmp("mount", command) == 0){
            op1 =  strtok(NULL, " ");
            nomeArq = op1;

            if (access(op1, F_OK) != -1){
                //arquivo do sistema de arquivos já existe
                arq = fopen(op1, "r");
            }else{
                //arquivo do sistema de arquivos não existe                
                //inicializa o vetor bitmap
                arq = fopen(op1, "w");

                bitmap.assign(25000, true);
                for (int i = 0; i < 26; i++) bitmap[i] = false;

                //inicializa a tabela FAT
                fat.assign(25000, 0);
                for (int i = 0; i < 12; i++) fat[i] = i+1;
                fat[12] = -1;
                for (int i = 13; i < 50; i++) fat[i] = i+1;
                fat[50] = -1;

                Diretorio* dir = new Diretorio("/", obterData(nomeArq, 1), obterData(nomeArq, 2), obterData(nomeArq, 3));
                dirs.push_back(dir);
            }
        }else if(strcmp("cp", command) == 0){
            op1 =  strtok(NULL, " ");
            op2 =  strtok(NULL, " ");

            arqUsr = fopen(op1, "r");

            dir = strtok(op2, "/");
            Diretorio* dirDestino = dirs[0];
            while(dir){
                int i;
                for (i = 0; i < dirDestino->filhosDir.size() && strcmp(dirDestino->filhosDir[i].second->nome.c_str(), dir) != 0; i++);
                dirDestino = dirDestino->filhosDir[i].second;
                dir = strtok(NULL, "/");
            }

            //printf("Tamanho em bytes: %d\n", attrib.st_size);
        }else if(strcmp("mkdir", command) == 0){
            op1 =  strtok(NULL, " ");

            dir = strtok(op1, "/");

            string nomeDir;
            Diretorio* dirDestino = dirs[0];
            while(dir){
                int i;
                for (i = 0; i < dirDestino->filhosDir.size() && strcmp(dirDestino->filhosDir[i].second->nome.c_str(), dir) != 0; i++);
                
                if (i >= dirDestino->filhosDir.size()){
                    string str(dir);
                    nomeDir = str;
                    dir = NULL;
                }else{
                    dirDestino = dirDestino->filhosDir[i].second;
                    dir = strtok(NULL, "/");
                }
            }

            time_t agora = time(NULL);
            tm *localtm = localtime(&agora);

            string data = formatarData(localtm->tm_mday, 1+localtm->tm_mon, 1900+localtm->tm_year, localtm->tm_hour, localtm->tm_min);

            Diretorio* novoDir = new Diretorio(nomeDir, data, data, data);
            dirDestino->filhosDir.push_back(make_pair(reservarEspacoDisco(), novoDir));
        }else if(strcmp("rmdir", command) == 0){

        }else if(strcmp("cat", command) == 0){

        }else if(strcmp("touch", command) == 0){

        }else if(strcmp("rm", command) == 0){

        }else if(strcmp("ls", command) == 0){

        }else if(strcmp("find", command) == 0){

        }else if(strcmp("df", command) == 0){

        }else if(strcmp("umount", command) == 0){
            vector<string> blocosDisco;
            blocosDisco.assign(25000, "\n");
            int blocoAtual = 0;

            //escreve o bitmap
            for(int i = 0; i < 25000; i++){
                int j;
                
                for(j = i; j - i < 2000 && j < 25000; j++)
                    if (bitmap[j]) lineArq += livre;
                    else lineArq += ocupado;

                blocosDisco[blocoAtual++] = lineArq+"\n";

                lineArq = "";
                i = j - 1;
            }

            //escreve a tabela FAT
            for(int i = 0; i < 25000; i++){
                int j;
                
                for(j = i; j - i < 333 && j < 25000; j++)
                    lineArq += to_string(fat[j])+" ";

                lineArq.pop_back();
                blocosDisco[blocoAtual++] = lineArq+"\n";

                lineArq = "";
                i = j - 1;
                fflush(arq);
            }

            string dirRaiz = dirs[0]->obterMetadados();
            string s = dirRaiz + " ";
            for (int i = 0; i < dirs[0]->filhosDir.size(); i++){
                s += to_string(dirs[0]->filhosDir[i].first) + " " + dirs[0]->filhosDir[i].second->obterMetadados()+" ";
                imprimirDiretorio(dirs[0]->filhosDir[i].second, dirs[0]->filhosDir[i].first, blocosDisco);
            }

            for (int i = 0; i < dirs[0]->filhosArq.size(); i++)
                s += to_string(dirs[0]->filhosArq[i].first) + " " + dirs[0]->filhosArq[i].second->obterMetadados()+" ";

            s.pop_back();
            s += "\n";
            blocosDisco[blocoAtual++] = s;
            
            for(int i = 0; i < 25000; i++){
                fwrite(blocosDisco[i].c_str(), blocosDisco[i].length(), 1, arq);
                fflush(arq);
            }

            fclose(arq);
        }else if (strcmp("sai", command) == 0) sai = true;
    }
    fclose(arq);

    return 0;
}