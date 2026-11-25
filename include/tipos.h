#ifndef TIPOS_H
#define TIPOS_H

#define LARGURA          40
#define ALTURA           16
#define VIDA_MAX        100
#define FOME_MAX        100
#define FOME_POR_TURNO    2
#define DANO_FOME         3
#define MOCHILA_MAX       9
#define TURNO_RESGATE    60
#define LIMITE_TURNOS   120
#define VIDA_ZUMBI        3
#define DANO_MORDIDA     10
#define DANO_SOCO         1
#define DANO_TIRO         3
#define ALCANCE_TIRO      6
#define ALCANCE_VISAO     6
#define VALOR_COMIDA     40
#define VALOR_REMEDIO    30
#define TIROS_POR_ITEM    4
#define MAX_NOME         16
#define MAX_RANKING       5
#define CAP_ZUMBIS       32
#define CAP_ITENS        32

#define PONTOS_ABATE     10
#define PONTOS_ITEM       5
#define PONTOS_VITORIA  500
#define PONTOS_POR_VIDA   2

typedef enum {
    ITEM_COMIDA, ITEM_REMEDIO, ITEM_MUNICAO, N_TIPOS_ITEM
} TipoItem;

typedef enum {
    DIR_CIMA, DIR_BAIXO, DIR_ESQUERDA, DIR_DIREITA, DIR_NENHUMA
} Direcao;

typedef enum {
    ACAO_MOVER, ACAO_ATIRAR, ACAO_COMER, ACAO_REMEDIO, ACAO_ESPERAR,
    ACAO_GRAVAR, ACAO_AJUDA, ACAO_SAIR, ACAO_INVALIDA
} TipoAcao;

typedef enum {
    PARTIDA_ANDAMENTO, PARTIDA_VITORIA, PARTIDA_DERROTA_VIDA, PARTIDA_DERROTA_TEMPO
} EstadoPartida;

typedef struct { int x, y; } Posicao;
typedef struct { TipoAcao tipo; Direcao dir; } Comando;

typedef struct {
    Posicao pos;
    int vida, fome, abates;
    int mochila[N_TIPOS_ITEM];
} Jogador;

typedef struct { Posicao pos; int vida; int ativo; } Zumbi;
typedef struct { Posicao pos; TipoItem tipo; int ativo; } Item;

typedef struct {
    int largura, altura;
    char **grade;
    Posicao heliponto;
} Mapa;

typedef struct {
    Mapa mapa;
    Jogador jogador;
    Zumbi *zumbis;
    int n_zumbis, cap_zumbis;
    Item *itens;
    int n_itens, cap_itens;
    int turno, pontos;
    EstadoPartida estado;
    char mensagem[120];
    unsigned int semente, sorteios;
} Jogo;

typedef struct {
    char nome[MAX_NOME];
    int pontos, turnos, venceu;
} RegistroRanking;

#endif
