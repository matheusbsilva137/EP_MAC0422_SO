#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "escalonador.h"

int main(int argc, char* argv[]){
    FILE* trace;
    int tipoEscalonador = atoi(argv[1]);
    char nomeR[30];
    int t0R, dtR, deadlineR, quantLidos, estaEscalonando = 1;
    int processadorLivre = 0;

    if (tipoEscalonador == 3) tempoExecucao = 0.8; 
    else tempoExecucao = 1;

    processo* filaEscalonador = malloc(sizeof(processo)); 
    filaEscalonador->prox = NULL;
    ultimo = filaEscalonador;
    processo* novaExecucao;

    trace = fopen(argv[2], "r");
    saida = fopen(argv[3], "w");

    opcao_d = (argc == 5 && strcmp(argv[4], "d") == 0);

    quantLidos = fscanf(trace, "%s%d%d%d", nomeR, &t0R, &dtR, &deadlineR);

    while (estaEscalonando == 1){
        novaExecucao = NULL;
        if (quantLidos == 4 && t0R == t){
            do{
                if (opcao_d) fprintf(stderr, "Entrada: ['%s'] - %d; %d; %d\n", nomeR, t0R, dtR, deadlineR);
                processo* novo = criarNovoProcesso(nomeR, t0R, dtR, deadlineR);
                
                if (processoEmExecucao == NULL){
                    processadorLivre = 1;
                    processoEmExecucao = filaEscalonador->prox;
                }

                if((processoEmExecucao != NULL && tipoEscalonador == 2 && dtR < processoEmExecucao->tempoRestante)
                    && (novaExecucao == NULL || dtR < novaExecucao->tempoRestante)){
                    
                    if (novaExecucao != NULL){
                        inserirDepois(ultimo, novaExecucao);
                        ultimo = novaExecucao;
                    }
                    novaExecucao = novo;
                }else{
                    inserirDepois(ultimo, novo);
                    ultimo = novo;
                }
                
                processo* a;
                quantLidos = fscanf(trace, "%s %d %d %d", nomeR, &t0R, &dtR, &deadlineR);
            }while (quantLidos == 4 && t0R == t);
        }

        if ( processadorLivre == 1 ){
            processadorLivre = 0;
            processoEmExecucao = NULL;
        }

        if (tipoEscalonador == 3){
            novaExecucao = removerPrimeiro(filaEscalonador);
            if (filaEscalonador->prox == NULL) ultimo = filaEscalonador;
            atualizarExecucao(filaEscalonador, novaExecucao, 3);
        }else atualizarExecucao(filaEscalonador, novaExecucao, tipoEscalonador);

        estaEscalonando = (quantLidos == 4 || filaEscalonador->prox != NULL || processoEmExecucao != NULL);
        t++;
    }
    fprintf(saida, "%d", mudContexto);
    if(opcao_d) fprintf(stderr, "%d\n", mudContexto);
    fclose(saida);
    fclose(trace);

    return 0;
}