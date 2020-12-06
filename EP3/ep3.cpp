#include <sched.h>
#include <iostream>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <vector>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include <chrono>

using namespace std;
vector<bool> bitmap;
vector<int> fat;
vector<string> blocosDisco;
const int BUFFER_SIZE = 4000;
int qDirs = 1, qArqs = 0;

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

        string obterListagem(){
            int diaCri, mesCri, anoCri, horaCri, minCri;
            int diaVis, mesVis, anoVis, horaVis, minVis;
            int diaMod, mesMod, anoMod, horaMod, minMod;
            sscanf(dataAcesso.c_str(), "%d %d %d %d %d", &diaVis, &mesVis, &anoVis, &horaVis, &minVis);
            sscanf(dataCriacao.c_str(), "%d %d %d %d %d", &diaCri, &mesCri, &anoCri, &horaCri, &minCri);
            sscanf(dataModificacao.c_str(), "%d %d %d %d %d", &diaMod, &mesMod, &anoMod, &horaMod, &minMod);

            string dataFormatAcesso = to_string(diaVis)+"/"+to_string(mesVis)+"/"+to_string(anoVis)+" "+to_string(horaVis)+":"+to_string(minVis);
            string dataFormatCriacao = to_string(diaCri)+"/"+to_string(mesCri)+"/"+to_string(anoCri)+" "+to_string(horaCri)+":"+to_string(minCri);
            string dataFormatModificacao = to_string(diaMod)+"/"+to_string(mesMod)+"/"+to_string(anoMod)+" "+to_string(horaMod)+":"+to_string(minMod);

            return ("A "+nome+" "+to_string(tamanho_bytes)+"KB C:"+dataFormatCriacao+" M:"+dataFormatModificacao+" A:"+dataFormatAcesso);
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
            if(filhosDir.size() > 0) filhosDir.clear();
            if(filhosArq.size() > 0) filhosArq.clear();
        }

        string obterMetadados(){
            return (nome+" "+dataCriacao+" "+dataModificacao+" "+dataAcesso);
        }

        string obterListagem(){
            int diaCri, mesCri, anoCri, horaCri, minCri;
            int diaVis, mesVis, anoVis, horaVis, minVis;
            int diaMod, mesMod, anoMod, horaMod, minMod;
            sscanf(dataAcesso.c_str(), "%d %d %d %d %d", &diaVis, &mesVis, &anoVis, &horaVis, &minVis);
            sscanf(dataCriacao.c_str(), "%d %d %d %d %d", &diaCri, &mesCri, &anoCri, &horaCri, &minCri);
            sscanf(dataModificacao.c_str(), "%d %d %d %d %d", &diaMod, &mesMod, &anoMod, &horaMod, &minMod);

            string dataFormatAcesso = to_string(diaVis)+"/"+to_string(mesVis)+"/"+to_string(anoVis)+" "+to_string(horaVis)+":"+to_string(minVis);
            string dataFormatCriacao = to_string(diaCri)+"/"+to_string(mesCri)+"/"+to_string(anoCri)+" "+to_string(horaCri)+":"+to_string(minCri);
            string dataFormatModificacao = to_string(diaMod)+"/"+to_string(mesMod)+"/"+to_string(anoMod)+" "+to_string(horaMod)+":"+to_string(minMod);

            return ("D "+nome+" C:"+dataFormatCriacao+" M:"+dataFormatModificacao+" A:"+dataFormatAcesso);
        }
};
Diretorio* diretorioRaiz;

void imprimirDiretorio(Diretorio* dir, int bloco, vector<string>& v){
    string s = "";

    for (int i = 0; i < dir->filhosDir.size(); i++){
        s += "D " + to_string(dir->filhosDir[i].first) + " " + dir->filhosDir[i].second->obterMetadados()+" ";
        imprimirDiretorio(dir->filhosDir[i].second, dir->filhosDir[i].first, v);
    }

    for (int i = 0; i < dir->filhosArq.size(); i++)
        s += "A " + to_string(dir->filhosArq[i].first) + " " + dir->filhosArq[i].second->obterMetadados()+" ";

    if (s.size() > 0) s.pop_back();
    s += "\n";

    v[bloco] = s;
}

