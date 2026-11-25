#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "interface.h"

#include "mapa.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

static int usar_ansi;

#define FUNDO      "\033[48;2;29;34;37m"
#define MOLDURA    "\033[38;5;245m"
#define CLARO      "\033[38;5;255m"
#define CINZA      "\033[38;5;246m"
#define ESCURO     "\033[38;5;240m"
#define CIANO      "\033[38;5;37m"
#define AZUL       "\033[38;5;38m"
#define VERDE      "\033[38;5;107m"
#define VERMELHO   "\033[38;5;209m"
#define VERMELHO_VIDA "\033[38;5;160m"
#define DOURADO    "\033[38;5;222m"
#define VIOLETA    "\033[38;5;141m"

static const struct {
    const char *glifo;
    const char *cor;
} simbolos_mapa[] = {
    ['#'] = {"■", ESCURO},
    ['.'] = {"░", ESCURO},
    ['~'] = {"≋", AZUL},
    ['T'] = {"✿", VERDE},
    ['@'] = {"●", CIANO},
    ['Z'] = {"☠", VERMELHO},
    ['F'] = {"✦", DOURADO},
    ['R'] = {"✦", VERMELHO_VIDA},
    ['M'] = {"✦", VIOLETA},
    ['h'] = {"h", CINZA},
    ['H'] = {"H", CLARO},
};

void interface_iniciar(void)
{
#ifdef _WIN32
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD modo;
    if (GetConsoleMode(console, &modo)) {
        SetConsoleOutputCP(CP_UTF8);
        usar_ansi = SetConsoleMode(console, modo | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
    if (GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &modo)) {
        SetConsoleCP(CP_UTF8);
    }
#else
    const char *term = getenv("TERM");
    usar_ansi = isatty(STDOUT_FILENO) && (term == NULL || strcmp(term, "dumb") != 0);
#endif
}

static int largura_terminal(void)
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
        return info.srWindow.Right - info.srWindow.Left + 1;
    }
#else
    struct winsize tamanho;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &tamanho) == 0) {
        return tamanho.ws_col;
    }
#endif
    return 0;
}

static void pintar(const char *texto, const char *cor)
{
    if (usar_ansi) {
        fputs(cor, stdout);
    }
    fputs(texto, stdout);
    if (usar_ansi) {
        fputs("\033[39m", stdout);
    }
}

static void abrir_linha(void)
{
    if (usar_ansi) {
        fputs(FUNDO, stdout);
    }
}

static void fechar_linha(void)
{
    if (usar_ansi) {
        fputs("\033[0m", stdout);
    }
    putchar('\n');
}

static void repetir(const char *glifo, int quantidade)
{
    for (int i = 0; i < quantidade; i++) {
        fputs(glifo, stdout);
    }
}

static void linha_moldura(const char *inicio, int largura, const char *fim)
{
    if (usar_ansi) {
        fputs(MOLDURA, stdout);
    }
    fputs(inicio, stdout);
    repetir("═", largura);
    fputs(fim, stdout);
    if (usar_ansi) {
        fputs("\033[39m", stdout);
    }
}

static void fechar_painel(int largura)
{
    abrir_linha();
    linha_moldura("╚", largura, "╝");
    fechar_linha();
}

static void quadro_simples(const char *inicio, int largura, const char *fim)
{
    abrir_linha();
    linha_moldura(inicio, largura, fim);
    fechar_linha();
}

static int largura_texto(const char *texto)
{
    int colunas = 0;
    for (const unsigned char *p = (const unsigned char *)texto; *p; p++) {
        colunas += (*p & 0xc0) != 0x80;
    }
    return colunas;
}

static void preencher_campo(const char *texto, int largura, const char *cor)
{
    pintar(texto, cor);
    repetir(" ", largura - largura_texto(texto));
}

static void linha_painel(const char *texto, int largura, const char *cor)
{
    abrir_linha();
    pintar("║ ", MOLDURA);
    preencher_campo(texto, largura - 1, cor);
    pintar("║", MOLDURA);
    fechar_linha();
}

