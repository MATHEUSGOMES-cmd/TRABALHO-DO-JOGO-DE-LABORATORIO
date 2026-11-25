#ifndef ARQUIVOS_H
#define ARQUIVOS_H

#include "tipos.h"

void arquivos_localizar_dados(void);
int arquivos_criar_dados(void);
RegistroRanking *ranking_carregar(const char *caminho, int *n);
void ranking_inserir(RegistroRanking *lista, int *n, RegistroRanking novo);
int ranking_salvar(const char *caminho, const RegistroRanking *lista, int n);
int partida_salvar(const Jogo *j, const char *caminho);
int partida_carregar(Jogo *j, const char *caminho);

#endif
