/*
meio auto explicativo, mas a main.c é onde fica o funcionamento do jogo
usem esse comando na hora de compilar:
gcc -Wall -Wextra -o jogo main.c jogo.c render.c
 */

#include <stdio.h>
#include <stdlib.h>
#include "jogo.h"

void jogar() {
    Jogo jogo;
    char comando = ' ';
    int dx, dy;

    if (!criarJogo(&jogo)) {
        printf("Erro: memória insuficiente!\n");
        return;
    }
    while (jogo.jogador.vida != 0) {
        desenharJogo(&jogo);
        printf(" Comando (w/a/s/d, q = menu): ");
        if (scanf(" %c", &comando) != 1) {
            break;
        }

        dx = 0;
        dy = 0;

        /* aqui é só pra pessoa usar wasd pra andar que nem jogo */
        switch (comando) {
            case 'w': case 'W': dy = -1; break;
            case 's': case 'S': dy = 1; break;
            case 'a': case 'A': dx = -1; break;
            case 'd': case 'D': dx = 1; break;
            case 'q': case 'Q': break;
            default: jogo.mensagem = "Comando inválido!";
        }

        if (dx != 0 || dy != 0) {
            moverJogador(&jogo, dx, dy);
        }
    }

    liberarJogo(&jogo);
}

int main() {

    jogar();

    return 0;
}
