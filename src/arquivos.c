#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "arquivos.h"

#include "mapa.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <wchar.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

#define TAM_LINHA 256
#define BONUS_MAXIMO (PONTOS_VITORIA + PONTOS_POR_VIDA * VIDA_MAX + LIMITE_TURNOS)
#define PONTOS_MAXIMOS (CAP_ZUMBIS * PONTOS_ABATE + CAP_ITENS * PONTOS_ITEM + BONUS_MAXIMO)

static int diretorio_dados_existe(void)
{
    struct stat dados;
    return stat("data", &dados) == 0 && S_ISDIR(dados.st_mode);
}

void arquivos_localizar_dados(void)
{
    /* Preserva os saves de quem já executa a partir de uma pasta com data/. */
    if (diretorio_dados_existe()) {
        return;
    }

#ifdef _WIN32
    wchar_t caminho[32768];
    DWORD tamanho = GetModuleFileNameW(NULL, caminho, sizeof caminho / sizeof caminho[0]);
    if (tamanho > 0 && tamanho < sizeof caminho / sizeof caminho[0]) {
        wchar_t *barra = wcsrchr(caminho, L'\\');
        if (barra != NULL) {
            barra[1] = L'\0';
            _wchdir(caminho);
        }
    }
#elif defined(__linux__)
    char caminho[4096];
    ssize_t tamanho = readlink("/proc/self/exe", caminho, sizeof caminho - 1);
    if (tamanho > 0 && (size_t)tamanho < sizeof caminho - 1) {
        caminho[tamanho] = '\0';
        char *barra = strrchr(caminho, '/');
        if (barra != NULL) {
            barra[1] = '\0';
            chdir(caminho);
        }
    }
#endif
}

int arquivos_criar_dados(void)
{
#ifdef _WIN32
    int resultado = _mkdir("data");
#else
    int resultado = mkdir("data", 0777);
#endif
    if (resultado == 0 || (errno == EEXIST && diretorio_dados_existe())) {
        return 0;
    }
    return -1;
}

/* Retorna 0 somente no fim do arquivo, sem erro. Drena linhas inválidas por inteiro. */
static int ler_linha(FILE *arq, char linha[TAM_LINHA])
{
    size_t n = 0;
    int invalida = 0;
    int ch;

    while ((ch = fgetc(arq)) != EOF && ch != '\n') {
        if (ch == '\0' || n == TAM_LINHA - 1) {
            invalida = 1;
        } else {
            linha[n++] = (char)ch;
        }
    }
    if (invalida || ferror(arq)) {
        return -1;
    }
    if (ch == EOF && n == 0) {
        return 0;
    }
    if (n > 0 && linha[n - 1] == '\r') {
        n--;
    }
    linha[n] = '\0';
    return 1;
}

static char *proximo_campo(char **resto)
{
    char *inicio = *resto;
    while (isspace((unsigned char)*inicio)) {
        inicio++;
    }
    char *fim = inicio;
    while (*fim != '\0' && !isspace((unsigned char)*fim)) {
        fim++;
    }
    if (*fim != '\0') {
        *fim++ = '\0';
    }
    *resto = fim;
    return inicio;
}

static int ler_numeros(char *resto, int *valores, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        const char *campo = proximo_campo(&resto);
        char *fim;
        errno = 0;
        long valor = strtol(campo, &fim, 10);
        if (fim == campo || *fim != '\0' || errno != 0 || valor < INT_MIN || valor > INT_MAX) {
            return 0;
        }
        valores[i] = (int)valor;
    }
    return *proximo_campo(&resto) == '\0';
}

static int ler_campos(FILE *arq, const char *rotulo, int *valores, size_t n)
{
    char linha[TAM_LINHA];
    char *resto = linha;
    if (ler_linha(arq, linha) != 1) {
        return 0;
    }
    if (rotulo != NULL && strcmp(proximo_campo(&resto), rotulo) != 0) {
        return 0;
    }
    return ler_numeros(resto, valores, n);
}

static FILE *abrir_temporario(const char *caminho, char **temporario)
{
#ifdef _WIN32
    unsigned long pid = GetCurrentProcessId();
#else
    unsigned long pid = (unsigned long)getpid();
#endif
    size_t tamanho = strlen(caminho) + 40;
    char *nome = malloc(tamanho);
    if (nome == NULL) {
        return NULL;
    }

    for (unsigned int tentativa = 0; tentativa < 100; tentativa++) {
        snprintf(nome, tamanho, "%s.%lu.%u.tmp", caminho, pid, tentativa);
        FILE *arq = fopen(nome, "wbx");
        if (arq != NULL) {
            *temporario = nome;
            return arq;
        }
        if (errno != EEXIST) {
            break;
        }
    }
    free(nome);
    return NULL;
}