static void abrir_painel(const char *titulo, int largura, const char *cor)
{
    abrir_linha();
    linha_moldura("╔", largura, "╗");
    fechar_linha();
    linha_painel(titulo, largura, cor);
    abrir_linha();
    linha_moldura("╠", largura, "╣");
    fechar_linha();
}

void mostrar_mensagem(const char *mensagem)
{
    pintar("▸ ", CIANO);
    puts(mensagem);
}

/* 1: linha válida; 0: longa demais ou contém NUL; -1: entrada encerrada. */
static int ler_linha(char *linha, int tam)
{
    fflush(stdout);
    /* O último NUL foi escrito por fgets, mesmo que a entrada contenha NUL. */
    memset(linha, 1, (size_t)tam);
    if (fgets(linha, tam, stdin) == NULL) {
        return -1;
    }

    int fim = tam - 1;
    while (linha[fim] != '\0') {
        fim--;
    }
    if (fim > 0 && linha[fim - 1] == '\n') {
        linha[--fim] = '\0';
    } else if (!feof(stdin)) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {
            /* Descarta o restante do mesmo comando. */
        }
        return 0;
    }
    if (fim > 0 && linha[fim - 1] == '\r') {
        linha[--fim] = '\0';
    }
    return memchr(linha, '\0', (size_t)fim) == NULL;
}

static char *aparar(char *linha)
{
    while (isspace((unsigned char)*linha)) {
        linha++;
    }
    size_t fim = strlen(linha);
    while (fim > 0 && isspace((unsigned char)linha[fim - 1])) {
        linha[--fim] = '\0';
    }
    return linha;
}

static void desenhar_menu(void);

int menu_principal(void)
{
    desenhar_menu();

    for (;;) {
        char linha[64];
        pintar("Opção: ", CIANO);
        int leitura = ler_linha(linha, sizeof linha);
        if (leitura < 0) {
            return 5;
        }
        if (leitura > 0) {
            const char *opcao = aparar(linha);
            if (*opcao >= '1' && *opcao <= '5' && opcao[1] == '\0') {
                return *opcao - '0';
            }
        }
        pintar("Opção inválida. Digite um número de 1 a 5.\n", VERMELHO);
    }
}

static void desenhar_casa(unsigned char c)
{
    if (!usar_ansi && c == 'R') {
        fputs("✚", stdout);
        return;
    }
    if (!usar_ansi && c == 'M') {
        fputs("◆", stdout);
        return;
    }
    if (c < sizeof simbolos_mapa / sizeof simbolos_mapa[0] && simbolos_mapa[c].glifo) {
        pintar(simbolos_mapa[c].glifo, simbolos_mapa[c].cor);
    } else {
        putchar(' ');
    }
}

static void entrada_legenda(unsigned char simbolo, const char *nome, int largura)
{
    desenhar_casa(simbolo);
    fputs(" = ", stdout);
    preencher_campo(nome, largura - 4, CLARO);
}

enum { LARGURA_MENU = 50 };

static void mostrar_legenda(int completa)
{
    static const unsigned char simbolos[][3] = {
        {'@', 'Z', 'T'}, {'.', '~', '#'}, {'R', 'F', 'M'}
    };
    static const char *const nomes[][3] = {
        {"Jogador", "Zumbi", "Árvore"}, {"Chão", "Água", "Parede"},
        {"Remédio", "Comida", "Munição"}
    };
    if (completa) {
        abrir_painel("LEGENDA", LARGURA_MENU, CLARO);
    } else {
        quadro_simples("╔", LARGURA_MENU, "╗");
    }
    for (int linha = 0; linha < (completa ? 3 : 2); linha++) {
        abrir_linha();
        pintar("║ ", MOLDURA);
        for (int coluna = 0; coluna < 3; coluna++) {
            entrada_legenda(simbolos[linha][coluna], nomes[linha][coluna], 16);
        }
        pintar(" ║", MOLDURA);
        fechar_linha();
    }
    if (completa) {
        linha_painel("h/H Heliponto", LARGURA_MENU, CLARO);
    }
    fechar_painel(LARGURA_MENU);
}

