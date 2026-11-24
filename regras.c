#include <stdlib.h>
#include <string.h>
#include "regras.h"

void iniciarRegras(EstadoRegras *reg, int recordeInicial) {
    if (!reg) return;
    reg->turno = 0;
    reg->status = STATUS_EM_ANDAMENTO;
    iniciarPontuacao(&reg->pontuacao, recordeInicial);
}

void definirMensagemJogo(Jogo *jogo, const char *texto) {
    if (!jogo || !texto) return;

    if (jogo->mensagem) {
        free(jogo->mensagem);
        jogo->mensagem = NULL;
    }

    jogo->mensagem = malloc(strlen(texto) + 1);
    if (jogo->mensagem) {
        strcpy(jogo->mensagem, texto);
    }
}

int contarInimigosVivos(const Jogo *jogo) {
    if (!jogo || !jogo->inimigos) return 0;

    int vivos = 0;
    for (int i = 0; i < jogo->qtdInimigos; i++) {
        if (jogo->inimigos[i].vida > 0) {
            vivos++;
        }
    }
    return vivos;
}

void verificarFimDeJogo(Jogo *jogo, EstadoRegras *reg) {
    if (!jogo || !reg) return;

    if (jogo->jogador.vida <= 0) {
        jogo->jogador.vida = 0;
        reg->status = STATUS_DERROTA;
        definirMensagemJogo(jogo, "Derrota! Sua vida chegou a zero.");
        return;
    }

    if (jogo->qtdInimigos > 0 && contarInimigosVivos(jogo) == 0) {
        reg->status = STATUS_VITORIA;
        adicionarPontos(&reg->pontuacao, PTS_VITORIA);
        definirMensagemJogo(jogo, "Vitoria! Todos os inimigos foram derrotados (+100 pts).");
    }
}

int processarMovimento(Jogo *jogo, EstadoRegras *reg, int dx, int dy) {
    if (!jogo || !reg || !jogo->mapa) return 0;

    if (reg->status != STATUS_EM_ANDAMENTO) {
        definirMensagemJogo(jogo, "Partida encerrada. Inicie outro jogo ou carregue um save.");
        return 0;
    }

    int nx = jogo->jogador.x + dx;
    int ny = jogo->jogador.y + dy;

    if (!podeAndar(jogo, nx, ny)) {
        definirMensagemJogo(jogo, "Movimento bloqueado (limite ou parede).");
        return 0;
    }

    reg->turno++;

    int idxInimigo = buscarInimigo(jogo, nx, ny);
    if (idxInimigo >= 0 && jogo->inimigos && jogo->inimigos[idxInimigo].vida > 0) {
        Entidade *inimigo = &jogo->inimigos[idxInimigo];
        inimigo->vida -= DANO_JOGADOR;
        adicionarPontos(&reg->pontuacao, PTS_ATAQUE_INIMIGO);

        if (inimigo->vida <= 0) {
            inimigo->vida = 0;
            adicionarPontos(&reg->pontuacao, PTS_DERROTAR_INIMIGO);
            jogo->jogador.x = nx;
            jogo->jogador.y = ny;
            definirMensagemJogo(jogo, "Inimigo derrotado! Voce avancou (+60 pts).");
        } else {
            jogo->jogador.vida -= DANO_INIMIGO;
            definirMensagemJogo(jogo, "Combate! Acertou (+10 pts) e recebeu dano.");
        }

        verificarFimDeJogo(jogo, reg);
        return 1;
    }

    jogo->jogador.x = nx;
    jogo->jogador.y = ny;
    adicionarPontos(&reg->pontuacao, PTS_MOVIMENTO);
    definirMensagemJogo(jogo, "Movimento valido (+1 pt).");
    verificarFimDeJogo(jogo, reg);
    return 1;
}