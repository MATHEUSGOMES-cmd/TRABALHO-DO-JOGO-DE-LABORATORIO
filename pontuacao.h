#ifndef PONTUACAO_H
#define PONTUACAO_H

#define PTS_MOVIMENTO 1
#define PTS_ATAQUE_INIMIGO 10
#define PTS_DERROTAR_INIMIGO 50
#define PTS_VITORIA 100

typedef struct {
    int pontuacao;
    int recorde;
} PontuacaoEstado;

void iniciarPontuacao(PontuacaoEstado *pts, int recordeInicial);
void adicionarPontos(PontuacaoEstado *pts, int delta);
void atualizarRecorde(PontuacaoEstado *pts);

#endif