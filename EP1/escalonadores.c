#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <sys/stat.h>

double tempoExecucao;

typedef struct celulaProcesso{
    char* nome;
    int t0, dt, deadline, executando;
    double tempoRestante;
    pthread_t tid;
    pthread_mutex_t mutex;
    processo* prox;
} processo;

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
    novo->nome = nome;
    novo->t0 = t0;
    novo->dt = dt;
    novo->tempoRestante = dt;
    novo->deadline = deadline;
    novo->executando = 0;
    novo->prox = NULL;
    return novo;
}

int estaVazia(processo* cabeca){
    return (cabeca->prox == NULL);
}

void* Thread(void *proc){
    processo* p = (processo*)proc;
    int i = 0;
    for(i = 0; i < p->dt; i += tempoExecucao){
        pthread_mutex_lock(&(p->mutex));
        //conta
        sleep(tempoExecucao);
        pthread_mutex_unlock(&(p->mutex));
    }
}

int main(int argc, char* argv[]){
    FILE* trace, *saida;
    int t = 1, tipoEscalonador = atoi(argv[1]);
    char* nomeR;
    int t0R, dtR, deadlineR, quantLidos, estaEscalonando = 1;

    processo* filaEscalonador = malloc(sizeof(processo)); 
    filaEscalonador->prox = NULL;
    processo* ultimo = filaEscalonador;
    processo* processoEmExecucao = NULL;

    trace = fopen(argv[2], "r");
    saida = fopen(argv[3], "w");

    quantLidos = fscanf(trace, "%s %d %d %d", nomeR, t0R, dtR, deadlineR);

    while (estaEscalonando == 1){
        if (quantLidos == 4 && t0R == t){
            do{
                if(argv[1] == 2){
                    //ver aonde colocar na fila
                }
                else{
                    processo* novo = criarNovoProcesso(nomeR, t0R, dtR, deadlineR);
                    inserirDepois(ultimo, novo);
                    ultimo = novo;
                }
            }while (quantLidos = fscanf(trace, "%s %d %d %d", nomeR, t0R, dtR, deadlineR) && t0R == t);
        }
        if(processoEmExecucao == NULL){
            processoEmExecucao = removerPrimeiro(filaEscalonador);
            if(processoEmExecucao != NULL){
                pthread_create(&(processoEmExecucao->tid), NULL,Thread, (void*)processoEmExecucao);
                pthread_mutex_unlock(&(processoEmExecucao->mutex));
                pthread_mutex_lock(&(processoEmExecucao->mutex));
            }
        }
        else{
            
            pthread_mutex_unlock(&(processoEmExecucao->mutex));
            pthread_mutex_lock(&(processoEmExecucao->mutex));
        }


        //estaEscalonando = está lendo ou fila do escalonador não vazia
        t++;
    }
}