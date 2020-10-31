#include "velodromo.h"

int main(int argc, char* argv[]){
    int t = 0, intervalo = 60;

    d = atoi(argv[1]);
    n = atoi(argv[2]);

    iniciarPista(d, n);

    // for (t = 0; quantCiclistasAtivos > 1; t += intervalo){
    //     atualizarPista();
    // }
    return 0;
}