//lê o conteúdo do arquivo arq do arquivo de texto do sistema de arquivos. O arquivo é iniciado no bloco b
void lerArquivo(Arquivo* arq, int b, char* nomeArqSA){
    int blocoAnt = 0;
    while (blocoAnt != -1){
        arq->blocos.push_back(b);
        blocosDisco[b] = lerBloco(b, nomeArqSA);

        b = fat[b];
        blocoAnt = b;        
    }
}

void lerDiretorio(Diretorio* dir, int b,  char* nomeArqSA){
    char* nomeDir = (char*) malloc(8*sizeof(char));
    int diaCri, mesCri, anoCri, horaCri, minCri;
    int diaVis, mesVis, anoVis, horaVis, minVis;
    int diaMod, mesMod, anoMod, horaMod, minMod;
    int tamArq;

    size_t len = 0;
    int qCharRead = 0, blocoInicio = 0;
    char tipo;
    char* line;

    FILE* arquivo = fopen(nomeArqSA, "r");
    for (int i = 0; i <= b; i++) getline(&line, &len, arquivo);
    if (line[0] == '\n') {
        free(nomeDir);
        fclose(arquivo);
        return;
    }
    
    while (sscanf(line, " %c%n", &tipo, &qCharRead) > 0){
        line += qCharRead;
        
        if (tipo == 'A'){
            //leitura dos dados do arquivo
            sscanf(line, "%d %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d%n", &blocoInicio, nomeDir, &tamArq,
            &diaCri, &mesCri, &anoCri, &horaCri, &minCri,
            &diaMod, &mesMod, &anoMod, &horaMod, &minMod,
            &diaVis, &mesVis, &anoVis, &horaVis, &minVis, &qCharRead);
            
            string nomeArquivo(nomeDir);
            Arquivo* arqNovo = new Arquivo(nomeArquivo, 
                                            formatarData(diaMod, mesMod, anoMod, horaMod, minMod), 
                                            formatarData(diaVis, mesVis, anoVis, horaVis, minVis), 
                                            formatarData(diaCri, mesCri, anoCri, horaCri, minCri), 
                                            tamArq);

            dir->filhosArq.push_back(make_pair(blocoInicio, arqNovo));

            qArqs++;
            lerArquivo(arqNovo, blocoInicio, nomeArqSA);
            line += qCharRead;
        }else{
            //leitura dos dados do diretório
            sscanf(line, "%d %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d%n", &blocoInicio, nomeDir,
            &diaCri, &mesCri, &anoCri, &horaCri, &minCri,
            &diaMod, &mesMod, &anoMod, &horaMod, &minMod,
            &diaVis, &mesVis, &anoVis, &horaVis, &minVis, &qCharRead);
            
            string nomeDiretorio(nomeDir);
            
            Diretorio* dirNovo = new Diretorio(nomeDiretorio,
                                                formatarData(diaMod, mesMod, anoMod, horaMod, minMod),
                                                formatarData(diaVis, mesVis, anoVis, horaVis, minVis),
                                                formatarData(diaCri, mesCri, anoCri, horaCri, minCri));
            
            qDirs++;
            dir->filhosDir.push_back(make_pair(blocoInicio, dirNovo));
            lerDiretorio(dirNovo, blocoInicio, nomeArqSA);
            line += qCharRead;
        }
    }

    free(nomeDir);
    fclose(arquivo);
}

