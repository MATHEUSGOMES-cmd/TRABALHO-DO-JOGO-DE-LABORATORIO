#ifndef ENTIDADES_H
#define ENTIDADES_H

#include "tipos.h"

int entidades_alocar(Jogo *j); // malloc vetor zumbis e itens, -1 se nao alocar
void entidades_liberar(Jogo *j); // free vetores
void entidades_popular(Jogo *j);  //varre o mapa e adiciona as entidades em seus lugares
int entidades_adicionar_zumbi(Jogo *j, int x, int y); // adiciona zumbi no mapa e faz realloc caso a capacidade seja totalmente utilizada
int entidades_adicionar_item(Jogo *j, int x, int y, TipoItem t); //adiciona item no mapa e faz realloc caso a capacidade seja totalmente utilizada
int zumbi_em(const Jogo *j, int x, int y); // ve se o zumbi ta ativo, retorna o indice se for encontrado ou -1 se nao for
int item_em(const Jogo *j, int x, int y);  // ve se o  item ta ativo, retorna o indice se for encontrado ou -1 se nao for
int jogador_mover(Jogo *j, Direcao d); // movimento do jogador
int jogador_coletar(Jogo *j); // coletar item
int jogador_atirar(Jogo *j, Direcao d); // atirar 
int zumbir_ferir(Jogo *j, int idx, int dano); // atacar zumbir
int zumbir_ferir(Jogo *j, int idx, int dano); // acao zumbi

#endif