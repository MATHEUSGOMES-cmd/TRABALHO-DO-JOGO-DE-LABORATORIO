#ifndef REGRAS_H
#define REGRAS_H

#include "tipos.h"

Jogo *jogo_criar(void);
void jogo_destruir(Jogo *j);
void jogo_semear(Jogo *j, unsigned int semente);
int jogo_turno(Jogo *j, Comando c);
int jogo_pontuacao(const Jogo *j);

#endif
