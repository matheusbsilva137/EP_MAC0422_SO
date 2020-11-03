#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "colocacao.h"
#define _USE_XOPEN2K

int d, n, intervaloSimulacao = 60, idAnterior = 0;
_Atomic int quantCiclistasAtivos;
int* primPistaVazia;
pthread_barrier_t barreiraInstante, barreiraSorteio;

typedef struct ciclista{
    int volta;
    int sprint;
    int velocidade;
    _Atomic int instante;
    int pistaCic;
    int ativo;
    double posCic;
    int id;
    
    pthread_t tid;
} ciclista;

typedef struct posicaoCircuito{
    ciclista* c;
    pthread_mutex_t semaforo;

} posicaoCircuito;
posicaoCircuito** pista;

double calcularNovaPosicao(ciclista* c){
    //adicionar módulo
    if (intervaloSimulacao == 60){
        if (c->velocidade == 30) return c->posCic + 0.5;
        else if (c->velocidade == 60) return c->posCic + 1.0;
        else c->posCic += 1.5;
    }else{
        if (c->velocidade == 30) return c->posCic + 0.5/3.0;
        else if (c->velocidade == 60) return c->posCic + 1.0/3.0;
        else return c->posCic + 0.5;
    }
}

/*
Verifica se o ciclista c consegue ultrapassar o cilista que está a sua frente.
A função retorna 0 se c não consegue fazer a ultrapassagem, ou o número da pista
mais interna em que c deve ficar após a ultrapassagem, caso consiga ultrapassar.
*/
int consegueUltrapassar(ciclista* c){
    int i = c->pistaCic, achei = 0;
    while (!achei && i < 10){
        for (i = c->pistaCic; i < 10 && pista[(int)c->posCic][i].c != NULL; i++);
        if (i < 10 && pthread_mutex_trylock(&(pista[(int)c->posCic][i].semaforo)) == 0){
            achei = 1;
            pthread_mutex_unlock(&(pista[(int)c->posCic][i].semaforo));
        } 
    }
    if (!achei) return 0;

    achei = 0;
    i = c->pistaCic;
    while (!achei && i < 10){
        for (i = c->pistaCic; i < 10 && pista[((int)c->posCic+1)%d][i].c != NULL; i++);
        if (i < 10 && pthread_mutex_trylock(&(pista[(int)c->posCic][i].semaforo)) == 0)
            achei = 1;
    }
    if (!achei) return 0;
    else return i;
}

void atualizarPosicao(ciclista* c){
    double novaPos = calcularNovaPosicao(c);
    
    if (abs((int)(floor(novaPos) - floor(c->posCic))) > 0){
        while (pista[((int)(c->posCic + 1))%d][c->pistaCic].c != NULL && pista[((int)(c->posCic + 1))%d][c->pistaCic].c->instante == c->instante)
            usleep(100);
        if (pista[((int)(c->posCic + 1))%d][c->pistaCic].c == NULL 
            && pthread_mutex_trylock(&(pista[((int)(c->posCic + 1))%d][c->pistaCic].semaforo)) == 0){
            
            pista[(int)(c->posCic)][c->pistaCic].c = NULL;
            if (novaPos >= d){
                novaPos -= d;
                c->volta += 1;
                int eliminado = classificarCiclista(c->id, c->volta);
                if (eliminado){
                    c->ativo = 0;
                    quantCiclistasAtivos--;
                    pista[c->posCic][c->pistaCic].c = NULL;
                    pthread_mutex_unlock(&(pista[c->posCic][c->pistaCic].semaforo));
                    return;
                }
            }

            int posAntiga = (int) c->posCic;
            c->posCic = novaPos;
            c->instante += 1;
            pista[(int)(c->posCic)][c->pistaCic].c = c;
            pthread_mutex_unlock(&(pista[posAntiga][c->pistaCic].semaforo));
            pista[posAntiga][c->pistaCic].c = NULL;
        }else{
            //tentativa de ultrapassagem
            int pistaUltrapassagem = consegueUltrapassar(c);
            if (pistaUltrapassagem != 0){
                pista[(int)(c->posCic)][c->pistaCic].c = NULL;
                if ((c->posCic + 1) >= d){
                    c->volta += 1;
                    int eliminado = classificarCiclista(c->id, c->volta);
                    if (eleiminado){
                        c->ativo = 0;
                        quantCiclistasAtivos--;
                        pista[c->posCic][c->pistaCic].c = NULL;
                        pthread_mutex_unlock(&(pista[c->posCic][c->pistaCic].semaforo));
                        return;
                    }
                }

                int posAntiga = (int) c->posCic;
                c->posCic = ((int)c->posCic + 1)%d;
                c->instante += 1;

                pista[posAntiga][c->pistaCic].c = NULL;
                pthread_mutex_unlock(&(pista[posAntiga][c->pistaCic].semaforo)); 
                c->pistaCic = pistaUltrapassagem;
                pista[(int)(c->posCic)][c->pistaCic].c = c;
            }else{
                int minVel = c->velocidade, i;
                for (i = ((int)(c->posCic+1))%d; minVel >= pista[i][c->pistaCic].c->velocidade && i%d != c->posCic && pista[i][c->pistaCic].c != NULL; i = (i+1)%d)
                    minVel = (minVel < pista[i][c->pistaCic].c->velocidade ? (minVel) : (pista[i][c->pistaCic].c->velocidade));

                while (i > c->posCic){
                    i--;
                    pista[i][c->pistaCic].c->velocidade = minVel;
                }

                double novaPos = calcularNovaPosicao(c);
    
                if (abs((int)(floor(novaPos) - floor(c->posCic))) > 0) c->instante += 1;
                else{
                    c->instante += 1;
                    c->posCic = novaPos;
                }                
            }
        }
    }else{
        c->instante += 1;
        c->posCic = novaPos;
    }
}