//retorna a quantidade de blocos usada pelo diretório
int tamanhoDir(Diretorio* diretorio){
    int tam = 1;

    for (int i = 0; i < diretorio->filhosDir.size(); i++)
        tam += tamanhoDir(diretorio->filhosDir[i].second);

    for (int i = 0; i < diretorio->filhosArq.size(); i++)
        tam += diretorio->filhosArq[i].second->blocos.size();

    return tam;
}

//retorna o total de caracteres despedicados no diretorio e em seus arquivos/diretorios internos
int espacoDesperdicadoDir(Diretorio* diretorio){
    int total = (1999 - diretorioRaiz->obterMetadados().length() + 3);
    int totMetadados = 0;

    for (int i = 0; i < diretorio->filhosArq.size(); i++)
        total += 1999 - blocosDisco[diretorioRaiz->filhosArq[i].second->blocos.back()].size();
    
    for (int i = 0; i < diretorio->filhosDir.size(); i++){
        totMetadados += diretorio->filhosDir[i].second->obterMetadados().length() + 4;
        total += espacoDesperdicadoDir(diretorio->filhosDir[i].second);
    }

    if (diretorio == diretorioRaiz) totMetadados += diretorioRaiz->obterMetadados().length() + 1;
    totMetadados--;
    total += totMetadados%1999;

    return total;
}

int obterEspacoLivre(){
    int total = 100000;

    total -= 4*(ceil((double)bitmap.size()/1999.0));
    total -= 4*(ceil((double)fat.size()/1999.0));

    total -= 4*tamanhoDir(diretorioRaiz);
    return total;
}

double obterEspacoDesperdicado(){
    double total = 0.002*((bitmap.size()%1999) + fat.size()%1999 + espacoDesperdicadoDir(diretorioRaiz)%1999);
}

void removerArquivo(Arquivo *arq){
    int atual;
    for(int i = 0; i < arq->blocos.size(); i++){
        atual = arq->blocos[i];
        blocosDisco[atual] = "\n";
        bitmap[atual] = 1;
        fat[atual] = 0;
    }
}

void removeDiretorio(Diretorio *dir, int bloco, bool imprimir){
    for(int j = 0; j < dir->filhosDir.size(); j++){
        removeDiretorio(dir->filhosDir[j].second, dir->filhosDir[j].first, imprimir);
    }

    for(int j = 0; j < dir->filhosArq.size(); j++){
        if (imprimir) cout << "Arquivo " << dir->filhosArq[j].second->nome << " removido." << endl;
        removerArquivo(dir->filhosArq[j].second);
        dir->filhosArq[j].second->~Arquivo();
    }

    bitmap[bloco] = 1;
    fat[bloco] = 0;
    blocosDisco[bloco] = "\n";

    if (imprimir) cout << "Diretório " << dir->nome << " removido." << endl;
    dir->~Diretorio();
}

void findArquivo(Diretorio *dir, string arq, string s){
    for(int i = 0; i < dir->filhosArq.size(); i++){
        if(dir->filhosArq[i].second->nome == arq) cout << s + "/" + dir->filhosArq[i].second->nome << endl;
    }
    for(int i = 0; i < dir->filhosDir.size(); i++) findArquivo(dir->filhosDir[i].second, arq, s+"/"+dir->filhosDir[i].second->nome);
}

