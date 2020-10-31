#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#define _USE_XOPEN2K

pthread_mutex_t** pista;
int d, n, quantCiclistasAtivos;
int* primPistaVazia;
pthread_barrier_t barreira;

typedef struct ciclista{
    int volta;
    int sprint;
    int velocidade;
    int instante;
    int pistaCic;
    double posCic;
    
    pthread_t tid;
} ciclista;

ciclista** ciclistas; 

void* Thread(void* barrier){
    printf("Chegou na barreira\n");
    pthread_barrier_wait(&(barreira));
    printf("Passou da barreira!\n");
}

ciclista* criarCiclista(){
    ciclista* c = malloc(sizeof(ciclista));
    c->volta = 0;
    c->sprint = 0;
    c->velocidade = 30;
    c->instante = 0;

    int achouPosicao = 0, posCiclista, pistaCiclista, max = ((int) ceil(((double)n)/5.0));
    while (!achouPosicao){
        posCiclista = rand()%max;
        if (posCiclista % 2 == 0) pistaCiclista = primPistaVazia[posCiclista];
        else pistaCiclista =  5 + primPistaVazia[posCiclista];

        printf("%d %d\n", posCiclista, pistaCiclista);

        if ((pistaCiclista <= 4 || posCiclista%2 != 0)
            && (pistaCiclista <= 9 || posCiclista%2 != 1)
            && pthread_mutex_trylock(&(pista[posCiclista][pistaCiclista])) == 0){
            
            primPistaVazia[posCiclista]++;
            c->pistaCic = pistaCiclista;
            c->posCic = posCiclista;
            achouPosicao = 1;

            pthread_create(&(c->tid), NULL, Thread, (void*) &(barreira));
            printf("Thread criada!\n");
        }
    }
    return c;
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
    pthread_barrier_init(&(barreira), NULL, quantCiclistasAtivos);

    primPistaVazia = malloc(d*sizeof(int));
    memset(primPistaVazia, 0, d*sizeof(int));

    pista = malloc(d*sizeof(pthread_mutex_t *));
    for (int i = 0; i < d; i++){
        pista[i] = malloc(10*sizeof(pthread_mutex_t));
        for (int j = 0; j < 10; j++) pthread_mutex_init(&(pista[i][j]), NULL);
    }

    ciclistas = malloc( (n+1)*sizeof(ciclista*));
    for (int i = 0; i < n; i++)
        ciclistas[i+1] = criarCiclista();

    for (int i = 0; i < n; i++)
        for (int j = 0; j < 10; j++)
            pthread_mutex_destroy(&(pista[i][j]));
}

void atualizarPista(){

}