#ifndef INTERFACE_H
#define INTERFACE_H

#include "tipos.h"

void interface_iniciar(void);
void mostrar_mensagem(const char *mensagem);
int menu_principal(void);
void mostrar_instrucoes(void);
void desenhar_tela(const Jogo *j);
Comando ler_comando(void);
void mostrar_fim(const Jogo *j);
void mostrar_ranking(const RegistroRanking *lista, int n);
void pedir_nome(char *nome, int tam);
int perguntar_reiniciar(void);

#endif
