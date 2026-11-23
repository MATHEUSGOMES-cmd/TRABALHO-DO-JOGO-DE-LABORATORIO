#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arquivos.h"

int salvarRecordeArquivo(const char *caminho, int recorde) {
    if (!caminho || recorde < 0) return 0;

    FILE *fp = fopen(caminho, "w");
    if (!fp) return 0;

    fprintf(fp, "RECORDE %d\n", recorde);
    fclose(fp);
    return 1;
}

int carregarRecordeArquivo(const char *caminho, int *recordeSaida) {
    if (!caminho || !recordeSaida) return 0;

    FILE *fp = fopen(caminho, "r");
    if (!fp) return 0;

    int rec = 0;
    int ok = (fscanf(fp, "RECORDE %d", &rec) == 1 && rec >= 0);
    fclose(fp);

    if (!ok) return 0;
    *recordeSaida = rec;
    return 1;
}

int salvarJogoArquivo(const char *caminho, const Jogo *jogo, const EstadoRegras *reg) {
    if (!caminho || !jogo || !reg || !jogo->mapa) return 0;

    FILE *fp = fopen(caminho, "w");
    if (!fp) return 0;

    fprintf(fp, "DIMENSOES %d %d\n", LARGURA, ALTURA);
    fprintf(fp, "ESTADO %d %d %d %d\n",
            reg->pontuacao.pontuacao,
            reg->pontuacao.recorde,
            reg->turno,
            (int)reg->status);
    fprintf(fp, "JOGADOR %d %d %d\n",
            jogo->jogador.x, jogo->jogador.y, jogo->jogador.vida);
    fprintf(fp, "INIMIGOS %d\n", jogo->qtdInimigos);

    for (int i = 0; i < jogo->qtdInimigos; i++) {
        fprintf(fp, "INIMIGO %d %d %d %d\n",
                i, jogo->inimigos[i].x, jogo->inimigos[i].y, jogo->inimigos[i].vida);
    }

    fprintf(fp, "MAPA\n");
    for (int y = 0; y < ALTURA; y++) {
        for (int x = 0; x < LARGURA; x++) {
            fputc(jogo->mapa[y][x], fp);
        }
        fputc('\n', fp);
    }

    fclose(fp);
    return 1;
}

int carregarJogoArquivo(const char *caminho, Jogo *jogo, EstadoRegras *reg) {
    if (!caminho || !jogo || !reg) return 0;

    FILE *fp = fopen(caminho, "r");
    if (!fp) return 0;

    int larg = 0, alt = 0;
    if (fscanf(fp, "DIMENSOES %d %d\n", &larg, &alt) != 2 ||
        larg != LARGURA || alt != ALTURA) {
        fclose(fp);
        return 0;
    }

    int pts = 0, rec = 0, turno = 0, statusInt = 0;
    if (fscanf(fp, "ESTADO %d %d %d %d\n", &pts, &rec, &turno, &statusInt) != 4 ||
        pts < 0 || rec < 0 || turno < 0) {
        fclose(fp);
        return 0;
    }

    int jx = 0, jy = 0, jvida = 0;
    if (fscanf(fp, "JOGADOR %d %d %d\n", &jx, &jy, &jvida) != 3) {
        fclose(fp);
        return 0;
    }

    int qtdInim = 0;
    if (fscanf(fp, "INIMIGOS %d\n", &qtdInim) != 1 || qtdInim < 0) {
        fclose(fp);
        return 0;
    }

    Entidade *novosInimigos = NULL;
    if (qtdInim > 0) {
        novosInimigos = malloc((size_t)qtdInim * sizeof(Entidade));
        if (!novosInimigos) {
            fclose(fp);
            return 0;
        }
        for (int i = 0; i < qtdInim; i++) {
            int idx = 0, ix = 0, iy = 0, ivida = 0;
            if (fscanf(fp, "INIMIGO %d %d %d %d\n", &idx, &ix, &iy, &ivida) != 4 ||
                idx != i) {
                free(novosInimigos);
                fclose(fp);
                return 0;
            }
            novosInimigos[i].x = ix;
            novosInimigos[i].y = iy;
            novosInimigos[i].vida = ivida;
        }
    }

    char rotuloMapa[16] = {0};
    if (fscanf(fp, "%15s\n", rotuloMapa) != 1 || strcmp(rotuloMapa, "MAPA") != 0) {
        free(novosInimigos);
        fclose(fp);
        return 0;
    }

    char **novoMapa = malloc((size_t)ALTURA * sizeof(char *));
    if (!novoMapa) {
        free(novosInimigos);
        fclose(fp);
        return 0;
    }

    for (int y = 0; y < ALTURA; y++) {
        novoMapa[y] = malloc((size_t)(LARGURA + 1));
        if (!novoMapa[y]) {
            for (int k = 0; k < y; k++) free(novoMapa[k]);
            free(novoMapa);
            free(novosInimigos);
            fclose(fp);
            return 0;
        }
        for (int x = 0; x < LARGURA; x++) {
            int ch = fgetc(fp);
            if (ch == EOF || ch == '\n') {
                for (int k = 0; k <= y; k++) free(novoMapa[k]);
                free(novoMapa);
                free(novosInimigos);
                fclose(fp);
                return 0;
            }
            novoMapa[y][x] = (char)ch;
        }
        novoMapa[y][LARGURA] = '\0';
        int nl = fgetc(fp);
        if (nl != '\n' && nl != EOF) {
            for (int k = 0; k <= y; k++) free(novoMapa[k]);
            free(novoMapa);
            free(novosInimigos);
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);

    if (jogo->mapa) {
        for (int y = 0; y < ALTURA; y++) {
            free(jogo->mapa[y]);
        }
        free(jogo->mapa);
    }
    if (jogo->inimigos) {
        free(jogo->inimigos);
    }

    jogo->mapa = novoMapa;
    jogo->inimigos = novosInimigos;
    jogo->qtdInimigos = qtdInim;
    jogo->jogador.x = jx;
    jogo->jogador.y = jy;
    jogo->jogador.vida = jvida;

    iniciarPontuacao(&reg->pontuacao, rec);
    reg->pontuacao.pontuacao = pts;
    atualizarRecorde(&reg->pontuacao);
    reg->turno = turno;
    reg->status = (StatusJogo)statusInt;

    definirMensagemJogo(jogo, "Jogo carregado do arquivo de texto.");
    return 1;
}