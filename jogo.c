/*
 jogo.c - Criação do mapa (com malloc) e regras do jogo
 O mapa é definido como o char MAPA_TXT, porem NADA aqui desenha na tela: isso fica no render.c
 */

#include <stdio.h>
#include <stdlib.h>
#include "jogo.h"

/*
O mapa desenhado aqui vai ser carregado no jogo, lista doque cada coisa significa:
# = parede
. = chão
~ = água
T = árvore
P = jogador
E = inimigo

AVISO: no jogo.h ta dizendo a altura e largura do mapa, se for mudar esses dois vão ter que mecher la também
 */
char MAPA_TXT[ALTURA][LARGURA + 1] = {
    "##############################",
    "#P..........~~~~........T....#",
    "#...........~~~~.............#",
    "#...T.......~~~...........E..#",
    "#............................#",
    "#.............TT.............#",
    "#............................#",
    "#...~~~......................#",
    "#...~~~.E....................#",
    "#.T.....................T....#",
    "#............................#",
    "##############################",
};

int criarJogo(Jogo *jogo) {
    /* Valores iniciais */
    jogo->mapa = NULL;
    jogo->inimigos = NULL;
    jogo->qtdInimigos = 0;
    jogo->mensagem = "";
    jogo->jogador.x = 1;
    jogo->jogador.y = 1;
    jogo->jogador.vida = 3;

    /* conta o número de inimigos no desenho */
    for (int linha = 0; linha < ALTURA; linha++) {
        for (int coluna = 0; coluna < LARGURA; coluna++) {
            if (MAPA_TXT[linha][coluna] == 'E') {
                jogo->qtdInimigos++;
            }
        }
    }

    /* aloca memoria */
    jogo->mapa = (char**) malloc(ALTURA * sizeof(char*));
    if (jogo->mapa == NULL) {
        return 0;
    }
    for (int linha = 0; linha < ALTURA; linha++) {
        jogo->mapa[linha] = (char*) malloc(LARGURA * sizeof(char));
        if (jogo->mapa[linha] == NULL) {
            for (int indice = 0; indice < linha; indice++) {
                free(jogo->mapa[indice]);   /* desfaz as linhas já criadas */
            }
            free(jogo->mapa);
            jogo->mapa = NULL;
            return 0;
        }
    }

    /* alocar os inimigos como vetor */
    if (jogo->qtdInimigos > 0) {
        jogo->inimigos = (Entidade*) malloc(jogo->qtdInimigos * sizeof(Entidade));
        if (jogo->inimigos == NULL) {
            liberarJogo(jogo);
            return 0;
        }
    }

    int indiceInimigo = 0;
    for (int linha = 0; linha < ALTURA; linha++) {
        for (int coluna = 0; coluna < LARGURA; coluna++) {
            char simbolo = MAPA_TXT[linha][coluna];

            if (simbolo == 'P') {
                jogo->jogador.x = coluna;
                jogo->jogador.y = linha;
                simbolo = '.';/* embaixo do jogador tem chão */
            } else if (simbolo == 'E') {
                jogo->inimigos[indiceInimigo].x = coluna;
                jogo->inimigos[indiceInimigo].y = linha;
                jogo->inimigos[indiceInimigo].vida = 1;
                indiceInimigo++;
                simbolo = '.';
            } else if (simbolo == '\0') { /* casos a linha seja curta demais */
                simbolo = ' ';
            }

            jogo->mapa[linha][coluna] = simbolo;
        }
    }
    return 1;
}

void liberarJogo(Jogo *jogo) {
    if (jogo->mapa != NULL) {
        for (int linha = 0; linha < ALTURA; linha++) {
            free(jogo->mapa[linha]);
        }
        free(jogo->mapa);
        jogo->mapa = NULL;
    }
    free(jogo->inimigos);
    jogo->inimigos = NULL;
}

/* Retorna 1 se a posição está dentro do mapa e não é obstáculo */
int podeAndar(Jogo *jogo, int x, int y) {
    if (x < 0 || x >= LARGURA || y < 0 || y >= ALTURA) {
        return 0;
    }
    char simbolo = jogo->mapa[y][x];
    if (simbolo == '#' || simbolo == '~' || simbolo == 'T') {
        return 0;
    }
    return 1;
}

/* retorna a posição do inimigo */
int buscarInimigo(Jogo *jogo, int x, int y) {
    for (int indice = 0; indice < jogo->qtdInimigos; indice++) {
        if (jogo->inimigos[indice].x == x && jogo->inimigos[indice].y == y) {
            return indice;
        }
    }
    return -1;
}

void moverJogador(Jogo *jogo, int dx, int dy) {
    int novoX = jogo->jogador.x + dx;
    int novoY = jogo->jogador.y + dy;

    if (!podeAndar(jogo, novoX, novoY)) {
        jogo->mensagem = "Caminho bloqueado!";
        return;
    }

    jogo->jogador.x = novoX;
    jogo->jogador.y = novoY;
    jogo->mensagem = " ";

}