static void linha_guia(const char *jogador, const char *objetivo, int largura,
                       int largura_jogador, const char *cor)
{
    abrir_linha();
    pintar("║ ", MOLDURA);
    preencher_campo(jogador, largura_jogador, cor);
    preencher_campo(objetivo, largura - 1 - largura_jogador, CLARO);
    pintar("║", MOLDURA);
    fechar_linha();
}

static void desenhar_menu(void)
{
    putchar('\n');
    quadro_simples("▣", LARGURA_MENU, "▣");
    linha_painel("", LARGURA_MENU, CLARO);
    linha_painel("                 Sobrevivência", LARGURA_MENU, DOURADO);
    linha_painel("                    ■ ■ ■ ■", LARGURA_MENU, VERMELHO);
    linha_painel("                         ■", LARGURA_MENU, VERMELHO);
    linha_painel("                       ■", LARGURA_MENU, VERMELHO);
    linha_painel("                     ■", LARGURA_MENU, VERMELHO);
    linha_painel("                    ■ ■ ■ ■", LARGURA_MENU, VERMELHO);
    linha_painel("", LARGURA_MENU, CLARO);
    linha_painel("", LARGURA_MENU, CLARO);
    quadro_simples("▣", LARGURA_MENU, "▣");

    putchar('\n');
    quadro_simples("╔", LARGURA_MENU, "╗");
    linha_painel("1. Novo jogo   2. Carregar partida   3. Ranking", LARGURA_MENU, CLARO);
    linha_painel("            4. Instruções   5. Sair", LARGURA_MENU, CLARO);
    fechar_painel(LARGURA_MENU);

    putchar('\n');
    quadro_simples("▣", LARGURA_MENU, "▣");
    linha_guia("  SOBRE O JOGADOR", "     OBJETIVO", LARGURA_MENU, 24, CLARO);
    linha_guia("As barras ficam", "Sobreviva até o resgate", LARGURA_MENU, 24, CLARO);
    linha_guia("abaixo do mapa. Use", "chegar. Vá ao heliponto", LARGURA_MENU, 24, CLARO);
    linha_guia("itens da mesma cor.", "para ser resgatado.", LARGURA_MENU, 24, CLARO);
    linha_painel("♥ = Vida", LARGURA_MENU, VERMELHO_VIDA);
    linha_painel("❖ = Fome", LARGURA_MENU, DOURADO);
    linha_painel("❖ = Munição", LARGURA_MENU, VIOLETA);
    linha_painel("", LARGURA_MENU, CLARO);
    quadro_simples("▣", LARGURA_MENU, "▣");

    putchar('\n');
    mostrar_legenda(0);
}

void mostrar_instrucoes(void)
{
    enum { LARGURA_AJUDA = 69 };
    char linha[128];
    putchar('\n');
    abrir_painel("INSTRUÇÕES", LARGURA_AJUDA, CIANO);
    linha_guia("SOBRE O JOGADOR", "OBJETIVO", LARGURA_AJUDA, 30, CLARO);
    linha_guia("As barras ficam sob o mapa.", "Sobreviva até o resgate chegar.",
               LARGURA_AJUDA, 30, CLARO);
    snprintf(linha, sizeof linha, "Embarque do turno %d ao %d.",
             TURNO_RESGATE, LIMITE_TURNOS);
    linha_guia("♥ Vida     ❖ Fome", linha, LARGURA_AJUDA, 30, CLARO);
    linha_guia("❖ Munição (barra roxa)", "h: aguardando    H: resgate disponível",
               LARGURA_AJUDA, 30, CLARO);
    quadro_simples("╠", LARGURA_AJUDA, "╣");
    linha_painel("Paredes, árvores e água bloqueiam passos e tiros.",
                 LARGURA_AJUDA, CLARO);
    snprintf(linha, sizeof linha, "Passe sobre um item para coletá-lo. Limite de %d por tipo.",
             MOCHILA_MAX);
    linha_painel(linha, LARGURA_AJUDA, CLARO);
    snprintf(linha, sizeof linha, "A fome cai %d por turno. Em 0, você perde %d de vida.",
             FOME_POR_TURNO, DANO_FOME);
    linha_painel(linha, LARGURA_AJUDA, CLARO);
    linha_painel("w/a/s/d  Andar. Tentar entrar na casa de um zumbi dá um golpe.",
                 LARGURA_AJUDA, CLARO);
    snprintf(linha, sizeof linha, "fw/fa/fs/fd  Atirar na direção indicada, até %d casas.",
             ALCANCE_TIRO);
    linha_painel(linha, LARGURA_AJUDA, CLARO);
    linha_painel("c  Comer   r  Usar remédio   x  Esperar", LARGURA_AJUDA, CLARO);
    linha_painel("g  Gravar   ?  Ajuda   q  Voltar ao menu", LARGURA_AJUDA, CLARO);
    linha_painel("Gravar, abrir a ajuda e voltar ao menu não gastam turno.",
                 LARGURA_AJUDA, CLARO);
    fechar_painel(LARGURA_AJUDA);
    mostrar_legenda(1);
}

