#include "velodromo.h"

int main(int argc, char* argv[]){
    int t = 0, intervalo = 60;

    d = atoi(argv[1]);
    n = atoi(argv[2]);

    printf("%lf\n", 1.0/3.0);
    printf("%lf\n", 1.0/3.0 + 1.0/3.0 +1.0/3.0);
    iniciarPista(d, n);

    // for (t = 0; quantCiclistasAtivos > 1; t += intervalo){
    //     atualizarPista();
    // }
    return 0;
}