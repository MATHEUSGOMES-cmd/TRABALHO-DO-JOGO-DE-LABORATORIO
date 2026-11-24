#ifndef ARQUIVOS_H
#define ARQUIVOS_H

#include "jogo.h"
#include "regras.h"

#define ARQUIVO_SAVE_TXT "save_jogo.txt"
#define ARQUIVO_RECORDE_TXT "recorde.txt"

int salvarRecordeArquivo(const char *caminho, int recorde);
int carregarRecordeArquivo(const char *caminho, int *recordeSaida);
int salvarJogoArquivo(const char *caminho, const Jogo *jogo, const EstadoRegras *reg);
int carregarJogoArquivo(const char *caminho, Jogo *jogo, EstadoRegras *reg);

#endif