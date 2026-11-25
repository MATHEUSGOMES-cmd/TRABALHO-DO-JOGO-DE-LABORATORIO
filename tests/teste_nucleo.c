#include "arquivos.h"
#include "entidades.h"
#include "mapa.h"
#include "regras.h"

#undef NDEBUG
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SAVE "teste_nucleo_save.txt"
#define CONTINUO "teste_nucleo_continuo.txt"
#define RETOMADO "teste_nucleo_retomado.txt"
#define INVALIDO "teste_nucleo_invalido.txt"
#define RANKING "teste_nucleo_ranking.txt"

static Jogo *criar_jogo(void)
{
    Jogo *j = jogo_criar();
    assert(j != NULL);
    jogo_semear(j, 42);
    return j;
}

static Jogo *criar_arena(void)
{
    Jogo *j = criar_jogo();
    j->n_zumbis = 0;
    j->n_itens = 0;
    j->jogador.pos = (Posicao){2, 3};
    for (int y = 0; y < ALTURA; y++) {
        for (int x = 0; x < LARGURA; x++) {
            int borda = x == 0 || y == 0 || x == LARGURA - 1 || y == ALTURA - 1;
            mapa_definir(&j->mapa, x, y, borda ? '#' : '.');
        }
    }
    mapa_definir(&j->mapa, j->mapa.heliponto.x, j->mapa.heliponto.y, 'H');
    return j;
}

static int agir(Jogo *j, TipoAcao acao, Direcao dir)
{
    return jogo_turno(j, (Comando){acao, dir});
}

