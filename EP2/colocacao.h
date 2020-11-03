#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int* colocacoesFinais;
int** colocacoesCorrida;
int n, d, fimFila = 0;
pthread_mutex_t* semaforos;
pthread_mutex_t semFinal;
_Atomic int *a;
_Atomic int *b;
_Atomic int aFinal;
_Atomic int bFinal;
_Atomic int *filaEliminados;

/*
Recebe o número de uma volta e imprime suas colocações correspondente.
*/
void imprimirColocacoes(int volta){
    pthread_mutex_lock(&(semaforos[volta-1]));
    printf(" |------- VOLTA %03d ------|\n", volta);
    printf(" |  COLOCAÇÃO  | CICLISTA |\n");
    for(int i = 0; i < a[volta-1]; i++){
        printf(" |     %03d     |    %03d   |\n", i+1, colocacoesCorrida[volta-1][i]);
    }
    pthread_mutex_unlock(&(semaforos[volta-1]));
}

void imprimirColocacoesFinais(){
    pthread_mutex_lock(&semFinal);
    printf(" |------ CLASSIFICAÇÃO FINAL ------|\n");
    printf(" |    COLOCAÇÃO    |   CICLISTA    |\n");
    for(int i = aFinal - 1; i >= 0; i--){
        printf(" |       %03d       |      %03d      |\n", aFinal-i, colocacoesFinais[i]);
    }
    for(int i = n-1; i > bFinal; i--){
        printf(" |    ELIMINADO    |      %03d      |\n", colocacoesFinais[i]);
    }
    pthread_mutex_unlock(&semFinal);
}

void criarColocacoes(int nCorrida, int dCorrida){
    n = nCorrida;
    d = dCorrida;
    colocacoesCorrida = (int**) malloc(2*(n-1)*sizeof(int*));
    for (int i = 0; i < 2*(n-1); i++) colocacoesCorrida[i] = NULL;

    colocacoesFinais = (int*) malloc(n*sizeof(_Atomic int));
    filaEliminados = (_Atomic int*) malloc(n*sizeof(_Atomic int));
    a = (_Atomic int*) malloc(2*(n-1)*sizeof(_Atomic int));
    b = (_Atomic int*) malloc(2*(n-1)*sizeof(_Atomic int));

    aFinal = 0;
    bFinal = n-1;

    for (int i = 0; i < 2*(n-1); i++){
        a[i] = 0;
        b[i] = n-1;
    }

    semaforos = (pthread_mutex_t*) malloc(2*(n-1)*sizeof(pthread_mutex_t));
    for (int i = 0; i < 2*(n-1); i++)
        pthread_mutex_init(&(semaforos[i]), NULL);
    pthread_mutex_init(&(semFinal), NULL);

    for (int i = 0; i < n; i++)
        filaEliminados[i] = 0;
}

void finalizarColocacoes(){
    for (int i = 0; i < 2*(n-1); i++) 
        if (colocacoesCorrida[i] != NULL) free(colocacoesCorrida[i]);
    free(colocacoesCorrida);
    free(colocacoesFinais);
    for (int i = 0; i < 2*(n-1); i++)
        pthread_mutex_destroy(&(semaforos[i]));
    free(semaforos);
    free(a);
    free(b);
    free(filaEliminados);
}

/*
Recebe o identificador de um ciclista e o número da volta
finalizada. Insere o ciclista nas classificações adequadas e
retorna 1 caso o ciclista tenha sido eliminado (ou 0, cc)
*/
int classificarCiclista(int id, int volta){
    pthread_mutex_lock(&(semaforos[volta-1]));
    

    if (colocacoesCorrida[volta-1] == NULL){
        if (volta == 1) colocacoesCorrida[volta-1] = (int*) malloc(n*sizeof(int));
        else{
            colocacoesCorrida[volta-1] = (int*) malloc((b[volta-2]+1 -((volta)%2))*sizeof(int));
            b[volta - 1] = b[volta-2]+1 -((volta)%2) - 1;
        }
    }
    //printf("colocando id %d e %d e %d\n", volta-1, a[volta-1], n-((volta)%2-(n-b[volta-2]-1)));
    colocacoesCorrida[volta-1][a[volta-1]] = id;
    a[volta - 1] += 1;
    printf("BBB- %d e B2- %d\n", b[volta-2] + 1 - (volta)%2 - 1, b[volta-2]);
    printf("A- %d e B- %d e volta- %d\n", a[volta-1], b[volta-1], volta);
    if (volta %2 != 0){
        if (a[volta-1] > b[volta-1]){
            pthread_mutex_unlock(&(semaforos[volta-1]));
            imprimirColocacoes(volta);
        } else pthread_mutex_unlock(&(semaforos[volta-1]));
        return 0;
    } 
    else if (a[volta-1] > b[volta-1]){
        printf("eliminado!!!\n");
        pthread_mutex_lock(&semFinal);
        colocacoesFinais[aFinal] = id;
        aFinal++;
        pthread_mutex_unlock(&semFinal);
        pthread_mutex_unlock(&(semaforos[volta-1]));
        imprimirColocacoes(volta);
        return 1;
    }
    pthread_mutex_unlock(&(semaforos[volta-1]));
    return 0;
}

void ajustarCiclistaQuebrado(int id, int volta){
    int colocacaoVazia = 0;
    for(int i = volta-1; i < 2*(n-1) && !colocacaoVazia; i++){
        pthread_mutex_lock(&(semaforos[i]));

        if (colocacoesCorrida[i] == NULL) colocacaoVazia = 1;
        else{
            colocacoesCorrida[i][b[i]] = id;
            b[i] -= 1;

            if (a[i] > b[i]){
                filaEliminados[fimFila++] = colocacoesCorrida[i][a[i]-1];
                imprimirColocacoes(i+1);
            }
        }

        pthread_mutex_lock(&semFinal);
        colocacoesFinais[bFinal] = id;
        bFinal--;
        pthread_mutex_unlock(&semFinal);

        pthread_mutex_unlock(&(semaforos[i]));
    }
}

/*
Recebe o id de um ciclista e retorna 1 se este deve ser eliminado
(ou 0, caso contrário).
*/
int verificarEliminacao(int id){
    for (int i = 0; i < fimFila && filaEliminados[i] != 0; i++){
        if (filaEliminados[i] == id) return 1;
    }
    return 0;
}