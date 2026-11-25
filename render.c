/*
Ainda to trabalhando na cor da interface
os arquivos do jogos são um template feito por ia só pra eu ir conseguindo ter algo pra ver como tela
*/

#include <stdio.h>
#include "jogo.h"

/* Esse trecho está defindo as cores, a lógica é parecida com python. Achei melhor que usar um struct pro tipo cor pois só preciso por "COR" ao inves de cor.COR
Inclusive isso já estava na função limpar tela, pois tinha começado a implementar*/

#define RESET "\x1b[0m"
#define COR_CINZA "\x1b[38;5;245m"
#define COR_ESCURO "\x1b[38;5;238m"
#define COR_AZUL "\x1b[38;5;38m"
#define COR_VERDE "\x1b[38;5;107m"
#define COR_CIANO "\x1b[1;38;5;37m"
#define COR_VERMELHO "\x1b[38;5;209m"
#define COR_VERMELHO_CLARO "\x1b[38;5;160m"
#define COR_BRANCO "\x1b[1;37m"
#define COR_AMARELO "\x1b[1;38;5;222m"
#define COR_ROXO "\x1b[1;38;5;141m"


#define LATERAL ""COR_CINZA "║" RESET""
#define TETO ""COR_CINZA "▣═════════════════════════════════════════════════▣\n" RESET""
#define CHAO ""COR_ESCURO "░ " RESET""
#define PAREDE ""COR_ESCURO "■ " RESET""
#define AGUA ""COR_AZUL "≋ " RESET""
#define ARVORE ""COR_VERDE "✿ " RESET""
#define ITEM "✦ " RESET""
#define INIMIGO ""COR_VERMELHO "☠ " RESET""
#define JOGADOR ""COR_CIANO "● " RESET""

void limparTela(void) {
    printf("\x1b[2J\x1b[H");
}

/* casa simbolo ta por padrão ocupando 2 caracteres, se tirar isso aq eles vão oculpar o espaço padrão e vai desalinhar tudo */
void desenharTile(char simbolo) {
    switch (simbolo) {
        case '.': printf(CHAO); break; /* chão */
        case '#': printf(PAREDE); break; /* parede */
        case '~': printf(AGUA); break; /* água */
        case 'T': printf(ARVORE); break; /* árvore */
        case 'F': printf(COR_AMARELO ITEM); break; /* item */
        case 'M': printf(COR_ROXO ITEM); break; /* item */
        case 'R': printf(COR_VERMELHO_CLARO ITEM); break; /* item */
        case 'H': printf(COR_BRANCO "H "); break; /* item */
        default:  printf("  "); break; /* vazio */
    }
}

void desenharMenu() {
    printf(TETO LATERAL "                                                  "LATERAL"\n" LATERAL "                "COR_AMARELO"Sobrevivência"RESET"                     "LATERAL"\n" LATERAL "                  "COR_VERMELHO" ■ ■ ■ ■ "RESET"                       "LATERAL"\n" LATERAL "                        "COR_VERMELHO"■"RESET"                         "LATERAL"\n" LATERAL "                      "COR_VERMELHO"■"RESET"                           "LATERAL"\n" LATERAL "                    "COR_VERMELHO"■"RESET"                             "LATERAL"\n" LATERAL "                  "COR_VERMELHO" ■ ■ ■ ■"RESET"                        "LATERAL"\n" LATERAL "                                                  "LATERAL"\n"LATERAL "                                                  "LATERAL"\n"  TETO"╔══════════════════════════════════════════════════╗\n╚══════════════════════════════════════════════════╝\n");

}

