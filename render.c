/*
Ainda to trabalhando na cor da interface
os arquivos do jogos são um template feito por ia só pra eu ir conseguindo ter algo pra ver como tela
*/

#include <stdio.h>
#include "jogo.h"

/* Esse trecho está defindo as cores, a lógica é parecida com python. Achei melhor que usar um struct pro tipo cor pois só preciso por "COR" ao inves de cor.COR
Inclusive isso já estava na função limpar tela, pois tinha começado a implementar*/

#define RESET "\x1b[0m"
#define COR_CINZA "\x1b[38;5;245m"
#define COR_ESCURO "\x1b[38;5;238m"
#define COR_AZUL "\x1b[38;5;32m"
#define COR_VERDE "\x1b[38;5;107m"
#define COR_AMARELO "\x1b[38;5;220m"
#define COR_CIANO "\x1b[1;38;5;51m"
#define COR_VERMELHO "\x1b[38;5;209m"
#define COR_VERMELHO_ESCURO "\x1b[38;5;88m"
#define COR_BRANCO "\x1b[1;37m"

#define INIMIGO ""COR_VERMELHO "☠ " RESET""
#define PAREDE ""COR_CINZA "■ " RESET""
#define CHAO ""COR_ESCURO "░ " RESET""
#define AGUA ""COR_AZUL "≋ " RESET""
#define ARVORE ""COR_VERDE "✿ " RESET""

void limparTela(void) {
    printf("\x1b[2J\x1b[H");
}

/* casa simbolo ta por padrão ocupando 2 caracteres, se tirar isso aq eles vão oculpar o espaço padrão e vai desalinhar tudo */
void desenharTile(char simbolo) {
    switch (simbolo) {
        case '#': printf(PAREDE); break; /* parede */
        case '.': printf(CHAO); break; /* chão */
        case '~': printf(AGUA); break; /* água */
        case 'T': printf(ARVORE); break; /* árvore */
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
                printf(INIMIGO);
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
