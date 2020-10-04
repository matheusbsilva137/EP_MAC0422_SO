#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sched.h>

#define TOL 0.05

double tempoExecucao;
pthread_mutex_t sem;
FILE* saida;
int t = 1, mudContexto = 0, ultimaExecucao = -1;
int opcao_d, quantDeadlinesCump = 0;

typedef struct celulaProcesso{
    char* nome;
    int t0, deadline, executando;
    double tempoRestante, dt;
    pthread_t tid;
    pthread_mutex_t mutex;
    struct celulaProcesso* prox;
} processo;

processo* processoEmExecucao = NULL;
processo* ultimo;

/*
Recebe um ponteiro para um elemento da lista (elemento) e um ponteiro
para uma célula processo (novo) e insere novo após elemento na lista.
*/
void inserirDepois(processo* elemento, processo* novo){
    novo->prox = elemento->prox;
    elemento->prox = novo;
}

/*
Recebe a cabeça da lista ligada e retorna o primeiro elemento da lista,
removendo-o da lista.
*/
processo* removerPrimeiro(processo* cabeca){
    processo* prim = cabeca->prox;
    if (prim != NULL){
        cabeca->prox = prim->prox;
        prim->prox = NULL;
    }

    return prim;
}

processo* criarNovoProcesso(char *nome, int t0, int dt, int deadline){
    processo *novo = malloc(sizeof(processo));
    novo->nome = malloc(30*sizeof(char));
    strcpy(novo->nome, nome);
    novo->t0 = t0;
    novo->dt = dt;
    novo->tempoRestante = dt;
    novo->deadline = deadline;
    novo->prox = NULL;
    novo->tid = 0;
    pthread_mutex_init(&(novo->mutex), NULL);
    return novo;
}

int estaVazia(processo* cabeca){
    return (cabeca->prox == NULL);
}

void* Thread(void *proc){
    processo* p = (processo*)proc;
    double i = 0;
    int j, r;
    for(i = 0; p->tempoRestante > tempoExecucao - TOL; i += tempoExecucao){
        for (j = 1; j <= 100*tempoExecucao; j++){
            r *= j;
        }
        usleep((int) (tempoExecucao*1000));
        p->tempoRestante -= tempoExecucao;
        pthread_mutex_unlock(&(sem));
        pthread_mutex_lock(&(p->mutex));
    }
}

void atualizarExecucao(processo* filaEscalonador, processo* novaExecucao, int tipoEscalonador){
    if (novaExecucao != NULL){
        //Preempção
        if ( t - ultimaExecucao == 1 || processoEmExecucao != NULL) mudContexto++;
        if (processoEmExecucao != NULL){
            inserirDepois(ultimo, processoEmExecucao);
            ultimo = processoEmExecucao;
            if(opcao_d) fprintf(stderr, "O processo %s parou de usar a CPU %d\n", processoEmExecucao->nome, sched_getcpu());
        }else pthread_mutex_lock(&(sem));

        processoEmExecucao = novaExecucao;
        if(opcao_d) fprintf(stderr, "O processo %s comecou a usar a CPU %d\n", processoEmExecucao->nome, sched_getcpu());
        if (processoEmExecucao->tid == 0){
            pthread_mutex_unlock(&(sem));

            pthread_mutex_lock(&(processoEmExecucao->mutex));
            pthread_mutex_lock(&(sem));
            if (processoEmExecucao->tid == 0){
                pthread_create(&(processoEmExecucao->tid), NULL,Thread, (void*)processoEmExecucao);
            }
            pthread_mutex_lock(&(sem));
        }else novaExecucao = NULL;
    }
    
    if(processoEmExecucao == NULL){
        if ( (tipoEscalonador == 1 || tipoEscalonador == 3) || (tipoEscalonador == 2 && novaExecucao == NULL)){
            processoEmExecucao = removerPrimeiro(filaEscalonador);
            if(processoEmExecucao != NULL){
                if(opcao_d) fprintf(stderr, "O processo %s comecou a usar a CPU %d\n", processoEmExecucao->nome, sched_getcpu());
                if ( t - ultimaExecucao == 1 ) mudContexto++;
                if (processoEmExecucao->tid == 0)
                    pthread_mutex_lock(&(processoEmExecucao->mutex));
                pthread_mutex_lock(&(sem));
                if (processoEmExecucao->tid == 0)
                    pthread_create(&(processoEmExecucao->tid), NULL,Thread, (void*)processoEmExecucao);
                else 
                    pthread_mutex_unlock(&(processoEmExecucao->mutex));
                pthread_mutex_lock(&(sem));
            }
        }else{
            if ( t - ultimaExecucao == 1 ) mudContexto++;
            processoEmExecucao = novaExecucao;
            if(opcao_d) fprintf(stderr, "O processo %s comecou a usar a CPU %d\n", processoEmExecucao->nome, sched_getcpu());
            if (processoEmExecucao->tid == 0){
                pthread_create(&(processoEmExecucao->tid), NULL,Thread, (void*)processoEmExecucao);
            }
            pthread_mutex_unlock(&(processoEmExecucao->mutex));
            pthread_mutex_lock(&(processoEmExecucao->mutex));
        }
    }else if (novaExecucao == NULL){
        pthread_mutex_unlock(&(processoEmExecucao->mutex));

        if (processoEmExecucao->tempoRestante > 0.0)
            pthread_mutex_lock(&(sem));
    }

    if (processoEmExecucao != NULL && processoEmExecucao->tempoRestante <= tempoExecucao - TOL){
        ultimaExecucao = t;
        fprintf(saida, "%s %d %d\n", processoEmExecucao->nome, t + 1, t + 1 - processoEmExecucao->t0);

        if(opcao_d) fprintf(stderr, "Saída: %s %d %d\n", processoEmExecucao->nome, t + 1, t + 1 - processoEmExecucao->t0);
        if(opcao_d) fprintf(stderr, "O processo %s parou de usar a CPU %d\n", processoEmExecucao->nome, sched_getcpu());


        if (t+1 <= processoEmExecucao->deadline) quantDeadlinesCump++;
        pthread_mutex_unlock(&(processoEmExecucao->mutex));
        pthread_mutex_unlock(&(sem));
        pthread_join(processoEmExecucao->tid, NULL);
        pthread_mutex_destroy(&(processoEmExecucao->mutex));

        free(processoEmExecucao->nome);
        free(processoEmExecucao);

        processoEmExecucao = NULL;
    }
}

