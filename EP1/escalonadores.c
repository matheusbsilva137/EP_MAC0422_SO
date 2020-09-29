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

typedef struct celulaProcesso{
    char* nome;
    int t0, dt, deadline;
    double tempoRestante;
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

int main(int argc, char* argv[]){
    FILE* trace, *saida;
    int t = 1, tipoEscalonador = atoi(argv[1]);
    char* nomeR;
    int t0R, dtR, deadlineR, quantLidos, estaEscalonando = 1;

    processo* filaEscalonador = malloc(sizeof(processo));
    filaEscalonador->prox = NULL;

    trace = fopen(argv[2], "r");
    saida = fopen(argv[3], "w");

    quantLidos = fscanf(trace, "%s %d %d %d", nomeR, t0R, dtR, deadlineR);

    while (estaEscalonando == 1){
        if (quantLidos == 4 && t0R == t){
            do{

            }while (quantLidos = fscanf(trace, "%s %d %d %d", nomeR, t0R, dtR, deadlineR) && t0R == t);
        }

        //estaEscalonando = está lendo ou fila do escalonador não vazia
        t++;
    }
}