int main(){
    FILE *arq, *arqUsr, *arqTimeIns, *arqTimeRmv;
    bool sai = false;
    char c[] = "a";
    char* line, *command, *op1, *op2, *nomeArq, *dir;
    string prompt, livre, ocupado, nextLine, lineArq = "";
    
    prompt = "[ep3]:";
    livre = "1";
    ocupado = "0";
    nextLine = "\n";

    arqTimeIns = fopen("insere.txt", "w");
    arqTimeRmv = fopen("remove.txt", "w");

    while (!sai){
        line = readline(prompt.c_str());
        add_history(line);
        
        command = strtok(line, " ");
        if (strcmp("mount", command) == 0){
            op1 =  strtok(NULL, " ");
            blocosDisco.assign(25000, "\n");
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
                        num = strtok(NULL, " ");
                    }
                }

                //lê o diretório /
                Diretorio* raiz = new Diretorio("/", "", "", "");
                char* nomeDir = (char*) malloc(8*sizeof(char));
                int diaCri, mesCri, anoCri, horaCri, minCri;
                int diaVis, mesVis, anoVis, horaVis, minVis;
                int diaMod, mesMod, anoMod, horaMod, minMod;
                int tamArq;
                diretorioRaiz = raiz;

                len = 0;
                int qCharRead = 0, blocoInicio = 0;
                char tipo;
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

                //lê os diretórios dentro de /
                while (sscanf(line, " %c%n", &tipo, &qCharRead) != EOF){
                    line += qCharRead;

                    if (tipo == 'A'){
                        //leitura dos dados do arquivo
                        sscanf(line, "%d %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d%n", &blocoInicio, nomeDir, &tamArq,
                        &diaCri, &mesCri, &anoCri, &horaCri, &minCri,
                        &diaMod, &mesMod, &anoMod, &horaMod, &minMod,
                        &diaVis, &mesVis, &anoVis, &horaVis, &minVis, &qCharRead);
                        
                        string nomeArquivo(nomeDir);
                        Arquivo* arqNovo = new Arquivo(nomeArquivo, 
                                                       formatarData(diaMod, mesMod, anoMod, horaMod, minMod), 
                                                       formatarData(diaVis, mesVis, anoVis, horaVis, minVis), 
                                                       formatarData(diaCri, mesCri, anoCri, horaCri, minCri), 
                                                       tamArq);

                        raiz->filhosArq.push_back(make_pair(blocoInicio, arqNovo));
                        qArqs++;

                        lerArquivo(arqNovo, blocoInicio, nomeArq);
                        line += qCharRead;
                    }else{
                        //leitura dos dados do diretório
                        sscanf(line, "%d %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d%n", &blocoInicio, nomeDir,
                        &diaCri, &mesCri, &anoCri, &horaCri, &minCri,
                        &diaMod, &mesMod, &anoMod, &horaMod, &minMod,
                        &diaVis, &mesVis, &anoVis, &horaVis, &minVis, &qCharRead);
                        
                        string nomeDiretorio(nomeDir);
                        Diretorio* dirNovo = new Diretorio(nomeDiretorio,
                                                           formatarData(diaMod, mesMod, anoMod, horaMod, minMod),
                                                           formatarData(diaVis, mesVis, anoVis, horaVis, minVis),
                                                           formatarData(diaCri, mesCri, anoCri, horaCri, minCri));
                        
                        qDirs++;
                        raiz->filhosDir.push_back(make_pair(blocoInicio, dirNovo));
                        lerDiretorio(dirNovo, blocoInicio, nomeArq);
                        line += qCharRead;
                    }
                }

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

                diretorioRaiz = new Diretorio("/", obterData(nomeArq, 1), obterData(nomeArq, 2), obterData(nomeArq, 3));
            }
        }else if(strcmp("cp", command) == 0){
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            op1 =  strtok(NULL, " ");
            op2 =  strtok(NULL, " ");

            arqUsr = fopen(op1, "r");

            dir = strtok(op2, "/");
            Diretorio* dirDestino = diretorioRaiz;
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
            qArqs++;

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

            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            fprintf(arqTimeIns, "%d\n", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
        }else if(strcmp("mkdir", command) == 0){
            op1 =  strtok(NULL, " ");

            dir = strtok(op1, "/");

            string nomeDir;
            Diretorio* dirDestino = diretorioRaiz;
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

            qDirs++;
            Diretorio* novoDir = new Diretorio(nomeDir, data, data, data);
            dirDestino->filhosDir.push_back(make_pair(reservarEspacoDisco(), novoDir));
        }else if(strcmp("rmdir", command) == 0){
            op1 =  strtok(NULL, " ");

            dir = strtok(op1, "/");

            string nomeDir;
            Diretorio* dirDestino = diretorioRaiz, *pai = diretorioRaiz;
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

            removeDiretorio(dirDestino, bloco, true);
            pai->filhosDir.erase(pai->filhosDir.begin() + indPai);
        }else if(strcmp("cat", command) == 0){
            op1 =  strtok(NULL, " ");

            dir = strtok(op1, "/");

            string nomeArquivo;
            Diretorio* dirDestino = diretorioRaiz;
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
                    if (blocosDisco[dirDestino->filhosArq[j].second->blocos[i]].length() == 2000) s.pop_back();
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
            Diretorio* dirDestino = diretorioRaiz, *pai = diretorioRaiz;
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
            }else dirDestino->filhosArq[j].second->dataAcesso = data;
        }else if(strcmp("rm", command) == 0){
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            op1 =  strtok(NULL, " ");

            dir = strtok(op1, "/");

            string nomeArquivo;
            Diretorio* dirDestino = diretorioRaiz;
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

            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            fprintf(arqTimeRmv, "%d\n", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
        }else if(strcmp("ls", command) == 0){
            op1 =  strtok(NULL, " ");

            dir = strtok(op1, "/");

            string nomeDir;
            Diretorio* dirDestino = diretorioRaiz;
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

            for (int i = 0; i < dirDestino->filhosArq.size(); i++)
                cout << dirDestino->filhosArq[i].second->obterListagem() << endl;
            
            for (int i = 0; i < dirDestino->filhosDir.size(); i++)
                cout << dirDestino->filhosDir[i].second->obterListagem() << endl;
        }else if(strcmp("find", command) == 0){
            op1 =  strtok(NULL, " ");
            op2 = strtok(NULL, " ");

            dir = strtok(op1, "/");

            string nomeArq;
            Diretorio* dirDestino = diretorioRaiz;
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

            string str(op1);
            string arquivo(op2);

            printf("Arquivos encontrados:\n");
            if (str == "/") str = "";            
            findArquivo(dirDestino, arquivo, str);
        }else if(strcmp("df", command) == 0){
            cout << "Quantidade de Diretórios: " << qDirs << endl;
            cout << "Quantidade de Arquivos: " << qArqs << endl;
            cout << "Espaço livre: " << obterEspacoLivre() << "KB" << endl;
            cout << "Espaço desperdiçado: " << obterEspacoDesperdicado() << "KB" << endl;
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

            string dirRaiz = diretorioRaiz->obterMetadados();
            string s = dirRaiz + " ";
            for (int i = 0; i < diretorioRaiz->filhosDir.size(); i++){
                s += "D " + to_string(diretorioRaiz->filhosDir[i].first) + " " + diretorioRaiz->filhosDir[i].second->obterMetadados()+" ";
                imprimirDiretorio(diretorioRaiz->filhosDir[i].second, diretorioRaiz->filhosDir[i].first, blocosDisco);
            }

            for (int i = 0; i < diretorioRaiz->filhosArq.size(); i++)
                s += "A " + to_string(diretorioRaiz->filhosArq[i].first) + " " + diretorioRaiz->filhosArq[i].second->obterMetadados()+" ";

            s.pop_back();
            s += "\n";
            blocosDisco[blocoAtual++] = s;
            
            for(int i = 0; i < 25000; i++){
                fwrite(blocosDisco[i].c_str(), blocosDisco[i].length(), 1, arq);
                fflush(arq);
            }

            fclose(arq);
            
            //liberar os vetores, arquivos e diretórios
            removeDiretorio(diretorioRaiz, 89, false);
            blocosDisco.clear();
            bitmap.clear();
            fat.clear();
        }else if (strcmp("sai", command) == 0) sai = true;
    }

    fclose(arqTimeRmv);
    fclose(arqTimeIns);
    return 0;
}