#ifndef ENTIDADES_H
#define ENTIDADES_H

#include "tipos.h"

int entidades_alocar(Jogo *j);
void entidades_liberar(Jogo *j);
void entidades_popular(Jogo *j);
int entidades_adicionar_zumbi(Jogo *j, int x, int y);
int entidades_adicionar_item(Jogo *j, int x, int y, TipoItem tipo);
int zumbi_em(const Jogo *j, int x, int y);
int item_em(const Jogo *j, int x, int y);
int jogador_mover(Jogo *j, Direcao d);
int jogador_coletar(Jogo *j);
int jogador_atirar(Jogo *j, Direcao d);
int zumbi_ferir(Jogo *j, int indice, int dano);
int zumbis_agir(Jogo *j);

#endif
