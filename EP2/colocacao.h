#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int* colocacoesFinais;
int** colocacoesCorrida;
int n;
pthread_mutex_t* semaforos;
_Atomic int *a;
_Atomic int *b;

void criarColocacoes(int nCorrida, int dCorrida){
    colocacoesCorrida = (int**) malloc(2*(n-1)*sizeof(int*));
    for (int i = 0; i < 2*(n-1); i++) colocacoesCorrida[i] = NULL;

    colocacoesFinais = (int*) malloc(n*sizeof(_Atomic int));
    a = (_Atomic int*) malloc(2*(n-1)*sizeof(_Atomic int));
    b = (_Atomic int*) malloc(2*(n-1)*sizeof(_Atomic int));

    semaforos = (pthread_mutex_t*) malloc(2*(n-1)*sizeof(pthread_mutex_t));
    for (int i = 0; i < 2*(n-1); i++)
        pthread_mutex_init(&(semaforos[i]), NULL);
}

void finalizarColocacoes(){
    for (int i = 0; i < 2*(n-1); i++) 
        if (colocacoesCorrida[i] != NULL) free(colocacoesCorrida[i]);
    free(colocacoesCorrida);
    free(colocacoesFinais);
    for (int i = 0; i < 2*(n-1); i++)
        pthread_mutex_destroy(&(semaforos[i]));
    free(semaforos);
}

/*
Recebe o identificador de um ciclista e o número da volta
finalizada. Insere o ciclista nas classificações adequadas e
retorna 1 caso o ciclista tenha sido eliminado (ou 0, cc)
*/
int classificarCiclista(int id, int volta){
    pthread_mutex_lock(&(semaforos[volta-1]));

    if (colocacoesCorrida[volta-1] == NULL){
        if (volta % 2 == 0) colocacoesCorrida[volta-1] = (int*) malloc(n-volta/2*sizeof(int));
        else colocacoesCorrida = (int*) malloc(n*sizeof(int)) ;     
    }

    if (volta %2 != 0) {
        return 0;
    }

    pthread_mutex_unlock(&(semaforos[volta-1]));
}

//n-1-quantQuebradosAnterior
//semaforo antes de verificar se é null