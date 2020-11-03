#include "velodromo.h"

int main(int argc, char* argv[]){
    d = atoi(argv[1]);
    n = atoi(argv[2]);

    iniciarPista(d, n);
    atualizarPista();
    
    return 0;
}