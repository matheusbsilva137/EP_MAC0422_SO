#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

int** pista;
int d, n, quantCiclistasAtivos;
int* primPistaVazia;

typedef struct ciclista{
    int volta;
    int sprint;
    int velocidade;
    int instante;

    //thread
} ciclista;

ciclista** ciclistas; 

ciclista* criarCiclista(){
    ciclista* c = malloc(sizeof(ciclista));
    c->volta = 0;
    c->sprint = 0;
    c->velocidade = 30;
    
    //thread

}

void* Thread(){

}

/*
 Sorteia e atualiza a velocidade de um ciclista
 considerando que este não está em alguma das duas últimas voltas.
*/
void sortearVelocidade(ciclista* c){
    if (c->volta == 0 || c->velocidade == 90) return;

    int r = rand()%100;
    if (c->velocidade == 30){
        if (r <= 19) c->velocidade = 30;
        else c->velocidade = 60;
    }else{
        if (r <= 39) c->velocidade = 30;
        else c->velocidade = 60;
    }
}

void iniciarPista(int d, int n){
    srand(time(NULL));

    quantCiclistasAtivos = n;
    primPistaVazia = malloc(d*sizeof(int));
    memset(primPistaVazia, 0, d*sizeof(int));

    pista = malloc(d*sizeof(int*));
    for (int i = 0; i < d; i++){
        pista[i] = malloc(10*sizeof(int));
        memset(pista[i], 0, 10*sizeof(int));
    }

    ciclistas = malloc( (n+1)*sizeof(ciclista*));
    for (int i = 0; i < n; i++){
        ciclistas[i+1] = criarCiclista();

        int pos = rand()%d;
        while (primPistaVazia[pos] < 5) pos = rand()%d;
        pista[pos][primPistaVazia[pos]] = i+1;
        primPistaVazia[pos]++;
    }
}

void atualizarPista(){

}