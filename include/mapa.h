#ifndef MAPA_H
#define MAPA_H

#include "tipos.h"

int mapa_alocar(Mapa *m, int largura, int altura);
int mapa_criar_padrao(Mapa *m);
void mapa_destruir(Mapa *m);
int mapa_dentro(const Mapa *m, int x, int y);
char mapa_obter(const Mapa *m, int x, int y);
void mapa_definir(Mapa *m, int x, int y, char c);
int mapa_livre(const Mapa *m, int x, int y);

#endif
