#! /usr/bin/unbsh
processos -elf > texto.txt
conta -l < texto.txt
acha . | filtra src > teste.txt
dorme 100 &
pwd
criadir meu_dir
mudar meu_dir
move ../texto.txt ../meu_dir/.
historico
historico 5
#nova bateria de comandos
mudar ..
criadir tmp
lista -a > ./tmp/saida.txt
mudar tmp
mostra saida.txt ../meu_dir/texto.txt | conta -l
uname -a
# aqui neste ponto compila um programa c qualquer (exemplo.c) que imprime “Alo UnB. Feliz 2022!”.
mudar ..
compila -o exemplo exemplo.c
criadir bin
move exemplo ./bin/.
mudar bin
exemplo
#fim do arquivo em lote