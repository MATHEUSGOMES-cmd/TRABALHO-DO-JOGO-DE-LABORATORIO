#include "regras.h"

#include "entidades.h"
#include "mapa.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Jogo *jogo_criar(void)
{
    Jogo *j = calloc(1, sizeof *j);
    if (j == NULL) {
        return NULL;
    }
    if (mapa_criar_padrao(&j->mapa) != 0 || entidades_alocar(j) != 0) {
        jogo_destruir(j);
        return NULL;
    }
    entidades_popular(j);
    j->jogador.vida = VIDA_MAX;
    j->jogador.fome = FOME_MAX;
    j->estado = PARTIDA_ANDAMENTO;
    j->semente = 1; /* Semente inicial de rand() se srand() ainda não foi chamado. */
    snprintf(j->mensagem, sizeof j->mensagem,
             "Você acordou num bairro abandonado. Ache comida, remédio e munição.");
    return j;
}

void jogo_destruir(Jogo *j)
{
    if (j != NULL) {
        entidades_liberar(j);
        mapa_destruir(&j->mapa);
        free(j);
    }
}

void jogo_semear(Jogo *j, unsigned int semente)
{
    j->semente = semente;
    j->sorteios = 0;
    srand(semente);
}

static void avisar(Jogo *j, const char *texto)
{
    snprintf(j->mensagem, sizeof j->mensagem, "%s", texto);
}

static void completar_aviso(Jogo *j, const char *texto)
{
    size_t usado = strlen(j->mensagem);
    snprintf(j->mensagem + usado, sizeof j->mensagem - usado, "%s", texto);
}

static void abater(Jogo *j)
{
    j->jogador.abates++;
    j->pontos += PONTOS_ABATE;
}

static int recuperar(int atual, int aumento, int maximo)
{
    int resultado = atual + aumento;
    return resultado > maximo ? maximo : resultado;
}

static int aplicar_acao(Jogo *j, Comando c)
{
    Jogador *p = &j->jogador;

    switch (c.tipo) {
    case ACAO_MOVER: {
        int resultado = jogador_mover(j, c.dir);
        if (resultado == 0) {
            avisar(j, "Caminho bloqueado. Escolha outra direção.");
            return 0;
        }
        if (resultado == 3) {
            abater(j);
            avisar(j, "Você abateu um zumbi com um golpe.");
        } else if (resultado == 2) {
            avisar(j, "Você acertou um golpe no zumbi.");
        } else {
            avisar(j, "Você se moveu.");
        }
        break;
    }
    case ACAO_ATIRAR: {
        if (p->mochila[ITEM_MUNICAO] == 0) {
            avisar(j, "Você está sem munição.");
            return 0;
        }
        if (c.dir < DIR_CIMA || c.dir >= DIR_NENHUMA) {
            avisar(j, "Informe a direção do tiro (fw, fa, fs ou fd).");
            return 0;
        }
        p->mochila[ITEM_MUNICAO]--;
        int indice = jogador_atirar(j, c.dir);
        if (indice < 0) {
            avisar(j, "Você disparou, mas não acertou nenhum zumbi.");
        } else if (!j->zumbis[indice].ativo) {
            abater(j);
            avisar(j, "Você abateu um zumbi com um tiro.");
        } else {
            avisar(j, "Você acertou um tiro no zumbi.");
        }
        break;
    }
    case ACAO_COMER:
        if (p->mochila[ITEM_COMIDA] == 0) {
            avisar(j, "Você está sem comida.");
            return 0;
        }
        p->mochila[ITEM_COMIDA]--;
        p->fome = recuperar(p->fome, VALOR_COMIDA, FOME_MAX);
        avisar(j, "Você comeu.");
        break;
    case ACAO_REMEDIO:
        if (p->mochila[ITEM_REMEDIO] == 0) {
            avisar(j, "Você está sem remédio.");
            return 0;
        }
        p->mochila[ITEM_REMEDIO]--;
        p->vida = recuperar(p->vida, VALOR_REMEDIO, VIDA_MAX);
        avisar(j, "Você usou um remédio.");
        break;
    case ACAO_ESPERAR:
        avisar(j, "Você esperou um turno.");
        break;
    case ACAO_GRAVAR:
    case ACAO_AJUDA:
    case ACAO_SAIR:
        return 0;
    default:
        avisar(j, "Comando inválido. Digite ? para ver a ajuda.");
        return 0;
    }
    return 1;
}

static void resolver_fim(Jogo *j)
{
    Posicao pos = j->jogador.pos;
    Posicao heliponto = j->mapa.heliponto;

    if (j->jogador.vida <= 0) {
        j->jogador.vida = 0;
        j->estado = PARTIDA_DERROTA_VIDA;
    } else if (j->turno >= TURNO_RESGATE && pos.x == heliponto.x && pos.y == heliponto.y) {
        j->estado = PARTIDA_VITORIA;
        j->pontos += PONTOS_VITORIA + PONTOS_POR_VIDA * j->jogador.vida
                     + LIMITE_TURNOS - j->turno;
    } else if (j->turno >= LIMITE_TURNOS) {
        j->estado = PARTIDA_DERROTA_TEMPO;
    }
}

int jogo_turno(Jogo *j, Comando c)
{
    if (j->estado != PARTIDA_ANDAMENTO || !aplicar_acao(j, c)) {
        return 0;
    }

    if (jogador_coletar(j)) {
        j->pontos += PONTOS_ITEM;
        completar_aviso(j, " Item coletado.");
    }

    int mordidas = zumbis_agir(j);
    j->jogador.vida -= DANO_MORDIDA * mordidas;
    if (mordidas > 0) {
        completar_aviso(j, " Você foi mordido.");
    }

    j->jogador.fome -= FOME_POR_TURNO;
    if (j->jogador.fome <= 0) {
        j->jogador.fome = 0;
        j->jogador.vida -= DANO_FOME;
        completar_aviso(j, " A fome tirou vida.");
    }

    j->turno++;
    if (j->turno == TURNO_RESGATE) {
        completar_aviso(j, " O helicóptero chegou!");
    }
    resolver_fim(j);
    return 1;
}

int jogo_pontuacao(const Jogo *j)
{
    return j->pontos;
}