static void testar_mapa(void)
{
    Jogo *j = criar_jogo();
    assert(j->mapa.largura == LARGURA && j->mapa.altura == ALTURA);
    assert(j->n_zumbis == 8 && j->n_itens == 12);
    assert(j->jogador.vida == 100 && j->jogador.fome == 100);
    assert(j->jogador.pos.x == 1 && j->jogador.pos.y == 14);
    assert(mapa_obter(&j->mapa, -1, 1) == '#' && mapa_obter(&j->mapa, LARGURA, 1) == '#');

    int visitado[ALTURA][LARGURA] = {0};
    Posicao fila[ALTURA * LARGURA] = {j->jogador.pos};
    int fim = 1;
    visitado[j->jogador.pos.y][j->jogador.pos.x] = 1;
    const Posicao vizinhos[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (int inicio = 0; inicio < fim; inicio++) {
        for (int i = 0; i < 4; i++) {
            int x = fila[inicio].x + vizinhos[i].x;
            int y = fila[inicio].y + vizinhos[i].y;
            if (mapa_livre(&j->mapa, x, y) && !visitado[y][x]) {
                visitado[y][x] = 1;
                fila[fim++] = (Posicao){x, y};
            }
        }
    }
    assert(visitado[j->mapa.heliponto.y][j->mapa.heliponto.x]);
    int contagem[N_TIPOS_ITEM] = {0};
    for (int i = 0; i < j->n_itens; i++) {
        const Item *item = &j->itens[i];
        assert(visitado[item->pos.y][item->pos.x]);
        contagem[item->tipo]++;
    }
    for (int tipo = 0; tipo < N_TIPOS_ITEM; tipo++) {
        assert(contagem[tipo] == 4);
    }
    jogo_destruir(j);
}

static void testar_invalidos(void)
{
    Jogo *j = criar_jogo();
    assert(agir(j, ACAO_MOVER, DIR_ESQUERDA) == 0);
    assert(agir(j, ACAO_ATIRAR, DIR_CIMA) == 0);
    assert(agir(j, ACAO_COMER, DIR_NENHUMA) == 0);
    assert(agir(j, ACAO_REMEDIO, DIR_NENHUMA) == 0);
    assert(agir(j, ACAO_INVALIDA, DIR_NENHUMA) == 0);
    assert(agir(j, ACAO_AJUDA, DIR_NENHUMA) == 0);
    assert(agir(j, ACAO_GRAVAR, DIR_NENHUMA) == 0);
    assert(j->turno == 0 && j->pontos == 0 && j->sorteios == 0);
    assert(j->jogador.vida == 100 && j->jogador.fome == 100);
    assert(j->jogador.pos.x == 1 && j->jogador.pos.y == 14);
    jogo_destruir(j);
}

static void testar_tiros(void)
{
    Jogo *j = criar_arena();
    j->turno = 1;
    j->jogador.mochila[ITEM_MUNICAO] = 3;
    assert(entidades_adicionar_zumbi(j, 8, 3) == 0);
    assert(entidades_adicionar_zumbi(j, 9, 3) == 0);
    assert(agir(j, ACAO_ATIRAR, DIR_DIREITA) == 1);
    assert(!j->zumbis[0].ativo && j->zumbis[1].ativo);
    assert(j->jogador.mochila[ITEM_MUNICAO] == 2);
    assert(j->jogador.abates == 1 && j->pontos == 10);

    j->turno = 3;
    assert(agir(j, ACAO_ATIRAR, DIR_DIREITA) == 1);
    assert(j->zumbis[1].ativo && j->pontos == 10);
    mapa_definir(&j->mapa, 4, 3, '#');
    j->jogador.pos.x = 3;
    j->turno = 5;
    assert(agir(j, ACAO_ATIRAR, DIR_DIREITA) == 1);
    assert(j->zumbis[1].ativo && j->jogador.mochila[ITEM_MUNICAO] == 0);
    jogo_destruir(j);
}

static void testar_socos_e_ia(void)
{
    Jogo *j = criar_arena();
    assert(entidades_adicionar_zumbi(j, 3, 3) == 0);
    assert(agir(j, ACAO_MOVER, DIR_DIREITA) == 1);
    assert(j->jogador.pos.x == 2 && j->zumbis[0].vida == 2 && j->jogador.vida == 90);
    assert(agir(j, ACAO_MOVER, DIR_DIREITA) == 1);
    assert(j->zumbis[0].vida == 1 && j->jogador.vida == 90);
    assert(agir(j, ACAO_MOVER, DIR_DIREITA) == 1);
    assert(!j->zumbis[0].ativo && j->jogador.abates == 1 && j->pontos == 10);
    assert(j->jogador.pos.x == 2 && j->jogador.vida == 90);
    assert(agir(j, ACAO_MOVER, DIR_DIREITA) == 1 && j->jogador.pos.x == 3);
    jogo_destruir(j);

    j = criar_arena();
    j->jogador.pos = (Posicao){5, 5};
    mapa_definir(&j->mapa, 7, 6, '#');
    assert(entidades_adicionar_zumbi(j, 8, 6) == 0);
    assert(entidades_adicionar_zumbi(j, 7, 5) == 0);
    assert(agir(j, ACAO_ESPERAR, DIR_NENHUMA) == 1);
    assert(j->zumbis[0].pos.x == 8 && j->zumbis[0].pos.y == 5);
    assert(j->zumbis[1].pos.x == 6 && j->zumbis[1].pos.y == 5);
    assert(agir(j, ACAO_ESPERAR, DIR_NENHUMA) == 1);
    assert(j->zumbis[0].pos.x == 8 && j->jogador.vida == 100);
    assert(agir(j, ACAO_ESPERAR, DIR_NENHUMA) == 1);
    assert(j->zumbis[0].pos.x == 7 && j->jogador.vida == 90);
    jogo_destruir(j);
}

static void testar_recursos(void)
{
    Jogo *j = criar_arena();
    j->jogador.vida = 85;
    j->jogador.fome = 45;
    j->jogador.mochila[ITEM_MUNICAO] = 6;
    assert(entidades_adicionar_item(j, 3, 3, ITEM_REMEDIO) == 0);
    assert(entidades_adicionar_item(j, 4, 3, ITEM_COMIDA) == 0);
    assert(entidades_adicionar_item(j, 5, 3, ITEM_MUNICAO) == 0);
    assert(agir(j, ACAO_MOVER, DIR_DIREITA) == 1 && j->jogador.mochila[ITEM_REMEDIO] == 1);
    assert(agir(j, ACAO_MOVER, DIR_DIREITA) == 1 && j->jogador.mochila[ITEM_COMIDA] == 1);
    assert(agir(j, ACAO_REMEDIO, DIR_NENHUMA) == 1 && j->jogador.vida == 100);
    assert(agir(j, ACAO_COMER, DIR_NENHUMA) == 1 && j->jogador.fome == 77);
    assert(agir(j, ACAO_MOVER, DIR_DIREITA) == 1 && j->itens[2].ativo);
    assert(j->jogador.mochila[ITEM_MUNICAO] == 6 && j->pontos == 10);
    assert(agir(j, ACAO_ATIRAR, DIR_DIREITA) == 1 && !j->itens[2].ativo);
    assert(j->jogador.mochila[ITEM_MUNICAO] == 9 && j->pontos == 15);

    j->jogador.mochila[ITEM_COMIDA] = 2;
    j->jogador.fome = 99;
    assert(agir(j, ACAO_COMER, DIR_NENHUMA) == 1);
    assert(j->jogador.fome == 98 && j->jogador.mochila[ITEM_COMIDA] == 1);
    jogo_destruir(j);
}

static void testar_desfechos(void)
{
    Jogo *j = criar_arena();
    j->turno = 58;
    j->jogador.pos = j->mapa.heliponto;
    assert(agir(j, ACAO_ESPERAR, DIR_NENHUMA) == 1 && j->estado == PARTIDA_ANDAMENTO);
    assert(agir(j, ACAO_ESPERAR, DIR_NENHUMA) == 1 && j->estado == PARTIDA_VITORIA);
    assert(j->turno == 60 && jogo_pontuacao(j) == 760);
    assert(agir(j, ACAO_ESPERAR, DIR_NENHUMA) == 0 && jogo_pontuacao(j) == 760);
    jogo_destruir(j);

    j = criar_arena();
    j->turno = 119;
    j->jogador.pos = j->mapa.heliponto;
    j->jogador.pos.x--;
    assert(agir(j, ACAO_MOVER, DIR_DIREITA) == 1 && j->estado == PARTIDA_VITORIA);
    assert(j->turno == 120 && jogo_pontuacao(j) == 700);
    jogo_destruir(j);

    j = criar_arena();
    j->turno = 119;
    j->pontos = 15;
    assert(agir(j, ACAO_ESPERAR, DIR_NENHUMA) == 1 && j->estado == PARTIDA_DERROTA_TEMPO);
    assert(j->turno == 120 && jogo_pontuacao(j) == 15);
    jogo_destruir(j);

    j = criar_arena();
    j->turno = 59;
    j->jogador.pos = j->mapa.heliponto;
    j->jogador.vida = 3;
    j->jogador.fome = 2;
    assert(agir(j, ACAO_ESPERAR, DIR_NENHUMA) == 1 && j->estado == PARTIDA_DERROTA_VIDA);
    assert(j->turno == 60 && j->jogador.vida == 0 && j->jogador.fome == 0 && j->pontos == 0);
    jogo_destruir(j);
}

static void comparar_arquivos(const char *a, const char *b)
{
    FILE *primeiro = fopen(a, "rb");
    FILE *segundo = fopen(b, "rb");
    assert(primeiro != NULL && segundo != NULL);
    int ch;
    do {
        ch = fgetc(primeiro);
        assert(ch == fgetc(segundo));
    } while (ch != EOF);
    assert(!ferror(primeiro) && !ferror(segundo));
    assert(fclose(primeiro) == 0);
    assert(fclose(segundo) == 0);
}

static void continuar(Jogo *j)
{
    assert(agir(j, ACAO_ESPERAR, DIR_NENHUMA) == 1);
    assert(agir(j, ACAO_MOVER, DIR_CIMA) == 1);
    assert(agir(j, ACAO_ESPERAR, DIR_NENHUMA) == 1);
}

static void testar_save(void)
{
    Jogo *j = criar_jogo();
    continuar(j);
    assert(partida_salvar(j, SAVE) == 0);
    assert(partida_salvar(j, SAVE) == 0);
    continuar(j);
    assert(partida_salvar(j, CONTINUO) == 0);
    jogo_destruir(j);

    j = criar_jogo();
    jogo_semear(j, 999);
    assert(partida_carregar(j, SAVE) == 0 && j->turno == 3);
    continuar(j);
    assert(partida_salvar(j, RETOMADO) == 0);
    comparar_arquivos(CONTINUO, RETOMADO);

    FILE *arq = fopen(INVALIDO, "wb");
    assert(arq != NULL);
    assert(fputs("SZ-SAVE 1\nturno 10\npontos 0\nestado 0\n"
                 "jogador 1 14 100 100 0 0 0 0\nzumbis 33\n", arq) >= 0);
    assert(fclose(arq) == 0);
    assert(partida_carregar(j, INVALIDO) == -1);
    assert(partida_salvar(j, RETOMADO) == 0);
    comparar_arquivos(CONTINUO, RETOMADO);

    assert(remove(SAVE) == 0);
    assert(remove(CONTINUO) == 0);
    assert(remove(RETOMADO) == 0);
    assert(remove(INVALIDO) == 0);
    jogo_destruir(j);
}

static void testar_ranking(void)
{
    int n;
    RegistroRanking *lista = ranking_carregar(RANKING, &n);
    assert(lista != NULL && n == 0);
    for (int i = 0; i < 9; i++) {
        RegistroRanking novo = {.nome = "Teste", .pontos = i * 10, .turnos = 60, .venceu = 1};
        ranking_inserir(lista, &n, novo);
    }
    assert(n == 5 && lista[0].pontos == 80 && lista[4].pontos == 40);
    assert(ranking_salvar(RANKING, lista, n) == 0);
    free(lista);

    lista = ranking_carregar(RANKING, &n);
    assert(lista != NULL && n == 5);
    for (int i = 0; i < n; i++) {
        assert(lista[i].pontos == (8 - i) * 10 && lista[i].venceu == 1);
        assert(strcmp(lista[i].nome, "Teste") == 0);
    }
    free(lista);
    assert(remove(RANKING) == 0);
}

static void testar_vetores(void)
{
    Jogo *j = criar_arena();
    for (int i = 0; i < 33; i++) {
        assert(entidades_adicionar_zumbi(j, i + 1, 5) == 0);
        assert(entidades_adicionar_item(j, i + 1, 5, ITEM_COMIDA) == 0);
    }
    assert(j->n_zumbis == 33 && j->cap_zumbis >= 33 && j->zumbis[32].pos.x == 33);
    assert(j->n_itens == 33 && j->cap_itens >= 33 && j->itens[32].pos.x == 33);
    jogo_destruir(j);
}

int main(void)
{
    const char *arquivos[] = {SAVE, CONTINUO, RETOMADO, INVALIDO, RANKING};
    for (size_t i = 0; i < sizeof arquivos / sizeof arquivos[0]; i++) {
        FILE *arq = fopen(arquivos[i], "rb");
        assert(arq == NULL && errno == ENOENT);
    }

    testar_mapa();
    testar_invalidos();
    testar_tiros();
    testar_socos_e_ia();
    testar_recursos();
    testar_desfechos();
    testar_save();
    testar_ranking();
    testar_vetores();
    puts("9 grupos de testes do núcleo passaram.");
    return 0;
}