static int concluir_gravacao(FILE *arq, char *temporario, const char *caminho)
{
    int falhou = ferror(arq);
    if (fclose(arq) != 0) {
        falhou = 1;
    }
    if (!falhou) {
#ifdef _WIN32
        falhou = !MoveFileExA(temporario, caminho, MOVEFILE_REPLACE_EXISTING);
#else
        falhou = rename(temporario, caminho) != 0;
#endif
    }
    if (falhou) {
        remove(temporario);
    }
    free(temporario);
    return falhou ? -1 : 0;
}

static int nome_valido(const char *nome)
{
    size_t n = strlen(nome);
    if (n == 0 || n >= MAX_NOME) {
        return 0;
    }
    for (size_t i = 0; i < n; i++) {
        if (isspace((unsigned char)nome[i]) || iscntrl((unsigned char)nome[i])) {
            return 0;
        }
    }
    return 1;
}

RegistroRanking *ranking_carregar(const char *caminho, int *n)
{
    *n = 0;
    RegistroRanking *lista = malloc((MAX_RANKING + 1) * sizeof *lista);
    if (lista == NULL) {
        return NULL;
    }

    FILE *arq = fopen(caminho, "rb");
    if (arq == NULL) {
        if (errno == ENOENT) {
            return lista;
        }
        free(lista);
        return NULL;
    }

    char linha[TAM_LINHA];
    int leitura;
    while ((leitura = ler_linha(arq, linha)) != 0 && !ferror(arq)) {
        if (leitura != 1) {
            continue;
        }
        char nome[MAX_NOME];
        int fim_nome;
        int campos[3];
        if (sscanf(linha, "%15s%n", nome, &fim_nome) != 1
            || !isspace((unsigned char)linha[fim_nome]) || !nome_valido(nome)
            || !ler_numeros(linha + fim_nome, campos, 3) || campos[0] < 0
            || campos[1] < 0 || campos[1] > LIMITE_TURNOS || campos[2] < 0 || campos[2] > 1) {
            continue;
        }

        RegistroRanking registro = {.pontos = campos[0], .turnos = campos[1], .venceu = campos[2]};
        memcpy(registro.nome, nome, strlen(nome) + 1);
        ranking_inserir(lista, n, registro);
    }
    int falhou = ferror(arq);
    if (fclose(arq) != 0 || falhou) {
        free(lista);
        *n = 0;
        return NULL;
    }
    return lista;
}

void ranking_inserir(RegistroRanking *lista, int *n, RegistroRanking novo)
{
    int i = *n;
    while (i > 0 && lista[i - 1].pontos < novo.pontos) {
        lista[i] = lista[i - 1];
        i--;
    }
    lista[i] = novo;
    if (*n < MAX_RANKING) {
        (*n)++;
    }
}

int ranking_salvar(const char *caminho, const RegistroRanking *lista, int n)
{
    char *temporario;
    FILE *arq = abrir_temporario(caminho, &temporario);
    if (arq == NULL) {
        return -1;
    }
    for (int i = 0; i < n; i++) {
        const RegistroRanking *r = &lista[i];
        fprintf(arq, "%s %d %d %d\n", r->nome, r->pontos, r->turnos, r->venceu);
    }
    return concluir_gravacao(arq, temporario, caminho);
}

int partida_salvar(const Jogo *j, const char *caminho)
{
    if (j->n_zumbis > CAP_ZUMBIS || j->n_itens > CAP_ITENS) {
        return -1;
    }
    char *temporario;
    FILE *arq = abrir_temporario(caminho, &temporario);
    if (arq == NULL) {
        return -1;
    }

    const Jogador *p = &j->jogador;
    fprintf(arq, "SZ-SAVE 1\nturno %d\npontos %d\nestado %d\n", j->turno, j->pontos, j->estado);
    fprintf(arq, "jogador %d %d %d %d %d %d %d %d\n", p->pos.x, p->pos.y, p->vida, p->fome,
            p->abates, p->mochila[ITEM_COMIDA], p->mochila[ITEM_REMEDIO], p->mochila[ITEM_MUNICAO]);
    fprintf(arq, "zumbis %d\n", j->n_zumbis);
    for (int i = 0; i < j->n_zumbis; i++) {
        const Zumbi *z = &j->zumbis[i];
        fprintf(arq, "%d %d %d %d\n", z->pos.x, z->pos.y, z->vida, z->ativo);
    }
    fprintf(arq, "itens %d\n", j->n_itens);
    for (int i = 0; i < j->n_itens; i++) {
        const Item *item = &j->itens[i];
        fprintf(arq, "%d %d %d %d\n", item->pos.x, item->pos.y, item->tipo, item->ativo);
    }
    fprintf(arq, "mapa %d %d\n", j->mapa.largura, j->mapa.altura);
    for (int y = 0; y < j->mapa.altura; y++) {
        for (int x = 0; x < j->mapa.largura; x++) {
            fputc(mapa_obter(&j->mapa, x, y), arq);
        }
        fputc('\n', arq);
    }

    fprintf(arq, "aleatorio %u %u\nmensagem %s\n", j->semente, j->sorteios, j->mensagem);
    return concluir_gravacao(arq, temporario, caminho);
}