static void desenhar_barra(const char *icone, const char *cor, int valor, int maximo,
                          int largura)
{
    int segmentos = largura > 13 ? 10 : largura - 3;
    int cheias = (valor * segmentos + maximo / 2) / maximo;
    putchar(' ');
    pintar(icone, cor);
    putchar(' ');
    for (int i = 0; i < segmentos; i++) {
        pintar(i < cheias || usar_ansi ? "■" : "▪", i < cheias ? cor : CINZA);
    }
    repetir(" ", largura - 3 - segmentos);
}

static void desenhar_contagem(unsigned char item, int quantidade, int largura)
{
    putchar(' ');
    desenhar_casa(item);
    printf(" %02d", quantidade);
    repetir(" ", largura - 5);
}

static void desenhar_painel(const Jogo *j, int largura)
{
    enum { ROSTO = 6, RELOGIO = 6 };
    int centro = largura - ROSTO - RELOGIO - 2;
    int coluna = centro / 3;
    int ultima = centro - 2 * coluna;
    const Jogador *p = &j->jogador;
    static const char *const olhos[] = {
        "☉ L ☉ ", "☉´L`☉ ", "☉ L ☉ ", "☉´L`☉ ", "⚈´L`⚈ ", "x L x "
    };
    static const char *const bocas[] = {
        "  __  ", "  __  ", "  --  ", "  ~~  ", "  ~~  ", "      "
    };
    int fase = j->turno / 24;
    if (p->vida == 0 || fase > 5) {
        fase = 5;
    }
    const char *expressao = fase >= 4 ? VERMELHO : CLARO;

    abrir_linha();
    linha_moldura("╔", ROSTO, "╦");
    linha_moldura("", centro, "╦");
    linha_moldura("", RELOGIO, "╗");
    fechar_linha();

    abrir_linha();
    pintar("║", MOLDURA);
    pintar(olhos[fase], expressao);
    pintar("║", MOLDURA);
    desenhar_barra("♥", VERMELHO_VIDA, p->vida, VIDA_MAX, coluna);
    desenhar_barra("❖", DOURADO, p->fome, FOME_MAX, coluna);
    desenhar_barra("❖", VIOLETA, p->mochila[ITEM_MUNICAO], MOCHILA_MAX, ultima);
    pintar("║  ◷   ║", MOLDURA);
    fechar_linha();

    abrir_linha();
    pintar("║", MOLDURA);
    pintar(bocas[fase], expressao);
    pintar("║", MOLDURA);
    desenhar_contagem('R', p->mochila[ITEM_REMEDIO], coluna);
    desenhar_contagem('F', p->mochila[ITEM_COMIDA], coluna);
    desenhar_contagem('M', p->mochila[ITEM_MUNICAO], ultima);
    pintar("║", MOLDURA);
    printf(" %d:%02d ", 5 + j->turno / 60, j->turno % 60);
    pintar("║", MOLDURA);
    fechar_linha();

    abrir_linha();
    linha_moldura("╚", ROSTO, "╩");
    linha_moldura("", centro, "╩");
    linha_moldura("", RELOGIO, "╝");
    fechar_linha();
}

