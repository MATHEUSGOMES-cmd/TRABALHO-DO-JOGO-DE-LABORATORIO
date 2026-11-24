#include "pontuacao.h"

void iniciarPontuacao(PontuacaoEstado *pts, int recordeInicial) {
    if (!pts) return;
    pts->pontuacao = 0;
    pts->recorde = (recordeInicial > 0) ? recordeInicial : 0;
}

void atualizarRecorde(PontuacaoEstado *pts) {
    if (!pts) return;
    if (pts->pontuacao < 0) {
        pts->pontuacao = 0;
    }
    if (pts->pontuacao > pts->recorde) {
        pts->recorde = pts->pontuacao;
    }
}

void adicionarPontos(PontuacaoEstado *pts, int delta) {
    if (!pts) return;
    pts->pontuacao += delta;
    atualizarRecorde(pts);
}