int main(int argc, char* argv[]){
    FILE* trace, *deadline, *mudancaContexto;
    int tipoEscalonador = atoi(argv[1]);
    char nomeR[30];
    int t0R, dtR, deadlineR, quantLidos, estaEscalonando = 1;
    int processadorLivre = 0;

    if (tipoEscalonador == 3) tempoExecucao = 0.8; 
    else tempoExecucao = 1;

    processo* filaEscalonador = malloc(sizeof(processo)); 
    filaEscalonador->prox = NULL;
    ultimo = filaEscalonador;
    processo* novaExecucao;

    trace = fopen(argv[2], "r");
    saida = fopen(argv[3], "w");
    if (tipoEscalonador == 1) {
        deadline = fopen("deadlines_esc1.txt", "a");
        mudancaContexto = fopen("contextos_esc1.txt", "a");
    }else if (tipoEscalonador == 2){
        deadline = fopen("deadlines_esc2.txt", "a");
        mudancaContexto = fopen("contextos_esc2.txt", "a");
    }else{
        deadline = fopen("deadlines_esc3.txt", "a");
        mudancaContexto = fopen("contextos_esc3.txt", "a");
    }

    opcao_d = (argc == 5 && strcmp(argv[4], "d") == 0);

    quantLidos = fscanf(trace, "%s%d%d%d", nomeR, &t0R, &dtR, &deadlineR);

    while (estaEscalonando == 1){
        novaExecucao = NULL;
        if (quantLidos == 4 && t0R == t){
            do{
                if (opcao_d) fprintf(stderr, "Entrada: ['%s'] - %d; %d; %d\n", nomeR, t0R, dtR, deadlineR);
                processo* novo = criarNovoProcesso(nomeR, t0R, dtR, deadlineR);
                
                if (processoEmExecucao == NULL){
                    processadorLivre = 1;
                    processoEmExecucao = filaEscalonador->prox;
                }

                if((processoEmExecucao != NULL && tipoEscalonador == 2 && dtR < processoEmExecucao->tempoRestante)
                    && (novaExecucao == NULL || dtR < novaExecucao->tempoRestante)){
                    
                    if (novaExecucao != NULL){
                        inserirDepois(ultimo, novaExecucao);
                        ultimo = novaExecucao;
                    }
                    novaExecucao = novo;
                }else{
                    inserirDepois(ultimo, novo);
                    ultimo = novo;
                }
                
                processo* a;
                quantLidos = fscanf(trace, "%s %d %d %d", nomeR, &t0R, &dtR, &deadlineR);
            }while (quantLidos == 4 && t0R == t);
        }

        if ( processadorLivre == 1 ){
            processadorLivre = 0;
            processoEmExecucao = NULL;
        }

        if (tipoEscalonador == 3){
            novaExecucao = removerPrimeiro(filaEscalonador);
            if (filaEscalonador->prox == NULL) ultimo = filaEscalonador;
            atualizarExecucao(filaEscalonador, novaExecucao, 3);
        }else atualizarExecucao(filaEscalonador, novaExecucao, tipoEscalonador);

        estaEscalonando = (quantLidos == 4 || filaEscalonador->prox != NULL || processoEmExecucao != NULL);
        t++;
    }
    fprintf(saida, "%d", mudContexto);
    fprintf(mudancaContexto, "%d\n", mudContexto);
    if(opcao_d) fprintf(stderr, "%d\n", mudContexto);
    fprintf(deadline, "%d\n", quantDeadlinesCump);
    fclose(saida);
    fclose(trace);
}