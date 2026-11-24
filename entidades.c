#include <stdio.h>
#include <stdlib.h>
#include "entidades.h"
//#include "mapa.h"

int entidades_alocar(Jogo *j){ // ALOCACAO DOS VETORES DAS ENTIDADES

    Zumbi *tempZumbi = (Zumbi*) malloc(CAP_ZUMBIS * sizeof(Zumbi));
    if(tempZumbi == NULL){
        return -1;
    }
    j->zumbis = tempZumbi;

    j->n_zumbis = 0;
    j->cap_zumbis = CAP_ZUMBIS;

    Item *tempItem = (Item*) malloc(CAP_ITENS * sizeof(Item));
    if(tempItem == NULL){
        free(j->zumbis);
        j->zumbis = NULL;
        return -1;
    }
    j->itens = tempItem;

    j->n_itens = 0;
    j->cap_itens = CAP_ITENS;

    return 0;
}

void entidades_liberar(Jogo *j){
    free(j->zumbis);
    free(j->itens);

    j->zumbis = NULL;
    j->itens = NULL;

    j->n_zumbis = 0;
    j->cap_zumbis = 0;

    j->n_itens = 0;
    j->cap_itens = 0;
}



int entidades_adicionar_zumbi(Jogo *j, int x, int y){ // adicionar zumbi no mapa

    if (j->n_zumbis == j->cap_zumbis){
        int nova_cap = j->cap_zumbis * 2;

        Zumbi *temp = (Zumbi *)realloc(j->zumbis, nova_cap * sizeof(Zumbi));
        if(temp == NULL){
            return -1;
        }
        
        j->zumbis = temp;
        j->cap_zumbis = nova_cap;
    }

    Zumbi *aux = j->zumbis + j->n_zumbis;
    
    aux->vida = VIDA_ZUMBI;
    aux->pos.x = x;
    aux->pos.y = y;
    aux->ativo = 1;
            
    j->n_zumbis++;
    return 0;
}

int entidades_adicionar_item(Jogo *j, int x, int y, TipoItem t){ // adicionar item no mapa
    
    if(j->n_itens == j->cap_itens){
        int nova_cap = j->cap_itens * 2;

        Item *temp = (Item *) realloc(j->itens, nova_cap * sizeof(Item));
        if(temp == NULL){
            return -1;
        }

        j->itens = temp;
        j->cap_itens = nova_cap;
    }

    Item *aux = j->itens + j->n_itens;

    aux->pos.x = x;
    aux->pos.y = y;
    aux->tipo = t;
    aux->ativo = 1;

    j->n_itens++;
    return 0;
}

void entidades_popular(Jogo *j){//adicionar as entidades no mapa

    for (int y = 0; y < j->mapa.altura; y++){
        for (int x = 0; x < j->mapa.largura; x++){
            char c = mapa_obter(&j->mapa, x, y);

            switch (c){
                case 'P':
                    j->jogador.pos.x = x;
                    j->jogador.pos.y = y;
                    j->jogador.vida = VIDA_MAX;
                    j->jogador.fome = FOME_MAX;
                    j->jogador.abates = 0;
                    for (int t = 0; t < N_TIPOS_ITEM; t++){
                        j->jogador.mochila[t] = 0;
                    }
                    mapa_definir(&j->mapa, x, y, '.');
                    break;

                case 'E':
                    entidades_adicionar_zumbi(j, x, y);
                    mapa_definir(&j->mapa, x, y, '.');
                    break;

                case 'F':
                    entidades_adicionar_item(j, x, y, ITEM_COMIDA);
                    mapa_definir(&j->mapa, x, y, '.');
                    break;

                case 'R':
                    entidades_adicionar_item(j, x, y, ITEM_REMEDIO);
                    mapa_definir(&j->mapa, x, y, '.');
                    break;

                case 'M':
                    entidades_adicionar_item(j, x, y, ITEM_MUNICAO);
                    mapa_definir(&j->mapa, x, y, '.');
                    break;

                default:
                    break; // '#', '.' e 'H' ficam como terreno 
            }
        }
    }
}


int zumbi_em(const Jogo *j, int x, int y){ // ve se tem zumbi na coordenada passada

    const Zumbi *aux = j->zumbis;
    
    for(int i = 0; i < j->n_zumbis; i++){
        if(aux->pos.x == x && aux->pos.y == y){
            if(aux->ativo == 1){
                return i;
            }
        }
        aux++;
    }
    return -1;
}

int item_em(const Jogo *j, int x, int y){ // vê se tem item na coordenada passada

    const Item *aux = j->itens;

    for(int i = 0; i < j->n_itens; i++){
        if(aux->pos.x == x && aux->pos.y == y){
            if(aux->ativo == 1){
                return i;
            }
        }
        aux++;
    }
    
    return -1;
}

int jogador_coletar(Jogo *j){ // coletar item se tiver na casa onde o jogador está

    int indice = item_em(j, j->jogador.pos.x, j->jogador.pos.y);

    if(indice == -1){
        return 0; //  nao tem item
    }

    TipoItem tipo = j->itens[indice].tipo;

    if(j->jogador.mochila[tipo] >= MOCHILA_MAX){
        return 0; // mochila cheia
    }

    j->jogador.mochila[tipo] += 1;
    j->itens[indice].ativo = 0;

    return 1; // PEGOU O ITEM

}

