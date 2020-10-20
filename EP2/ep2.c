#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

int** pista;
int d, n, quantCiclistasAtivos;

int main(int argc, char* argv[]){
    int t = 0, intervalo = 60;
    
    d = atoi(argv[1]);
    n = atoi(argv[2]);
    quantCiclistasAtivos = n;

    pista = malloc(d*sizeof(int*));
    for (int i = 0; i < d; i++){
        pista[i] = malloc(10*sizeof(int));
        memset(pista[i], 0, 10*sizeof(int));
    }   

    for (t = 0; quantCiclistasAtivos > 1; t += intervalo){

    }

    return 0;
}