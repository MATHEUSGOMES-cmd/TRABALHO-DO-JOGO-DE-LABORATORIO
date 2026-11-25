#include "interface.h"

#include "arquivos.h"
#include "regras.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CAMINHO_SAVE "data/save.txt"
#define CAMINHO_RANKING "data/ranking.txt"

static int ler_semente(int argc, char **argv, unsigned int *semente)
{
    if (argc == 1) {
        *semente = (unsigned int)time(NULL);
        return 0;
    }
    if (argc != 2 || argv[1][0] < '0' || argv[1][0] > '9') {
        return -1;
    }
    char *fim;
    errno = 0;
    unsigned long valor = strtoul(argv[1], &fim, 10);
    if (errno != 0 || *fim != '\0' || valor > UINT_MAX) {
        return -1;
    }
    *semente = (unsigned int)valor;
    return 0;
}

static void gravar_com_aviso(Jogo *j)
{
    const char *texto = "Partida gravada em " CAMINHO_SAVE ".";
    if (arquivos_criar_dados() != 0) {
        texto = "Não foi possível preparar a pasta data/. Verifique o acesso à pasta.";
    } else if (partida_salvar(j, CAMINHO_SAVE) != 0) {
        texto = "Não foi possível gravar a partida em " CAMINHO_SAVE ". Verifique o acesso ao arquivo.";
    }
    snprintf(j->mensagem, sizeof j->mensagem, "%s", texto);
}

static void jogar(Jogo *j)
{
    desenhar_tela(j);
    while (j->estado == PARTIDA_ANDAMENTO) {
        Comando c = ler_comando();
        if (c.tipo == ACAO_SAIR) {
            return;
        }
        if (c.tipo == ACAO_AJUDA) {
            mostrar_instrucoes();
            continue;
        }
        if (c.tipo == ACAO_GRAVAR) {
            gravar_com_aviso(j);
        } else {
            jogo_turno(j, c);
        }
        desenhar_tela(j);
    }
}

static void exibir_ranking(void)
{
    int n;
    RegistroRanking *lista = ranking_carregar(CAMINHO_RANKING, &n);
    if (lista == NULL) {
        mostrar_mensagem("Não foi possível ler o ranking.");
        return;
    }
    mostrar_ranking(lista, n);
    free(lista);
}

static void registrar_partida(const Jogo *j)
{
    int n;
    RegistroRanking *lista = ranking_carregar(CAMINHO_RANKING, &n);
    if (lista == NULL) {
        mostrar_mensagem("Não foi possível ler o ranking.");
        return;
    }

    RegistroRanking novo = {
        .pontos = jogo_pontuacao(j),
        .turnos = j->turno,
        .venceu = j->estado == PARTIDA_VITORIA
    };
    pedir_nome(novo.nome, sizeof novo.nome);
    ranking_inserir(lista, &n, novo);
    if (arquivos_criar_dados() != 0 || ranking_salvar(CAMINHO_RANKING, lista, n) != 0) {
        mostrar_mensagem("Não foi possível gravar o ranking.");
    }
    mostrar_ranking(lista, n);
    free(lista);
}

static void iniciar_partidas(int carregar, unsigned int semente)
{
    int reiniciar;
    do {
        Jogo *j = jogo_criar();
        if (j == NULL) {
            mostrar_mensagem("Não foi possível criar a partida.");
            return;
        }
        jogo_semear(j, semente);
        if (carregar && partida_carregar(j, CAMINHO_SAVE) != 0) {
            mostrar_mensagem("Partida ausente ou inválida em " CAMINHO_SAVE ".");
            jogo_destruir(j);
            return;
        }
        carregar = 0;

        jogar(j);
        reiniciar = 0;
        if (j->estado != PARTIDA_ANDAMENTO) {
            mostrar_fim(j);
            registrar_partida(j);
            reiniciar = perguntar_reiniciar();
        }
        jogo_destruir(j);
    } while (reiniciar);
}

int main(int argc, char **argv)
{
    unsigned int semente;
    interface_iniciar();
    if (ler_semente(argc, argv, &semente) != 0) {
        mostrar_mensagem("Uso: sobrevivenciaz [semente inteira sem sinal]");
        return EXIT_FAILURE;
    }

    arquivos_localizar_dados();
    for (;;) {
        switch (menu_principal()) {
        case 1: iniciar_partidas(0, semente); break;
        case 2: iniciar_partidas(1, semente); break;
        case 3: exibir_ranking(); break;
        case 4: mostrar_instrucoes(); break;
        case 5: return EXIT_SUCCESS;
        }
    }
}
