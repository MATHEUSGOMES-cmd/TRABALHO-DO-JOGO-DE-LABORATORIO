#ifndef REGRAS_H
#define REGRAS_H

#include "jogo.h"
#include "pontuacao.h"

#define DANO_JOGADOR 10
#define DANO_INIMIGO 5

typedef enum {
    STATUS_EM_ANDAMENTO = 0,
    STATUS_VITORIA = 1,
    STATUS_DERROTA = -1
} StatusJogo;

typedef struct {
    int turno;
    StatusJogo status;
    PontuacaoEstado pontuacao;
} EstadoRegras;

void iniciarRegras(EstadoRegras *reg, int recordeInicial);
void definirMensagemJogo(Jogo *jogo, const char *texto);
int contarInimigosVivos(const Jogo *jogo);
void verificarFimDeJogo(Jogo *jogo, EstadoRegras *reg);
int processarMovimento(Jogo *jogo, EstadoRegras *reg, int dx, int dy);

#endif