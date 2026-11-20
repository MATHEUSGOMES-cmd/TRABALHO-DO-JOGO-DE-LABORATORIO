/*
 * aqui é a base onde ficam os structs e declarações das funções
 * (os .c incluem este arquivo com #include "jogo.h")
 */

#ifndef JOGO_H
#define JOGO_H

#define LARGURA 30
#define ALTURA 12

/* mesma base pro jogar e inimigo ou qualquer ser */
typedef struct {
    int x, y;
    int vida;
} Entidade;

/* aqui é a estrutura do jogo*/
typedef struct {
    char **mapa; /* o mapa é uma matriz de altura e largura que tem a posição de tudo dentro do jogo */
    Entidade jogador;
    Entidade *inimigos;
    int qtdInimigos;
    char *mensagem;
} Jogo;

int criarJogo(Jogo *jogo); /* retorna 1 se deu certo, 0 se faltou memória */
void liberarJogo(Jogo *jogo);
int podeAndar(Jogo *jogo, int x, int y);
int buscarInimigo(Jogo *jogo, int x, int y);
void moverJogador(Jogo *jogo, int dx, int dy);

/* parte que chama o arquivo que renderiza a tela (no caso: escreve)*/
void limparTela(void);
void desenharJogo(Jogo *jogo);

#endif
