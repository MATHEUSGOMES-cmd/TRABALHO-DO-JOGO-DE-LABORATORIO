#include "entidades.h"

#include "mapa.h"

#include <limits.h>
#include <stdlib.h>

static const Posicao passos[] = {
    [DIR_CIMA] = {0, -1},
    [DIR_BAIXO] = {0, 1},
    [DIR_ESQUERDA] = {-1, 0},
    [DIR_DIREITA] = {1, 0}
};

static int mesma_casa(Posicao pos, int x, int y)
{
    return pos.x == x && pos.y == y;
}

static int mover_posicao(Posicao *pos, Direcao d)
{
    if (d < DIR_CIMA || d >= DIR_NENHUMA) {
        return 0;
    }
    pos->x += passos[d].x;
    pos->y += passos[d].y;
    return 1;
}

int entidades_alocar(Jogo *j)
{
    j->zumbis = malloc(CAP_ZUMBIS * sizeof *j->zumbis);
    j->itens = malloc(CAP_ITENS * sizeof *j->itens);
    if (j->zumbis == NULL || j->itens == NULL) {
        entidades_liberar(j);
        return -1;
    }
    j->cap_zumbis = CAP_ZUMBIS;
    j->cap_itens = CAP_ITENS;
    return 0;
}

void entidades_liberar(Jogo *j)
{
    free(j->zumbis);
    free(j->itens);
    j->zumbis = NULL;
    j->itens = NULL;
    j->n_zumbis = 0;
    j->n_itens = 0;
    j->cap_zumbis = 0;
    j->cap_itens = 0;
}

static void *ampliar_vetor(void *vetor, int *capacidade, size_t tamanho)
{
    if (*capacidade <= 0 || *capacidade > INT_MAX / 2
        || (size_t)*capacidade > (size_t)-1 / tamanho / 2) {
        return NULL;
    }

    int nova_capacidade = *capacidade * 2;
    void *novo = realloc(vetor, (size_t)nova_capacidade * tamanho);
    if (novo != NULL) {
        *capacidade = nova_capacidade;
    }
    return novo;
}

int entidades_adicionar_zumbi(Jogo *j, int x, int y)
{
    if (!mapa_livre(&j->mapa, x, y) || zumbi_em(j, x, y) >= 0) {
        return -1;
    }
    if (j->n_zumbis == j->cap_zumbis) {
        Zumbi *novo = ampliar_vetor(j->zumbis, &j->cap_zumbis, sizeof *novo);
        if (novo == NULL) {
            return -1;
        }
        j->zumbis = novo;
    }
    j->zumbis[j->n_zumbis++] = (Zumbi){{x, y}, VIDA_ZUMBI, 1};
    return 0;
}

int entidades_adicionar_item(Jogo *j, int x, int y, TipoItem tipo)
{
    if (!mapa_livre(&j->mapa, x, y) || item_em(j, x, y) >= 0
        || tipo < ITEM_COMIDA || tipo >= N_TIPOS_ITEM) {
        return -1;
    }
    if (j->n_itens == j->cap_itens) {
        Item *novo = ampliar_vetor(j->itens, &j->cap_itens, sizeof *novo);
        if (novo == NULL) {
            return -1;
        }
        j->itens = novo;
    }
    j->itens[j->n_itens++] = (Item){{x, y}, tipo, 1};
    return 0;
}

void entidades_popular(Jogo *j)
{
    for (int y = 0; y < j->mapa.altura; y++) {
        for (int x = 0; x < j->mapa.largura; x++) {
            char casa = mapa_obter(&j->mapa, x, y);
            int resultado = 0;

            switch (casa) {
            case '@':
                j->jogador.pos = (Posicao){x, y};
                break;
            case 'Z':
                resultado = entidades_adicionar_zumbi(j, x, y);
                break;
            case 'F':
                resultado = entidades_adicionar_item(j, x, y, ITEM_COMIDA);
                break;
            case 'R':
                resultado = entidades_adicionar_item(j, x, y, ITEM_REMEDIO);
                break;
            case 'M':
                resultado = entidades_adicionar_item(j, x, y, ITEM_MUNICAO);
                break;
            default:
                continue;
            }
            if (resultado == 0) {
                mapa_definir(&j->mapa, x, y, '.');
            }
        }
    }
}

int zumbi_em(const Jogo *j, int x, int y)
{
    for (int i = 0; i < j->n_zumbis; i++) {
        if (j->zumbis[i].ativo && mesma_casa(j->zumbis[i].pos, x, y)) {
            return i;
        }
    }
    return -1;
}

