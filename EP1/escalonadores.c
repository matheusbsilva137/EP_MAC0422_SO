#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <sys/stat.h>

double tempoExecucao;
pthread_mutex_t sem;

typedef struct celulaProcesso{
    char* nome;
    int t0, dt, deadline, executando;
    double tempoRestante;
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
    for(i = 0; i < p->dt; i += tempoExecucao){
        //conta
        printf("Dormiu\n");
        sleep(tempoExecucao);
        printf("Acordou\n");
        p->tempoRestante -= tempoExecucao;
        pthread_mutex_unlock(&(sem));
        pthread_mutex_lock(&(p->mutex));
    }
}

void atualizarExecucao(processo* filaEscalonador, processo* novaExecucao, int tipoEscalonador){
    if(processoEmExecucao == NULL){
        if ( (tipoEscalonador == 1 || tipoEscalonador == 3) || (tipoEscalonador == 2 && novaExecucao == NULL)){
            processoEmExecucao = removerPrimeiro(filaEscalonador);
            if(processoEmExecucao != NULL){
                printf("%s\n", processoEmExecucao->nome);
                pthread_mutex_lock(&(processoEmExecucao->mutex));
                pthread_mutex_lock(&(sem));
                if (processoEmExecucao->tid == 0)
                    pthread_create(&(processoEmExecucao->tid), NULL,Thread, (void*)processoEmExecucao);
                pthread_mutex_lock(&(sem));
            }
        }else{
            processoEmExecucao = novaExecucao;
            if (processoEmExecucao->tid == 0)
                pthread_create(&(processoEmExecucao->tid), NULL,Thread, (void*)processoEmExecucao);
            pthread_mutex_unlock(&(processoEmExecucao->mutex));
            pthread_mutex_lock(&(processoEmExecucao->mutex));
        }
    }else{
        printf("%s\n", processoEmExecucao->nome);
        pthread_mutex_unlock(&(processoEmExecucao->mutex));
        pthread_mutex_lock(&(sem));

        if (processoEmExecucao->tempoRestante <= 0){
            pthread_mutex_unlock(&(processoEmExecucao->mutex));
            pthread_mutex_unlock(&(sem));
            pthread_join(processoEmExecucao->tid, NULL);
            pthread_mutex_destroy(&(processoEmExecucao->mutex));

            free(processoEmExecucao->nome);
            free(processoEmExecucao);

            processoEmExecucao = NULL;
        }else if (novaExecucao != NULL){
            //Preempção
            inserirDepois(ultimo, processoEmExecucao);
            ultimo = processoEmExecucao;
            processoEmExecucao = novaExecucao;
        }
    }
}

int main(int argc, char* argv[]){
    FILE* trace, *saida;
    int t = 1, tipoEscalonador = atoi(argv[1]);
    char nomeR[30];
    int t0R, dtR, deadlineR, quantLidos, estaEscalonando = 1;

    tempoExecucao = 1;

    processo* filaEscalonador = malloc(sizeof(processo)); 
    filaEscalonador->prox = NULL;
    ultimo = filaEscalonador;
    processo* novaExecucao;

    trace = fopen(argv[2], "r");
    saida = fopen(argv[3], "w");

    quantLidos = fscanf(trace, "%s%d%d%d", nomeR, &t0R, &dtR, &deadlineR);
    printf("%s", nomeR);

    while (estaEscalonando == 1){
        novaExecucao = NULL;
        printf("OALAOLA %d\n", t);
        printf("Quantl lidos: %d\n", quantLidos);
        if (quantLidos == 4 && t0R == t){
            do{
                printf("OI\n");
                if(tipoEscalonador == 2){
                    //ver aonde colocar na fila

                }
                else{
                    processo* novo = criarNovoProcesso(nomeR, t0R, dtR, deadlineR);
                    inserirDepois(ultimo, novo);
                    ultimo = novo;
                    printf("ULTIMO %s\n", ultimo->nome);
                }
                processo* a;
                for (a = filaEscalonador->prox; a != NULL; a = a->prox)
                    printf("%s %d\n", a->nome, a->dt);
                printf("\n");
                quantLidos = fscanf(trace, "%s %d %d %d", nomeR, &t0R, &dtR, &deadlineR);
            }while (quantLidos == 4 && t0R == t);
        }

        atualizarExecucao(filaEscalonador, novaExecucao, tipoEscalonador);

        estaEscalonando = (quantLidos == 4 || filaEscalonador->prox != NULL || processoEmExecucao != NULL);
        t++;
    }
}