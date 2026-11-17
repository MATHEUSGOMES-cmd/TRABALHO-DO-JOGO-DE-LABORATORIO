#ifndef ENTIDADES_H
#define ENTIDADES_H

#define CAP_ZUMBIS 8
#define CAP_ITENS 12
//#include tipos.h

int entidades_alocar(Jogo *j); // malloc dos vetores com cap zumbis e cap itens
void entidades_liberar(Jogo *j); // free vetores
void entidades_popular(Jogo *j); 
int entidades_adicionar_zumbi(Jogo *j, int x, int y); // 
int entidades_adicionar_item(Jogo *j, int x, int y, TipoItem t);
int zumbi_em(const Jogo *j, int x, int y);
int item_em(const Jogo *j, int x, int y);
int jogador_mover(Jogo *j, Direcao d);
int jogador_coletar(Jogo *j);
int jogador_atirar(Jogo *j, Direcao d);
int zumbir_ferir(jogo *j, int idx, int dano);
int zumbis_agir(Jogo *j);


#endif