static void desenhar_mapa(const Jogo *j, int passo)
{
    static const char simbolos_itens[N_TIPOS_ITEM] = {'F', 'R', 'M'};
    char tela[ALTURA][LARGURA];

    for (int y = 0; y < ALTURA; y++) {
        for (int x = 0; x < LARGURA; x++) {
            char terreno = mapa_obter(&j->mapa, x, y);
            int borda = y == 0 || y == ALTURA - 1 || x == 0 || x == LARGURA - 1;
            if (borda && terreno == '#') {
                terreno = ' ';
            }
            tela[y][x] = terreno == 'H' && j->turno < TURNO_RESGATE ? 'h' : terreno;
        }
    }
    for (int i = 0; i < j->n_itens; i++) {
        const Item *item = &j->itens[i];
        if (item->ativo && mapa_dentro(&j->mapa, item->pos.x, item->pos.y)) {
            tela[item->pos.y][item->pos.x] = simbolos_itens[item->tipo];
        }
    }
    for (int i = 0; i < j->n_zumbis; i++) {
        const Zumbi *zumbi = &j->zumbis[i];
        if (zumbi->ativo && mapa_dentro(&j->mapa, zumbi->pos.x, zumbi->pos.y)) {
            tela[zumbi->pos.y][zumbi->pos.x] = 'Z';
        }
    }
    const Posicao p = j->jogador.pos;
    if (mapa_dentro(&j->mapa, p.x, p.y)) {
        tela[p.y][p.x] = '@';
    }

    int largura = LARGURA * passo;
    quadro_simples("▣", largura, "▣");
    for (int y = 0; y < ALTURA; y++) {
        abrir_linha();
        pintar("║", MOLDURA);
        for (int x = 0; x < LARGURA; x++) {
            desenhar_casa(tela[y][x]);
            repetir(" ", passo - 1);
        }
        pintar("║", MOLDURA);
        fechar_linha();
    }
    quadro_simples("▣", largura, "▣");
}

void desenhar_tela(const Jogo *j)
{
    if (usar_ansi) {
        fputs("\033[2J\033[H", stdout);
    }
    int passo = largura_terminal() > 2 * LARGURA + 2 ? 2 : 1;
    pintar("SOBREVIVÊNCIA Z", CIANO);
    printf("   Turno %d/%d", j->turno, LIMITE_TURNOS);
    if (j->turno < TURNO_RESGATE) {
        printf("   Resgate no turno %d\n", TURNO_RESGATE);
    } else {
        pintar("   Resgate disponível no H\n", VERDE);
    }
    desenhar_mapa(j, passo);
    putchar('\n');
    desenhar_painel(j, LARGURA * passo);
    printf("Vida %3d%%   Fome %3d%%   Abates %d   Pontos %d\n",
           j->jogador.vida, j->jogador.fome, j->jogador.abates, j->pontos);
    printf("> %s\n", j->mensagem);
}

static Direcao ler_direcao(char letra)
{
    switch (letra) {
    case 'w': return DIR_CIMA;
    case 's': return DIR_BAIXO;
    case 'a': return DIR_ESQUERDA;
    case 'd': return DIR_DIREITA;
    default:  return DIR_NENHUMA;
    }
}

Comando ler_comando(void)
{
    char linha[64];
    pintar("Comando (w/a/s/d, f+dir, c, r, x, g, q, ?): ", CIANO);
    int leitura = ler_linha(linha, sizeof linha);
    if (leitura < 0) {
        return (Comando){ACAO_SAIR, DIR_NENHUMA};
    }
    if (leitura == 0) {
        return (Comando){ACAO_INVALIDA, DIR_NENHUMA};
    }

    char *comando = aparar(linha);
    for (char *p = comando; *p != '\0'; p++) {
        *p = (char)tolower((unsigned char)*p);
    }
    size_t tamanho = strlen(comando);
    if (tamanho == 2 && comando[0] == 'f') {
        Direcao dir = ler_direcao(comando[1]);
        if (dir != DIR_NENHUMA) {
            return (Comando){ACAO_ATIRAR, dir};
        }
    }
    if (tamanho == 1) {
        Direcao dir = ler_direcao(comando[0]);
        if (dir != DIR_NENHUMA) {
            return (Comando){ACAO_MOVER, dir};
        }
        switch (comando[0]) {
        case 'c': return (Comando){ACAO_COMER, DIR_NENHUMA};
        case 'r': return (Comando){ACAO_REMEDIO, DIR_NENHUMA};
        case 'x': return (Comando){ACAO_ESPERAR, DIR_NENHUMA};
        case 'g': return (Comando){ACAO_GRAVAR, DIR_NENHUMA};
        case '?': return (Comando){ACAO_AJUDA, DIR_NENHUMA};
        case 'q': return (Comando){ACAO_SAIR, DIR_NENHUMA};
        }
    }
    return (Comando){ACAO_INVALIDA, DIR_NENHUMA};
}

