/*
Ainda to trabalhando na cor da interface
os arquivos do jogos são um template feito por ia só pra eu ir conseguindo ter algo pra ver como tela 
*/

#include <stdio.h>
#include "jogo.h"

void limparTela(void) {
    printf("\x1b[2J\x1b[H");
}

/* casa simbolo ta por padrão ocupando 2 caracteres, se tirar isso aq eles vão oculpar o espaço padrão e vai desalinhar tudo */
void desenharTile(char simbolo) {
    switch (simbolo) {
        case '#': printf("■ "); break; /* parede */
        case '.': printf("░ "); break; /* chão */
        case '~': printf("≋ "); break; /* água */
        case 'T': printf("✿ "); break; /* árvore */
        default:  printf("  "); break; /* vazio */
    }
}

void desenharJogo(Jogo *jogo) {
    limparTela();

    /* aqui ele vai receber a posição de cada coisa e desenhar */
    for (int linha = 0; linha < ALTURA; linha++) {
        for (int coluna = 0; coluna < LARGURA; coluna++) {
            if (jogo->jogador.x == coluna && jogo->jogador.y == linha) {
                printf("● ");
            } else if (buscarInimigo(jogo, coluna, linha) != -1) {
                printf("☠ ");
            } else {
                desenharTile(jogo->mapa[linha][coluna]);
            } /* já que o inimigo e o jogador são as unicas coisas que se mechem o else ta ai pra desenhar todo o resto do cenário (que é estático) */
        }
        printf("\n");
    }

    /* status e dados do jogador */
    printf("\n Vida: ");
    for (int indice = 0; indice < jogo->jogador.vida; indice++) {
        printf("❤ ");
    }
    printf(" %s\n\n", jogo->mensagem);
}