/*
 Sorteia e atualiza a velocidade de um ciclista
 considerando que este não está em alguma das duas últimas voltas.
*/
void atualizarVelocidade(ciclista* c){
    if (c->volta == 1 || c->velocidade == 90) return;

    int r = rand()%100;
    if (c->velocidade == 30){
        if (r <= 19) c->velocidade = 30;
        else c->velocidade = 60;
    }else{
        if (r <= 39) c->velocidade = 30;
        else c->velocidade = 60;
    }
    pthread_barrier_wait(&barreiraSorteio);
}

void* Thread(void* c){
    ciclista* cic = (ciclista*) c;

    while (cic->ativo){
        pthread_barrier_wait(&barreiraInstante);
        if (cic->volta % 6 == 0 && quantCiclistasAtivos > 5 && rand()%100 <= 4){
            quantCiclistasAtivos--;
            cic->ativo = 0;
            ajustarCiclistaQuebrado(cic->id, cic->volta);
            pista[cic->posCic][cic->pistaCic].c = NULL;
            pthread_mutex_unlock(&(pista[cic->posCic][cic->pistaCic]));
            printf("Ciclista %d quebrou na volta %d\n", cic->id, cic->volta);
            free(cic);
        }else{
            if (cic->volta > 1) atualizarVelocidade(cic);
            atualizarPosicao(cic);
        }
    }
}

void criarCiclista(){
    ciclista* c = malloc(sizeof(ciclista));
    c->volta = 1;
    c->sprint = 0;
    c->velocidade = 30;
    c->instante = 0;
    c->ativo = 1;
    c->id = ++idAnterior;

    int achouPosicao = 0, posCiclista, pistaCiclista, max = ((int) ceil(((double)n)/5.0));
    while (!achouPosicao){
        posCiclista = rand()%max;
        if (posCiclista % 2 == 0) pistaCiclista = primPistaVazia[posCiclista];
        else pistaCiclista =  5 + primPistaVazia[posCiclista];

        printf("%d %d\n", posCiclista, pistaCiclista);

        if ((pistaCiclista <= 4 || posCiclista%2 != 0)
            && (pistaCiclista <= 9 || posCiclista%2 != 1)
            && pthread_mutex_trylock(&(pista[posCiclista][pistaCiclista].semaforo)) == 0){
            
            primPistaVazia[posCiclista]++;
            c->pistaCic = pistaCiclista;
            c->posCic = posCiclista;
            achouPosicao = 1;
            pista[posCiclista][pistaCiclista].c = c;
            pthread_mutex_lock(&(pista[posCiclista][pistaCiclista].semaforo));

            pthread_create(&(c->tid), NULL, Thread, (void*) c);
        }
    }
}

void iniciarPista(int d, int n){
    
    srand(time(NULL));
    criarColocacoes(n, d);

    quantCiclistasAtivos = n;
    pthread_barrier_init(&(barreiraInstante), NULL, quantCiclistasAtivos);
    pthread_barrier_init(&(barreiraSorteio), NULL, quantCiclistasAtivos);

    primPistaVazia = malloc(d*sizeof(int));
    memset(primPistaVazia, 0, d*sizeof(int));

    pista = malloc(d*sizeof(posicaoCircuito *));
    for (int i = 0; i < d; i++){
        pista[i] = malloc(10*sizeof(posicaoCircuito));
        for (int j = 0; j < 10; j++) pthread_mutex_init(&(pista[i][j].semaforo), NULL);
    }

    for (int i = 0; i < n; i++)
        for (int j = 0; j < 10; j++)
            pista[i][j].c = NULL;

    for (int i = 0; i < n; i++)
        criarCiclista();

    for (int i = 0; i < n; i++)
        for (int j = 0; j < 10; j++)
            pthread_mutex_destroy(&(pista[i][j].semaforo));
}

void atualizarPista(){
    for (int t = 0; quantCiclistasAtivos > 1; t += 60){

    }

    imprimirColocacoes();
    finalizarColocacoes();
}