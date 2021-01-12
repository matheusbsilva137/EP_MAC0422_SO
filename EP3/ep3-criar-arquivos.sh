#!/bin/bash

echo "O script criará os arquivos /tmp/arquivo10M de 10MB e /tmp/arquivo50 de 50B"
echo "Se já existir arquivos com esses nomes, eles serão sobrescritos"
echo "Pressione ENTER para continuar ou CTRL+c para interromper"
read

rm -f /tmp/arquivo10M
rm -f /tmp/arquivo50
for I in `seq 1 200000`;do echo -n "bancos futeis pagavam lhe queijo whisky e xadrez  " >> /tmp/arquivo10M; done
echo -n "bancos futeis pagavam lhe queijo whisky e xadrez  " > /tmp/arquivo50

exit 0
