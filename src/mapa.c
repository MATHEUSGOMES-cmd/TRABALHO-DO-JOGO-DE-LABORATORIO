#include "mapa.h"

#include <stdlib.h>
#include <string.h>

static const char mapa_padrao[ALTURA][LARGURA + 1] = {
    "########################################",
    "#.....................................H#",
    "#.~~~~#####...~~~~~####...TT#########T.#",
    "#.~.F...R.#...~.M.....#...T...Z......#.#",
    "#.~.......#...~.....Z.#...#.......R..T.#",
    "#.~~~#.####...~~~~.####...#TT#.###T###.#",
    "#......................................#",
    "#...........Z.........M....Z...........#",
    "#............................TT##.###T.#",
    "#.###.##T#...~~~~~~######....T.......#.#",
    "#.#..Z...#...~..........#....#.M.Z.F.#.#",
    "#.#...R..T...~...F....M.#....#.......#.#",
    "#.T#######...~~~~##.#####....###T##T##.#",
    "#...........R...........Z...........Z..#",
    "#@..........................F..........#",
    "########################################",
};

int mapa_alocar(Mapa *m, int largura, int altura)
{
    if (largura <= 0 || altura <= 0) {
        return -1;
    }

    m->grade = calloc((size_t)altura, sizeof *m->grade);
    if (m->grade == NULL) {
        return -1;
    }
    m->largura = largura;
    m->altura = altura;
    m->heliponto = (Posicao){-1, -1};

    for (int y = 0; y < altura; y++) {
        m->grade[y] = malloc((size_t)largura + 1);
        if (m->grade[y] == NULL) {
            mapa_destruir(m);
            return -1;
        }
        memset(m->grade[y], '.', (size_t)largura);
        m->grade[y][largura] = '\0';
    }
    return 0;
}

int mapa_criar_padrao(Mapa *m)
{
    if (mapa_alocar(m, LARGURA, ALTURA) != 0) {
        return -1;
    }

    for (int y = 0; y < ALTURA; y++) {
        memcpy(m->grade[y], mapa_padrao[y], sizeof mapa_padrao[y]);
        for (int x = 0; x < LARGURA; x++) {
            if (m->grade[y][x] == 'H') {
                m->heliponto = (Posicao){x, y};
            }
        }
    }
    return 0;
}

void mapa_destruir(Mapa *m)
{
    if (m->grade != NULL) {
        for (int y = 0; y < m->altura; y++) {
            free(m->grade[y]);
        }
        free(m->grade);
    }
    *m = (Mapa){0};
}

int mapa_dentro(const Mapa *m, int x, int y)
{
    return x >= 0 && x < m->largura && y >= 0 && y < m->altura;
}

char mapa_obter(const Mapa *m, int x, int y)
{
    return mapa_dentro(m, x, y) ? m->grade[y][x] : '#';
}

void mapa_definir(Mapa *m, int x, int y, char c)
{
    if (mapa_dentro(m, x, y)) {
        m->grade[y][x] = c;
    }
}

int mapa_livre(const Mapa *m, int x, int y)
{
    char casa = mapa_obter(m, x, y);
    return casa != '#' && casa != '~' && casa != 'T';
}
