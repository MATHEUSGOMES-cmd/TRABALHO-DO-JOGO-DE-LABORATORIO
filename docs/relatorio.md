# Relatório técnico de Sobrevivência Z

## Objetivo

Implementar um jogo de turnos no terminal em que o jogador precisa administrar vida, fome e recursos enquanto atravessa o bairro. O resgate chega no turno 60 e a janela de embarque termina no turno 120.

## Organização

| Módulo | Responsabilidade |
| --- | --- |
| `main.c` | Controla menu, jogo, carregamento, encerramento, ranking e reinício. |
| `interface.c` | Lê os comandos e desenha mapa, molduras, painel de status e menus em Unicode. Aplica a paleta ANSI em terminais compatíveis. |
| `mapa.c` | Aloca e libera a matriz, contém o cenário inicial e protege o acesso às casas. |
| `entidades.c` | Aloca vetores, converte os marcadores do mapa em entidades e aplica movimento, colisões, tiros e comportamento dos zumbis. |
| `regras.c` | Cria e destrói a partida, cobra recursos, executa o turno, pontua e decide o resultado. |
| `arquivos.c` | Lê e grava o ranking e o save de texto. Valida o estado inteiro antes de efetivar o carregamento. |

A struct `Jogo` guarda terreno, jogador, vetores de zumbis e itens, contadores e a mensagem atual. `regras` coordena a criação e liberação do mapa e das entidades. A matriz é acessada diretamente apenas por `mapa.c`. Zumbis e itens usam vetores dinâmicos com expansão por `realloc`. O ranking é alocado ao abrir o arquivo e liberado pela função chamadora.

## Resolução de um turno

1. Validar o comando e executar a ação do jogador.
2. Coletar o item na casa atual quando há espaço na mochila.
3. Processar a fase dos zumbis e aplicar as mordidas.
4. Diminuir o indicador de fome e aplicar o dano, quando necessário.
5. Incrementar o turno e verificar vida, embarque e tempo, nessa ordem.

Como o incremento é o último passo, a fase dos zumbis consulta o contador anterior ao incremento. Ela ocorre com o contador em 0, 2, 4 e assim por diante. A perseguição usa distância Manhattan. Se houver empate entre os eixos, testa primeiro o horizontal. Um zumbi distante sorteia uma direção por fase com `rand()`.

## Persistência

O formato `SZ-SAVE 1` contém turno, pontos, estado, jogador, zumbis, itens e mapa. O jogo aceita saves de versão 1 que terminam após a última linha do mapa. Seus saves acrescentam `aleatorio`, com semente e total de sorteios, e `mensagem`, com o último aviso. Ao retomar, a sequência de `rand()` é avançada até o ponto gravado. Uma reprodução idêntica depende da mesma implementação de `rand()` na biblioteca C.

O carregamento monta um estado temporário e valida formato, contagens, dimensões, coordenadas, faixas e consistência do desfecho. O estado atual só é substituído após a validação completa. A gravação escreve em um arquivo temporário no mesmo diretório e o move para o destino depois de fechar com sucesso. O ranking seleciona os cinco maiores resultados e mantém a ordem de chegada em caso de empate.

## Requisitos e verificação

| Requisito | Onde está atendido |
| --- | --- |
| R1 e R2 | C11 com GCC, flags `-Wall -Wextra` e seis módulos. |
| R3 e R4 | A struct `Jogo` reúne matriz, vetores dinâmicos e entidades. As operações recebem estruturas por ponteiro. |
| R5 | Menu principal, instruções, partida, resultados e opção de reiniciar. |
| R6 e R7 | Ranking e salvamento completo em arquivos de texto. |
| R8 | Entrada por linha, coordenadas protegidas e validação do save antes de aplicar dados. |
| R9 | Casos documentados e testes do núcleo e do terminal. |
| R10 | Movimentação, combate, IA e pontuação implementados no projeto. |

[Casos de teste](../tests/casos_de_teste.md)
