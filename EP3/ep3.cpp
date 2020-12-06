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
vector<string> blocosDisco;
const int BUFFER_SIZE = 4000;

string formatarData(int dia, int mes, int ano, int hora, int min){
    return (to_string(dia)+" "+to_string(mes)+" "+to_string(ano)+" "+to_string(hora)+" "+to_string(min));
}

string lerBloco(int numBloco, char* nomeArqSA){
    FILE* arq = fopen(nomeArqSA, "r");
    size_t len = 0;
    char* line;

    while (numBloco--) getline(&line, &len, arq);

    getline(&line, &len, arq);
    string s(line);
    fclose(arq);
    return s;
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
    fat[i] = -1;
    return i;
}



class Arquivo{
    public:
        string nome, dataModificacao, dataAcesso, dataCriacao;
        vector<int> blocos;
        int tamanho_bytes;

        Arquivo(string nomeArq, string dataModArq, string dataVisArq, string dataCriArq, int tam_bytes){
            nome = nomeArq;
            dataModificacao = dataModArq;
            dataAcesso = dataVisArq;
            dataCriacao = dataCriArq;
            tamanho_bytes = tam_bytes;
        }

        ~Arquivo(){
            nome.clear();
            dataModificacao.clear();
            dataAcesso.clear();
            dataCriacao.clear();
            tamanho_bytes = 0;
            blocos.clear();
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

        ~Diretorio(){
            nome.clear();
            dataModificacao.clear();
            dataAcesso.clear();
            dataCriacao.clear();
            filhosDir.clear();
            filhosArq.clear();
        }

        string obterMetadados(){
            return (nome+" "+dataCriacao+" "+dataModificacao+" "+dataAcesso);
        }
};
vector<Diretorio*> dirs;


void imprimirDiretorio(Diretorio* dir, int bloco, vector<string>& v){
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

//lê o arquivo arq do arquivo de texto do sistema de arquivos. O arquivo é iniciado no bloco b
void lerArquivo(Arquivo* arq, int b, char* nomeArqSA){
    int blocoAnt = 0;
    while (blocoAnt != -1){
        arq->blocos.push_back(b);
        blocosDisco[b] = lerBloco(b, nomeArqSA);

        blocoAnt = b;
        if (b != -1) b = fat[b];
    }
}

void removerArquivo(Arquivo *arq){
    int atual;
    for(int i = 0; i < arq->blocos.size(); i++){
        atual = arq->blocos[i];
        blocosDisco[atual].clear();
        bitmap[atual] = 1;
        fat[atual] = 0;
    }
}

void removeDiretorio(Diretorio *dir, int bloco){
    for(int j = 0; j < dir->filhosDir.size(); j++) removeDiretorio(dir->filhosDir[j].second, dir->filhosDir[j].first);

    for(int j = 0; j < dir->filhosArq.size(); j++){
        removerArquivo(dir->filhosArq[j].second);
        dir->filhosArq[j].second->~Arquivo();
    }

    bitmap[bloco] = 1;
    fat[bloco] = 0;
    blocosDisco[bloco].clear();

    dir->~Diretorio();
}

void findArquivo(Diretorio *dir, string arq, string s){
    printf("Arquivos encontrados:\n");
    for(int i = 0; i < dir->filhosArq.size(); i++){
        if(dir->filhosArq[i].second->nome == arq) cout << s + "/" + dir->filhosArq[i].second->nome << endl;
    }
    for(int i = 0; i < dir->filhosDir.size(); i++) findArquivo(dir->filhosDir[i].second, arq, s+"/"+dir->filhosDir[i].second->nome);
}

void lerDiretorio(Diretorio* dir, int b,  char* nomeArqSA){
    for (int i = 0; i < dir->filhosDir.size(); i++){

    }

    for (int i = 0; i < dir->filhosArq.size(); i++){

    }
}



int main(){
    FILE *arq, *arqUsr;
    bool sai = false;
    char c[] = "a";
    char* line, *command, *op1, *op2, *nomeArq, *dir;
    string prompt, livre, ocupado, nextLine, lineArq = "";
    
    blocosDisco.assign(25000, "\n");
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

                //lê o bitmap (da linha 1 a 13)
                size_t len = 0;
                ssize_t read;
                
                for (int i = 0; i < 13; i++){
                    read = getline(&line, &len, arq);
                    for (int j = 0; j < read - 1; j++)
                        bitmap.push_back(line[j] - '0');
                }

                //lê a tabela FAT (da linha 14 a 89)
                char* num;
                for (int i = 0; i < 76; i++){
                    getline(&line, &len, arq);
                    num = strtok(line, " ");

                    while (num != NULL){
                        fat.push_back(atoi(num));
                        cout << num;
                        num = strtok(NULL, " ");
                    }
                    cout << endl;
                }

                //lê o diretório /
                Diretorio* raiz = new Diretorio("/", "", "", "");
                dirs.push_back(raiz);
                char* nomeDir = (char*) malloc(8*sizeof(char));
                int diaCri, mesCri, anoCri, horaCri, minCri;
                int diaVis, mesVis, anoVis, horaVis, minVis;
                int diaMod, mesMod, anoMod, horaMod, minMod;

                len = 0;
                int qCharRead = 0, blocoInicio = 0;
                getline(&line, &len, arq);

                sscanf(line, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d%n", nomeDir,
                        &diaCri, &mesCri, &anoCri, &horaCri, &minCri,
                        &diaMod, &mesMod, &anoMod, &horaMod, &minMod,
                        &diaVis, &mesVis, &anoVis, &horaVis, &minVis, &qCharRead);

                string nomeDiretorio(nomeDir);
                raiz->nome = nomeDiretorio;
                raiz->dataAcesso = formatarData(diaVis, mesVis, anoVis, horaVis, minVis);
                raiz->dataCriacao = formatarData(diaCri, mesCri, anoCri, horaCri, minCri);
                raiz->dataModificacao = formatarData(diaMod, mesMod, anoMod, horaMod, minMod);
                line += qCharRead;
                cout << "LEU: " << qCharRead << endl;

                //lê os diretórios dentro de /
                while (sscanf(line, "%d %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d%n", &blocoInicio, nomeDir,
                        &diaCri, &mesCri, &anoCri, &horaCri, &minCri,
                        &diaMod, &mesMod, &anoMod, &horaMod, &minMod,
                        &diaVis, &mesVis, &anoVis, &horaVis, &minVis, &qCharRead) != EOF){
                            string nomeDiretorio(nomeDir);
                            raiz->nome = nomeDiretorio;
                            raiz->dataAcesso = formatarData(diaVis, mesVis, anoVis, horaVis, minVis);
                            raiz->dataCriacao = formatarData(diaCri, mesCri, anoCri, horaCri, minCri);
                            raiz->dataModificacao = formatarData(diaMod, mesMod, anoMod, horaMod, minMod);

                            cout << "DADOS: " << raiz->obterMetadados() << endl;
                            line += qCharRead;
                        }

                // while(fscanf(arq, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", nomeDir,
                //         &diaCri, &mesCri, &anoCri, &horaCri, &minCri,
                //         &diaMod, &mesMod, &anoMod, &horaMod, &minMod,
                //         &diaVis, &mesVis, &anoVis, &horaVis, &minVis) != ){

                // }

                fclose(arq);
                arq = fopen(op1, "w");//liberar os vetores, arquivos e diretórios
            }else{
                //arquivo do sistema de arquivos não existe                
                //inicializa o vetor bitmap
                arq = fopen(op1, "w");

                bitmap.assign(25000, true);
                for (int i = 0; i < 90; i++) bitmap[i] = false;

                //inicializa a tabela FAT
                fat.assign(25000, 0);
                for (int i = 0; i < 12; i++) fat[i] = i+1;
                fat[12] = -1;
                for (int i = 13; i < 89; i++) fat[i] = i+1;
                fat[89] = -1;
                fat[90] = -1;

                Diretorio* dir = new Diretorio("/", obterData(nomeArq, 1), obterData(nomeArq, 2), obterData(nomeArq, 3));
                dirs.push_back(dir);
            }
        }else if(strcmp("cp", command) == 0){
            op1 =  strtok(NULL, " ");
            op2 =  strtok(NULL, " ");

            arqUsr = fopen(op1, "r");

            dir = strtok(op2, "/");
            Diretorio* dirDestino = dirs[0];
            string nomeArq;
            while(dir){
                int i;
                for (i = 0; i < dirDestino->filhosDir.size() && strcmp(dirDestino->filhosDir[i].second->nome.c_str(), dir) != 0; i++);
                
                if (i >= dirDestino->filhosDir.size()){
                    string str(dir);
                    nomeArq = str;
                    dir = NULL;
                }else{
                    dirDestino = dirDestino->filhosDir[i].second;
                    dir = strtok(NULL, "/");
                }
            }

            struct stat attrib;
            stat(op1, &attrib);

            //insere o conteúdo de todo o arquivo na string conteudoArq
            size_t len = 0;
            ssize_t read;
            int blocoAnterior = -1;
            Arquivo* novoArq = new Arquivo(nomeArq, obterData(op1, 1), obterData(op1, 2), obterData(op1, 3), attrib.st_size);

            while ((read = getline(&line, &len, arqUsr)) != -1) {
                string s(line);
                while (read > 2000 || (read == 2000 && line[1999] != '\n')){
                    int blocoNovo = reservarEspacoDisco();
                    novoArq->blocos.push_back(blocoNovo);

                    if (blocoAnterior != -1) fat[blocoAnterior] = blocoNovo;
                    blocoAnterior = blocoNovo;
                    
                    string textoBloco = s.substr(0, 1999) + "\n";
                    blocosDisco[blocoNovo] = textoBloco;
                    s = s.erase(0, 1999);
                    read -= 1999;
                }

                int blocoNovo = reservarEspacoDisco();
                novoArq->blocos.push_back(blocoNovo);
                if (blocoAnterior != -1) fat[blocoAnterior] = blocoNovo;
                blocoAnterior = blocoNovo;

                if (s.back() != '\n') s.push_back('\n');
                blocosDisco[blocoNovo] = s;
            }

            dirDestino->filhosArq.push_back(make_pair(novoArq->blocos[0], novoArq));
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
            op1 =  strtok(NULL, " ");

            dir = strtok(op1, "/");

            string nomeDir;
            Diretorio* dirDestino = dirs[0], *pai = dirs[0];
            int bloco = 0, indPai = 0;
            int i;
            while(dir){
                for (i = 0; i < dirDestino->filhosDir.size() && strcmp(dirDestino->filhosDir[i].second->nome.c_str(), dir) != 0; i++);
                
                if (i >= dirDestino->filhosDir.size()){
                    string str(dir);
                    nomeDir = str;
                    dir = NULL;
                }else{
                    pai = dirDestino;
                    indPai = i;
                    bloco = dirDestino->filhosDir[i].first;
                    dirDestino = dirDestino->filhosDir[i].second;
                    dir = strtok(NULL, "/");
                }
            }

            pai->filhosDir.erase(pai->filhosDir.begin() + i - 1);
            removeDiretorio(dirDestino, bloco);
        

        }else if(strcmp("cat", command) == 0){
            
            op1 =  strtok(NULL, " ");

            dir = strtok(op1, "/");

            string nomeArquivo;
            Diretorio* dirDestino = dirs[0];
            int i;
            while(dir){
                for (i = 0; i < dirDestino->filhosDir.size() && strcmp(dirDestino->filhosDir[i].second->nome.c_str(), dir) != 0; i++);
                
                if (i >= dirDestino->filhosDir.size()){
                    string str(dir);
                    nomeArquivo = str;
                    dir = NULL;
                }else{
                    dirDestino = dirDestino->filhosDir[i].second;
                    dir = strtok(NULL, "/");
                }
            }
            int j;
            for(j = 0; j < dirDestino->filhosArq.size() && strcmp(dirDestino->filhosArq[j].second->nome.c_str(), nomeArquivo.c_str()) != 0; j++);
            if(j >= dirDestino->filhosArq.size()) printf("Esse arquivo não existe!");
            else{
                string s;
                for(int i = 0; i < dirDestino->filhosArq[j].second->blocos.size(); i++){
                    s += blocosDisco[dirDestino->filhosArq[j].second->blocos[i]];
                }
                cout << s;

                time_t agora = time(NULL);
                tm *localtm = localtime(&agora);
                string data = formatarData(localtm->tm_mday, 1+localtm->tm_mon, 1900+localtm->tm_year, localtm->tm_hour, localtm->tm_min);
                dirDestino->filhosArq[j].second->dataAcesso = data;
            }



        }else if(strcmp("touch", command) == 0){
            op1 =  strtok(NULL, " ");

            dir = strtok(op1, "/");

            string nomeArquivo;
            Diretorio* dirDestino = dirs[0], *pai = dirs[0];
            int bloco = 0, indPai = 0;
            int i;
            while(dir){
                for (i = 0; i < dirDestino->filhosDir.size() && strcmp(dirDestino->filhosDir[i].second->nome.c_str(), dir) != 0; i++);
                
                if (i >= dirDestino->filhosDir.size()){
                    string str(dir);
                    nomeArquivo = str;
                    dir = NULL;
                }else{
                    pai = dirDestino;
                    indPai = i;
                    bloco = dirDestino->filhosDir[i].first;
                    dirDestino = dirDestino->filhosDir[i].second;
                    dir = strtok(NULL, "/");
                }
            }

            int j;
            time_t agora = time(NULL);
            tm *localtm = localtime(&agora);
            string data = formatarData(localtm->tm_mday, 1+localtm->tm_mon, 1900+localtm->tm_year, localtm->tm_hour, localtm->tm_min);

            for(j = 0; j < dirDestino->filhosArq.size() && (dirDestino->filhosArq[j].second->nome != nomeArquivo); j++);

            if(j >= dirDestino->filhosArq.size()){
                Arquivo *novoArquivo = new Arquivo(nomeArquivo, data, data, data, 0);
                int novoBloco = reservarEspacoDisco();
                novoArquivo->blocos.push_back(novoBloco);
                blocosDisco[novoBloco] = "\n";
                dirDestino->filhosArq.push_back(make_pair(novoBloco, novoArquivo));
            }else{
                dirDestino->filhosArq[j].second->dataAcesso = data;
            }

        }else if(strcmp("rm", command) == 0){
            op1 =  strtok(NULL, " ");

            dir = strtok(op1, "/");

            string nomeArquivo;
            Diretorio* dirDestino = dirs[0];
            int i;
            while(dir){
                for (i = 0; i < dirDestino->filhosDir.size() && strcmp(dirDestino->filhosDir[i].second->nome.c_str(), dir) != 0; i++);
                
                if (i >= dirDestino->filhosDir.size()){
                    string str(dir);
                    nomeArquivo = str;
                    dir = NULL;
                }else{
                    dirDestino = dirDestino->filhosDir[i].second;
                    dir = strtok(NULL, "/");
                }
            }

            int j;
            for(j = 0; j < dirDestino->filhosArq.size() && strcmp(dirDestino->filhosArq[j].second->nome.c_str(), nomeArquivo.c_str()) != 0; j++);
            if(j >= dirDestino->filhosArq.size()) printf("Esse arquivo não existe!");
            else{
                removerArquivo(dirDestino->filhosArq[j].second);
                dirDestino->filhosArq.erase(dirDestino->filhosArq.begin() + j - 1);
            }

            
        }else if(strcmp("ls", command) == 0){
            //fazer
            
        }else if(strcmp("find", command) == 0){
            op1 =  strtok(NULL, " ");
            op2 = strtok(NULL, " ");

            dir = strtok(op1, "/");

            string nomeArq;
            Diretorio* dirDestino = dirs[0];
            int i;
            while(dir){
                for (i = 0; i < dirDestino->filhosDir.size() && strcmp(dirDestino->filhosDir[i].second->nome.c_str(), dir) != 0; i++);
                
                if (i >= dirDestino->filhosDir.size()){
                    string str(dir);
                    nomeArq = str;
                    dir = NULL;
                }else{
                    dirDestino = dirDestino->filhosDir[i].second;
                    dir = strtok(NULL, "/");
                }
            }
            char *aux = strtok(op2, "/");
            char *arquivoAux;
            while(aux){
                arquivoAux = aux;
                aux = strtok(NULL, "/");
            }

            string str(op1);
            string arquivo(arquivoAux);
            
            findArquivo(dirDestino, arquivo, str);

        }else if(strcmp("df", command) == 0){

        }else if(strcmp("umount", command) == 0){
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
            //liberar os vetores, arquivos e diretórios
        }else if (strcmp("sai", command) == 0) sai = true;
    }
    fclose(arq);

    return 0;
}