#include <stdio.h>
#include <stdlib.h>
//#include <mapa.h>
#include <entidades.h>

entidades_alocar(j){
    Zumbi *temp = (Zumbi *) malloc(CAP_ZUMBIS * sizeof(Zumbi));
    if(temp == NULL){
        return -1;
    }
    j->zumbis = temp;
    free(temp);
    

    Item *tempItem = (Item*) malloc(CAP_ITENS * sizeof(Item));

    if(tempItem == NULL){
        return -1;
    }
    j->itens = tempItem;
    free(tempItem);

    return 0;
}

entidades_liberar(j){
    if(j->zumbis == NULL){
        return;
    }
    if(j->itens == NULL){
        return;
    }

    free(j->zumbis);
    free(j->itens);
}

entidades_populares(j){

}

entidades_adicionar_zumbi(j){
    
}

zumbi_em(j, x, y){
    for(int i = 0; i < CAP_ZUMBIS; i ++){
        if(*(j->zumbis->pos + i) == (x, y)){
            if(*(j->zumbis->ativo + i) == 1){
                return 1;
            }
        }
    }

    return 0;
}

item_em(j, x, y){
    for(int i = 0; i <  CAP_ITENS; i ++){
        if(*(j->itens->pos + i) == (x,y)){
            if(*(j->itens->ativo + i) == 1){
                return 1;
            }
        }
    }
    return 0;
}

zumbir_ferir(j, indice, dano){
    for(int i = 0; i < CAP_ZUMBIS; i ++){
        if(*(j->zumbi + i) == indice){
            j->zumbi->vida - 3;
            return -3;
        }
    }
}