void desenharGuia() {
    printf(TETO LATERAL "   "COR_BRANCO"SOBRE O JOGADOR"RESET"              "COR_BRANCO"OBJETIVO"RESET"          "LATERAL"\n"LATERAL " seus atributos ficam    Sobreviva até o resgate  "LATERAL"\n"LATERAL " abaixo do mapa e são    chegar. E lembresse de   "LATERAL"\n"LATERAL " recarregados com um     estar no local para não  "LATERAL"\n"LATERAL " item "ITEM" daquela cor     ser deixado para trás    "LATERAL"\n"LATERAL " "COR_VERMELHO_CLARO "❤ "RESET" = vida                                        "LATERAL"\n"LATERAL " "COR_AMARELO"❖ "RESET" = fome                                        "LATERAL"\n"LATERAL " "COR_ROXO"❖ "RESET" = munição                                     "LATERAL"\n"LATERAL "                                                  "LATERAL"\n"TETO"╔══════════════════════════════════════════════════╗\n║  "JOGADOR"= Jogador     "INIMIGO" = Inimigo      " ARVORE "= Arvore    ║\n║  " CHAO "= Chão        "AGUA "= Água          "PAREDE "= parede    ║\n╚══════════════════════════════════════════════════╝\n");
}

void escolherOlho(int tempo) {
    printf("\n║");
    switch (tempo / 24) {
        case 0: printf("☉ L ☉ "); break;
        case 1: printf("☉´L`☉ "); break;
        case 2: printf(COR_BRANCO"☉"RESET" L "COR_BRANCO"☉"RESET" "); break;
        case 3: printf(COR_BRANCO"☉"RESET"´L`"COR_BRANCO"☉"RESET" "); break;
        case 4: printf("⚈´L`⚈ "); break;
        default: printf("x L x "); break;
    }
    printf("║");
}

void escolherBoca(int tempo) {
    printf("\n║ ");
    tempo = tempo / 24;
    switch (tempo) {
        case 2: printf(" -- "); break;
        case 3:
        case 4: printf(" ~~ "); break;
        case 5: printf("    "); break;
        default: printf(" __ "); break;
    }
    printf(" ║");
}

void desenharJogo(Jogo *jogo) {
    limparTela();
    /* aqui ele vai receber a posição de cada coisa e desenhar */
    printf(TETO);
    for (int linha = 0; linha < ALTURA; linha++) {
        printf(" "LATERAL);
        for (int coluna = 0; coluna < LARGURA; coluna++) {
            if (jogo->jogador.x == coluna && jogo->jogador.y == linha) {
                printf(JOGADOR);
            } else if (buscarInimigo(jogo, coluna, linha) != -1) {
                printf(INIMIGO);
            } else {
                desenharTile(jogo->mapa[linha][coluna]);
            } /* já que o inimigo e o jogador são as unicas coisas que se mechem o else ta ai pra desenhar todo o resto do cenário (que é estático) */
            if (coluna != LARGURA - 1) {
                continue;
            }
            printf(LATERAL);
        }
        printf("\n");
    }
    printf(TETO);

    //    void desenharHud() {
            // topo
            /*
                printf("\n╔══════╦══════════════════╗");

            */
            // linha 2
            // linha 3
            // base
    //    }

    printf("╔══════╦═════════════════════════════════════╦════╗");
    escolherOlho(jogo->tempo);
    printf(COR_VERMELHO_CLARO" ❤  ");
    for (int indice = 0; indice < jogo->jogador.vida_maxima; indice++) {
        if (jogo->jogador.vida == indice) {
            printf(RESET);
        }
        printf("■");
    }
    printf(RESET);
    printf(COR_AMARELO" ❖  ");
    for (int indice = 0; indice < jogo->jogador.energia_maxima; indice++) {
        if (jogo->jogador.energia == indice) {
            printf(RESET);
        }
        printf("■");
    }
    printf(RESET);
    printf(COR_ROXO" ❖  ");
    for (int indice = 0; indice < jogo->jogador.energia_maxima; indice++) {
        if (jogo->jogador.energia == indice) {
            printf(RESET);
        }
        printf("■");
    }
    printf(" ║ ◷  ║");
    escolherBoca(jogo->tempo);
    printf(" "COR_VERMELHO_CLARO ITEM RESET "04        " COR_AMARELO ITEM RESET "04        " COR_ROXO ITEM RESET "04        ");
    printf("║%i:%02i║", 5 + (jogo->tempo >= 60 ? jogo->tempo / 60 : 0), jogo->tempo % 60);
    printf("\n╚══════╩═════════════════════════════════════╩════╝");

    printf(" %s\n\n", jogo->mensagem);

}