static int ler_jogador(FILE *arq, Jogo *j)
{
    int estado;
    int p[8];
    if (!ler_campos(arq, "turno", &j->turno, 1)
        || !ler_campos(arq, "pontos", &j->pontos, 1)
        || !ler_campos(arq, "estado", &estado, 1)
        || !ler_campos(arq, "jogador", p, 8)
        || j->turno < 0 || j->turno > LIMITE_TURNOS
        || j->pontos < 0 || j->pontos > PONTOS_MAXIMOS
        || estado < PARTIDA_ANDAMENTO || estado > PARTIDA_DERROTA_TEMPO
        || p[2] < 0 || p[2] > VIDA_MAX || p[3] < 0 || p[3] > FOME_MAX
        || p[4] < 0 || p[4] > CAP_ZUMBIS) {
        return 0;
    }

    j->estado = (EstadoPartida)estado;
    j->jogador = (Jogador){.pos = {p[0], p[1]}, .vida = p[2], .fome = p[3], .abates = p[4]};
    for (int tipo = 0; tipo < N_TIPOS_ITEM; tipo++) {
        if (p[5 + tipo] < 0 || p[5 + tipo] > MOCHILA_MAX) {
            return 0;
        }
        j->jogador.mochila[tipo] = p[5 + tipo];
    }
    return 1;
}

static int ler_zumbis(FILE *arq, Jogo *j)
{
    if (!ler_campos(arq, "zumbis", &j->n_zumbis, 1)
        || j->n_zumbis < 0 || j->n_zumbis > j->cap_zumbis) {
        return 0;
    }
    for (int i = 0; i < j->n_zumbis; i++) {
        int z[4];
        if (!ler_campos(arq, NULL, z, 4) || z[2] < 0 || z[2] > VIDA_ZUMBI
            || (z[3] != 0 && z[3] != 1) || z[3] != (z[2] > 0)) {
            return 0;
        }
        j->zumbis[i] = (Zumbi){{z[0], z[1]}, z[2], z[3]};
    }
    return 1;
}

static int ler_itens(FILE *arq, Jogo *j)
{
    if (!ler_campos(arq, "itens", &j->n_itens, 1)
        || j->n_itens < 0 || j->n_itens > j->cap_itens) {
        return 0;
    }
    for (int i = 0; i < j->n_itens; i++) {
        int item[4];
        if (!ler_campos(arq, NULL, item, 4) || item[2] < ITEM_COMIDA
            || item[2] >= N_TIPOS_ITEM || (item[3] != 0 && item[3] != 1)) {
            return 0;
        }
        j->itens[i] = (Item){{item[0], item[1]}, (TipoItem)item[2], item[3]};
    }
    return 1;
}

static int ler_mapa(FILE *arq, Mapa *mapa)
{
    int dimensoes[2];
    if (!ler_campos(arq, "mapa", dimensoes, 2) || dimensoes[0] != LARGURA
        || dimensoes[1] != ALTURA || mapa_alocar(mapa, LARGURA, ALTURA) != 0) {
        return 0;
    }

    int helipontos = 0;
    char linha[TAM_LINHA];
    for (int y = 0; y < ALTURA; y++) {
        if (ler_linha(arq, linha) != 1 || strlen(linha) != LARGURA) {
            return 0;
        }
        for (int x = 0; x < LARGURA; x++) {
            char casa = linha[x];
            if (casa == 'H') {
                mapa->heliponto = (Posicao){x, y};
                helipontos++;
            } else if (casa != '.' && casa != '#' && casa != '~' && casa != 'T') {
                return 0;
            }
            mapa_definir(mapa, x, y, casa);
        }
    }
    return helipontos == 1;
}

static int ler_unsigned(char **resto, unsigned int *valor)
{
    const char *campo = proximo_campo(resto);
    if (*campo < '0' || *campo > '9') {
        return 0;
    }
    char *fim;
    errno = 0;
    unsigned long n = strtoul(campo, &fim, 10);
    if (errno != 0 || *fim != '\0' || n > UINT_MAX) {
        return 0;
    }
    *valor = (unsigned int)n;
    return 1;
}

