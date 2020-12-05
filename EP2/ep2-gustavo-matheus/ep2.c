#include "velodromo.h"

int main(int argc, char* argv[]){
    d = atoi(argv[1]);
    n = atoi(argv[2]);
    if(argc > 3 && strcmp("d", argv[3]) == 0) debug = 1;
    
    iniciarPista(d, n);
    atualizarPista();
    
    return 0;
}