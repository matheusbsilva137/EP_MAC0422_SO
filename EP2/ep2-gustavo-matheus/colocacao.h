#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int* colocacoesFinais;
int** colocacoesCorrida;
int n, d, idVencedor;
pthread_mutex_t* semaforos;
pthread_mutex_t semFinal;
_Atomic int *a;
_Atomic int *b;
_Atomic int aFinal;
_Atomic int bFinal;
_Atomic int fimFila;
_Atomic int *filaEliminados, *filaEliminadosVoltas;
_Atomic int velocidade90 = 0;

/*
Recebe o número de uma volta e imprime suas colocações correspondente.
*/
void imprimirColocacoes(int volta){
    pthread_mutex_lock(&(semaforos[volta-1]));
    fprintf(stderr, " |------- VOLTA %03d ------|\n", volta);
    fprintf(stderr, " |  COLOCAÇÃO  | CICLISTA |\n");
    for(int i = 0; i < a[volta-1]; i++){
        fprintf(stderr, " |     %03d     |    %03d   |\n", i+1, colocacoesCorrida[volta-1][i]);
    }
    fprintf(stderr, "\n");
    pthread_mutex_unlock(&(semaforos[volta-1]));
}

void imprimirColocacoesFinais(){
    pthread_mutex_lock(&semFinal);

    fprintf(stderr, " |------ CLASSIFICAÇÃO FINAL ------|\n");
    fprintf(stderr, " |    COLOCAÇÃO    |   CICLISTA    |\n");
    for(int i = aFinal - 1; i >= 0; i--){
        fprintf(stderr, " |       %03d       |      %03d      |\n", aFinal-i, colocacoesFinais[i]);
    }
    for(int i = n-1; i > bFinal; i--){
        fprintf(stderr, " |    ELIMINADO    |      %03d      |\n", colocacoesFinais[i]);
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
    filaEliminadosVoltas = (_Atomic int*) malloc(n*sizeof(_Atomic int));
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

    for (int i = 0; i < n; i++){
        filaEliminados[i] = 0;
        filaEliminadosVoltas[i] = 0;
    }
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
    free(filaEliminadosVoltas);
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

    colocacoesCorrida[volta-1][a[volta-1]] = id;
    a[volta - 1] += 1;
    if (volta %2 != 0){
        if (a[volta-1] > b[volta-1]){
            pthread_mutex_unlock(&(semaforos[volta-1]));
            //imprimirColocacoes(volta);
        } else pthread_mutex_unlock(&(semaforos[volta-1]));
        return 0;
    }else if (a[volta-1] > b[volta-1]){
        pthread_mutex_lock(&semFinal);
        colocacoesFinais[aFinal] = id;
        aFinal++;

        srand(time(NULL));
        if((bFinal - aFinal) == 1 && (rand()%100) <= 10)
            velocidade90 = 1; 

        pthread_mutex_unlock(&semFinal);
        pthread_mutex_unlock(&(semaforos[volta-1]));
        //imprimirColocacoes(volta);
        return 1;
    }
    
    pthread_mutex_unlock(&(semaforos[volta-1]));
    return 0;
}

/*
A funão retorna 1 se o ciclista quebrado completa sua volta,
ou 0 caso contrário.
*/
int ajustarCiclistaQuebrado(int id, int volta){
    int colocacaoVazia = 0, res = 0;
    
    if (colocacoesCorrida[volta-1] == NULL){
        colocacoesCorrida[volta-1] = (int*) malloc((b[volta-2]+1 -((volta)%2))*sizeof(int));
        b[volta - 1] = b[volta-2]+1 -((volta)%2) - 1;
    }

    for(int i = volta-1; i < 2*(n-1) && !colocacaoVazia; i++){
        pthread_mutex_lock(&(semaforos[i]));

        if (colocacoesCorrida[i] == NULL) colocacaoVazia = 1;
        else{
            colocacoesCorrida[i][b[i]] = id;
            b[i] -= 1;

            if (a[i] > b[i]){
                res = 1;
            }

            pthread_mutex_lock(&semFinal);
            colocacoesFinais[bFinal] = id;
            bFinal--;
            pthread_mutex_unlock(&semFinal);
        }

        pthread_mutex_unlock(&(semaforos[i]));
    }
    return res;
}

/*
Recebe o id de um ciclista e retorna 1 se este deve ser eliminado
(ou 0, caso contrário).
*/
int verificarEliminacao(int id){
    for (int i = 0; i < fimFila; i++){
        if (filaEliminados[i] == id){
            
            //imprimirColocacoes(filaEliminadosVoltas[i]);
            return 1;
        }
    }
    return 0;
}

/*
Na última volta, o primeiro a passar pela linha de chegada é o vencedor,
que é mantido para que possa ser inserido na classificação final posteriormente.
*/
void guardarVencedor(int id){
    idVencedor = id;
}

/*
Insere o 1º colocado na classificação final
*/
void subidaAoPodio(){
    colocacoesFinais[aFinal++] = idVencedor;
}