static int ler_complemento(FILE *arq, Jogo *j)
{
    char linha[TAM_LINHA];
    int leitura = ler_linha(arq, linha);
    if (leitura == 0) {
        strcpy(j->mensagem, "Partida carregada.");
        return 1;
    }
    char *resto = linha;
    unsigned int max_sorteios = (unsigned int)((j->turno + 1) / 2 * j->n_zumbis);
    if (leitura != 1 || strcmp(proximo_campo(&resto), "aleatorio") != 0
        || !ler_unsigned(&resto, &j->semente) || !ler_unsigned(&resto, &j->sorteios)
        || *proximo_campo(&resto) != '\0' || j->sorteios > max_sorteios
        || ler_linha(arq, linha) != 1 || strncmp(linha, "mensagem ", 9) != 0
        || strlen(linha + 9) >= sizeof j->mensagem) {
        return 0;
    }
    for (const char *p = linha + 9; *p != '\0'; p++) {
        if (iscntrl((unsigned char)*p)) {
            return 0;
        }
    }
    strcpy(j->mensagem, linha + 9);
    return ler_linha(arq, linha) == 0;
}

static int validar_posicoes(const Jogo *j)
{
    Posicao jogador = j->jogador.pos;
    if (!mapa_livre(&j->mapa, jogador.x, jogador.y)) {
        return 0;
    }

    int ocupadas[ALTURA][LARGURA] = {0};
    ocupadas[jogador.y][jogador.x] = 1;
    for (int i = 0; i < j->n_zumbis; i++) {
        const Zumbi *z = &j->zumbis[i];
        int x = z->pos.x;
        int y = z->pos.y;
        if (!mapa_livre(&j->mapa, x, y) || (z->ativo && ocupadas[y][x])) {
            return 0;
        }
        if (z->ativo) {
            ocupadas[y][x] = 1;
        }
    }

    int itens[ALTURA][LARGURA] = {0};
    for (int i = 0; i < j->n_itens; i++) {
        const Item *item = &j->itens[i];
        int x = item->pos.x;
        int y = item->pos.y;
        if (!mapa_livre(&j->mapa, x, y) || (item->ativo && itens[y][x])) {
            return 0;
        }
        if (item->ativo) {
            itens[y][x] = 1;
        }
    }
    return 1;
}

static int validar_estado(const Jogo *j)
{
    if (j->jogador.vida == 0) {
        return j->estado == PARTIDA_DERROTA_VIDA;
    }
    if (j->turno >= TURNO_RESGATE
        && j->jogador.pos.x == j->mapa.heliponto.x && j->jogador.pos.y == j->mapa.heliponto.y) {
        return j->estado == PARTIDA_VITORIA;
    }
    if (j->turno == LIMITE_TURNOS) {
        return j->estado == PARTIDA_DERROTA_TEMPO;
    }
    return j->estado == PARTIDA_ANDAMENTO;
}

int partida_carregar(Jogo *j, const char *caminho)
{
    FILE *arq = fopen(caminho, "rb");
    if (arq == NULL) {
        return -1;
    }

    Zumbi zumbis[CAP_ZUMBIS];
    Item itens[CAP_ITENS];
    Jogo novo = {
        .zumbis = zumbis, .cap_zumbis = CAP_ZUMBIS,
        .itens = itens, .cap_itens = CAP_ITENS, .semente = j->semente
    };
    char linha[TAM_LINHA];
    int valido = ler_linha(arq, linha) == 1 && strcmp(linha, "SZ-SAVE 1") == 0
        && ler_jogador(arq, &novo) && ler_zumbis(arq, &novo) && ler_itens(arq, &novo)
        && ler_mapa(arq, &novo.mapa) && validar_posicoes(&novo) && validar_estado(&novo)
        && ler_complemento(arq, &novo);
    if (fclose(arq) != 0 || !valido || novo.n_zumbis > j->cap_zumbis || novo.n_itens > j->cap_itens) {
        mapa_destruir(&novo.mapa);
        return -1;
    }

    /* Só substitui o estado original depois que todo o arquivo foi validado. */
    memcpy(j->zumbis, zumbis, (size_t)novo.n_zumbis * sizeof *zumbis);
    memcpy(j->itens, itens, (size_t)novo.n_itens * sizeof *itens);
    novo.zumbis = j->zumbis;
    novo.itens = j->itens;
    novo.cap_zumbis = j->cap_zumbis;
    novo.cap_itens = j->cap_itens;
    mapa_destruir(&j->mapa);
    *j = novo;

    srand(j->semente);
    for (unsigned int i = 0; i < j->sorteios; i++) {
        (void)rand();
    }
    return 0;
}