int item_em(const Jogo *j, int x, int y)
{
    for (int i = 0; i < j->n_itens; i++) {
        if (j->itens[i].ativo && mesma_casa(j->itens[i].pos, x, y)) {
            return i;
        }
    }
    return -1;
}

int zumbi_ferir(Jogo *j, int indice, int dano)
{
    if (indice < 0 || indice >= j->n_zumbis || dano <= 0) {
        return 0;
    }
    Zumbi *z = &j->zumbis[indice];
    if (!z->ativo) {
        return 0;
    }
    if (dano >= z->vida) {
        z->vida = 0;
        z->ativo = 0;
        return 1;
    }
    z->vida -= dano;
    return 0;
}

int jogador_mover(Jogo *j, Direcao d)
{
    Posicao destino = j->jogador.pos;
    if (!mover_posicao(&destino, d) || !mapa_livre(&j->mapa, destino.x, destino.y)) {
        return 0;
    }

    int indice = zumbi_em(j, destino.x, destino.y);
    if (indice >= 0) {
        return zumbi_ferir(j, indice, DANO_SOCO) ? 3 : 2;
    }
    j->jogador.pos = destino;
    return 1;
}

int jogador_coletar(Jogo *j)
{
    Posicao pos = j->jogador.pos;
    int indice = item_em(j, pos.x, pos.y);
    if (indice < 0) {
        return 0;
    }

    Item *item = &j->itens[indice];
    int quantidade = item->tipo == ITEM_MUNICAO ? TIROS_POR_ITEM : 1;
    int *total = &j->jogador.mochila[item->tipo];
    if (*total > MOCHILA_MAX - quantidade) {
        return 0;
    }
    *total += quantidade;
    item->ativo = 0;
    return 1;
}

int jogador_atirar(Jogo *j, Direcao d)
{
    Posicao pos = j->jogador.pos;
    for (int passo = 0; passo < ALCANCE_TIRO; passo++) {
        if (!mover_posicao(&pos, d) || !mapa_livre(&j->mapa, pos.x, pos.y)) {
            break;
        }
        int indice = zumbi_em(j, pos.x, pos.y);
        if (indice >= 0) {
            zumbi_ferir(j, indice, DANO_TIRO);
            return indice;
        }
    }
    return -1;
}

static int zumbi_tentar_mover(Jogo *j, Zumbi *z, Direcao d)
{
    Posicao destino = z->pos;
    if (!mover_posicao(&destino, d) || !mapa_livre(&j->mapa, destino.x, destino.y)
        || mesma_casa(j->jogador.pos, destino.x, destino.y)
        || zumbi_em(j, destino.x, destino.y) >= 0) {
        return 0;
    }
    z->pos = destino;
    return 1;
}

static void zumbi_perseguir(Jogo *j, Zumbi *z, int dx, int dy)
{
    Direcao horizontal = DIR_NENHUMA;
    Direcao vertical = DIR_NENHUMA;
    if (dx != 0) {
        horizontal = dx > 0 ? DIR_DIREITA : DIR_ESQUERDA;
    }
    if (dy != 0) {
        vertical = dy > 0 ? DIR_BAIXO : DIR_CIMA;
    }

    Direcao primeira = horizontal;
    Direcao segunda = vertical;
    if (abs(dy) > abs(dx)) {
        primeira = vertical;
        segunda = horizontal;
    }
    if (!zumbi_tentar_mover(j, z, primeira)) {
        zumbi_tentar_mover(j, z, segunda);
    }
}

int zumbis_agir(Jogo *j)
{
    if (j->turno % 2 != 0) {
        return 0;
    }

    int mordidas = 0;
    for (int i = 0; i < j->n_zumbis; i++) {
        Zumbi *z = &j->zumbis[i];
        if (!z->ativo) {
            continue;
        }
        int dx = j->jogador.pos.x - z->pos.x;
        int dy = j->jogador.pos.y - z->pos.y;
        int distancia = abs(dx) + abs(dy);

        if (distancia == 1) {
            mordidas++;
        } else if (distancia <= ALCANCE_VISAO) {
            zumbi_perseguir(j, z, dx, dy);
        } else {
            j->sorteios++;
            zumbi_tentar_mover(j, z, (Direcao)(rand() % DIR_NENHUMA));
        }
    }
    return mordidas;
}
