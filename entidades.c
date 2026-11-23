#include <stdio.h>
#include <stdlib.h>
#include "entidades.h"
//#include "mapa.h"

int entidades_alocar(Jogo *j){

    Zumbi *tempZumbi = (Zumbi*) malloc(CAP_ZUMBIS * sizeof(Zumbi));
    if(tempZumbi == NULL){
        return -1;
    }
    j->zumbis = tempZumbi;

    j->n_zumbis = 0;
    j->cap_zumbis = CAP_ZUMBIS;

    Item *tempItem = (Item*) malloc(CAP_ITENS * sizeof(Item));
    if(tempItem == NULL){
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

void entidade_popular(Jogo *j){//FAZER QUANDO MAPPA TIVER PRONTO


}

int entidades_adicionar_zumbi(Jogo *j, int x, int y){

    if (j->n_zumbis == j->cap_zumbis){
        int nova_cap = CAP_ZUMBIS * 2;

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

int entidades_adicionar_item(Jogo *j, int x, int y, TipoItem t){
    
    if(j->n_itens == j->cap_itens){
        int nova_cap = CAP_ITENS * 2;

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

int zumbi_em(const Jogo *j, int x, int y){

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

int item_em(const Jogo *j, int x, int y){

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

int jogador_coletar(Jogo *j){

    int indice = item_em(j, j->jogador.pos.x, j->jogador.pos.y);

    if(indice == -1){
        return 0;
    }

    TipoItem tipo = j->itens[indice].tipo;

    if(j->jogador.mochila[tipo] >= MOCHILA_MAX){
        return 0;
    }

    j->jogador.mochila[tipo] += 1;
    j->itens[indice].ativo = 0;

    return 1;

}

int jogador_mover(Jogo *j, Direcao d){// fazer quando mapa tiver pronto


}

int jogador_atirar(Jogo *j, Direcao d) { //fazer quando mapa tiver pronto

}

int zumbi_ferir(Jogo *j, int indice, int dano){ // fazer quando mapa tiver pronto
    if(indice < 0 || indice >= j->cap_zumbis){
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

int zumbi_agir(Jogo *j, int idx, int dano){ // fazer quando mapa tuver pronto


}