static void direcao_jogador(Direcao d, int *dx, int *dy){
    *dx = 0;
    *dy = 0;

    if( d == DIR_CIMA){
        *dy = -1;
    }else if( d == DIR_BAIXO){
        *dy = 1;
    }else if(d == DIR_ESQUERDA){
        *dx = -1;
    }else if(d == DIR_DIREITA){
        *dx = 1;
    }

}

int jogador_mover(Jogo *j, Direcao d){ // movimento do jogador

    if(d == DIR_NENHUMA){
        return 0;
    }

    int dx, dy;
    direcao_jogador(d, &dx, &dy); 

    int destinox = j->jogador.pos.x + dx;
    int destinoy = j->jogador.pos.y + dy;

    int indice = zumbi_em(j, destinox, destinoy);

    if(indice != -1){
        if(zumbi_ferir(j, indice, DANO_SOCO) == 1){
            return 3; // matou zumbi
        }
        return 2; // zumbi continua vivo
    }

    if(mapa_livre(&j->mapa, destinox, destinoy)){
        j->jogador.pos.x = destinox;
        j->jogador.pos.y = destinoy;
        return 1; //jogador andou
    }

    return 0; //parede
    
}

int jogador_atirar(Jogo *j, Direcao d) { // ACAO DE ATIRAR
    if(d == DIR_NENHUMA){
        return -1;
    }

    int dx, dy;
    direcao_jogador(d, &dx, &dy);

    int x = j->jogador.pos.x;
    int y = j->jogador.pos.y;

    for(int i = 0; i < ALCANCE_TIRO; i++){
        x+= dx;
        y+= dy;

        if(!mapa_livre(&j->mapa, x, y)){
            return -1; // parede
        }

        int indice = zumbi_em(j, x, y);
        if(indice != -1){
            zumbi_ferir(j, indice, DANO_TIRO);
            return indice; // acerta o zumbi
        }
    }

    return -1;

}

int zumbi_ferir(Jogo *j, int indice, int dano){ // acao de dano contra zumbi
    if(indice < 0 || indice >= j->n_zumbis){
        return 0; // indice invalido
    }

    Zumbi *z = j->zumbis + indice;

    z->vida = z->vida - dano;

    if(z->vida <= 0){
        z->ativo = 0; // zumbi morre ao chegar em 0 ou menos
        return 1;
    }
    
    return 0;
}

static int casa_disponivel(const Jogo *j, int x, int y){//retorna 1 se tiiver livre, ou 0 se n estiver
    if(!mapa_livre(&j->mapa, x, y)){// QUANDO TEM ALGO IMPEDINDO DE ANDAR, PAREDE, ARVORE ETC
        return 0;
    }
    if(zumbi_em(j, x, y) != -1){
        return 0; // TEM ZUMBI
    }
    if(x == j->jogador.pos.x && y == j->jogador.pos.y){
        return 0; // MESMA COORDENADA DO JOGADOR
    }

    return 1;
}

int zumbis_agir(Jogo *j){ 
    
    int mordidas = 0;
    
    if(j->turno  % 2 != 0){
        return 0; // zumbis andam a cada 2 turnos
    }

    int jogadorx = j->jogador.pos.x;
    int jogadory = j->jogador.pos.y;
    
    for(int i = 0; i < j->n_zumbis; i++){
        Zumbi *z = j->zumbis + i;

        if(z->ativo == 0){
            continue;
        }

        int distanciax = jogadorx - z->pos.x;
        int distanciay = jogadory - z->pos.y;

        int distancia = abs(distanciax) + abs(distanciay);

        int passox = 0;
        if(distanciax > 0){
            passox = 1;
        }else if(distanciax < 0){
            passox = -1;
        }

        int passoy = 0;
        if(distanciay > 0){
            passoy = 1;
        }else if(distanciay < 0){
            passoy = -1;
        }

        int x1, y1, x2, y2;
        
        if(distancia == 1){
            mordidas++;
            continue;
        }else if(distancia <= ALCANCE_VISAO){
            
            if(abs(distanciax) >= abs(distanciay)){// anda na horizontal
                x1 = z->pos.x + passox;
                y1 = z->pos.y;

                x2 = z->pos.x;
                y2 = z->pos.y + passoy;
                
                
            }else{
                x1 = z->pos.x; 
                y1 = z->pos.y + passoy;

                x2 = z->pos.x + passox;
                y2 = z->pos.y;
            }

            if(casa_disponivel(j, x1, y1) == 1){
                z->pos.x = x1;
                z->pos.y = y1;
            }else if(casa_disponivel(j,x2,y2) == 1){
                z->pos.x = x2;
                z->pos.y = y2;
            }
        }else{
            int direcao = rand() % 4; // dir recebe restos de 4 (0, 1, 2 oou 3)
            int dx = 0, dy = 0; // dy vertical, dx horizontal

            if(direcao == 0){
                dy = -1;
            }else if(direcao == 1){
                dy = 1;
            }else if(direcao == 2){
                dx = 1;
            }else{
                dx = -1;
            }

            if(casa_disponivel(j, z->pos.x + dx, z->pos.y + dy) == 1){
                z->pos.x += dx;
                z->pos.y += dy;
            }
        }
    }
    return mordidas;
}