void mostrar_fim(const Jogo *j)
{
    const char *texto;
    switch (j->estado) {
    case PARTIDA_VITORIA:
        texto = "VITÓRIA! Você embarcou no helicóptero.";
        break;
    case PARTIDA_DERROTA_VIDA:
        texto = "DERROTA! Sua vida chegou a zero.";
        break;
    case PARTIDA_DERROTA_TEMPO:
        texto = "DERROTA! O helicóptero partiu sem você.";
        break;
    default:
        return;
    }
    char linha[128];
    snprintf(linha, sizeof linha, "Turnos: %d   Vida: %d   Abates: %d   Pontos: %d",
             j->turno, j->jogador.vida, j->jogador.abates, j->pontos);
    putchar('\n');
    abrir_painel("RESULTADO", 56, CIANO);
    linha_painel(texto, 56, j->estado == PARTIDA_VITORIA ? VERDE : VERMELHO);
    linha_painel(linha, 56, CLARO);
    fechar_painel(56);
}

void mostrar_ranking(const RegistroRanking *lista, int n)
{
    putchar('\n');
    abrir_painel("RANKING", 56, DOURADO);
    if (n == 0) {
        linha_painel("Nenhuma partida registrada.", 56, CLARO);
    }
    for (int i = 0; i < n; i++) {
        char linha[128];
        snprintf(linha, sizeof linha, "%d. %-15s %4d pontos  %3d turnos  %s",
                 i + 1, lista[i].nome, lista[i].pontos, lista[i].turnos,
                 lista[i].venceu ? "Vitória" : "Derrota");
        linha_painel(linha, 56, lista[i].venceu ? VERDE : CLARO);
    }
    fechar_painel(56);
}

void pedir_nome(char *nome, int tam)
{
    if (tam <= 0) {
        return;
    }
    char linha[256];
    pintar("Nome para o ranking: ", CIANO);
    int leitura = ler_linha(linha, sizeof linha);
    char *entrada = leitura > 0 ? aparar(linha) : linha;
    if (leitura <= 0 || *entrada == '\0') {
        snprintf(nome, (size_t)tam, "%s", "Anonimo");
        return;
    }

    size_t n = strlen(entrada);
    if (n >= (size_t)tam) {
        n = (size_t)tam - 1;
        /* Mantém completo o último caractere quando o nome está em UTF-8. */
        while (n > 0 && ((unsigned char)entrada[n] & 0xc0) == 0x80) {
            n--;
        }
    }
    for (size_t i = 0; i < n; i++) {
        unsigned char ch = (unsigned char)entrada[i];
        nome[i] = isspace(ch) || iscntrl(ch) ? '_' : (char)ch;
    }
    nome[n] = '\0';
}

int perguntar_reiniciar(void)
{
    for (;;) {
        char linha[64];
        pintar("Jogar de novo? (s/n): ", CIANO);
        int leitura = ler_linha(linha, sizeof linha);
        if (leitura < 0) {
            return 0;
        }
        if (leitura > 0) {
            const char *resposta = aparar(linha);
            if (*resposta != '\0' && resposta[1] == '\0') {
                int letra = tolower((unsigned char)resposta[0]);
                if (letra == 's' || letra == 'n') {
                    return letra == 's';
                }
            }
        }
        pintar("Digite s para reiniciar ou n para voltar ao menu.\n", VERMELHO);
    }
}
