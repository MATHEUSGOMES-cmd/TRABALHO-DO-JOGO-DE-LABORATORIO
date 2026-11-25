# Sobrevivência Z

Sobrevivência Z é um jogo de sobrevivência por turnos desenvolvido em linguagem C para ser executado diretamente no terminal. O jogador assume o controle de um sobrevivente preso em um bairro abandonado, cercado por zumbis, obstáculos e recursos limitados.
Durante a partida, é necessário explorar o mapa, encontrar alimentos, remédios e munições, enfrentar zumbis e escolher cuidadosamente cada ação. O jogador pode se movimentar, atacar, esperar, recuperar vida, controlar a fome, coletar recursos e gravar o progresso da partida. Cada ação pode consumir um turno e influenciar diretamente as chances de sobrevivência.
O objetivo principal é chegar ao heliponto e permanecer vivo até que o resgate esteja disponível, a partir do turno 60. A partida pode terminar com uma vitória, caso o jogador consiga alcançar o resgate, ou com uma derrota, caso fique sem vida, não consiga controlar a fome ou ultrapasse o limite de 120 turnos longe do heliponto.

Além da campanha principal, o jogo possui sistema de salvamento e carregamento de partidas, ranking de pontuações. Dessa forma, sobreviver é apenas parte do desafio, pois o jogador também precisa tomar decisões estratégicas para alcançar a maior pontuação possível.


## Compilar e jogar


No terminal Linux ou Ubuntu no WSL, abra a pasta `sobrevivenciaz` e execute:
```bash

make

./sobrevivenciaz

```

Com GCC diretamente, a partir da mesma pasta:
```bash

gcc -Wall -Wextra -std=c11 -Iinclude src/*.c -o sobrevivenciaz

./sobrevivenciaz 42

```
O argumento `42` fixa a semente aleatória. Com a mesma semente, a mesma sequência de comandos e a mesma biblioteca C, o comportamento dos zumbis é reproduzível. Sem argumento, o jogo usa a hora atual.


No PowerShell do Windows, com o GCC do MSYS2 no PATH:
```powershell

$fontes = @(Get-ChildItem src -Filter '*.c' | Select-Object -ExpandProperty FullName)

gcc -Wall -Wextra -std=c11 -Iinclude @fontes -o sobrevivenciaz.exe

.\sobrevivenciaz.exe 42

```

O jogo usa `data/` da pasta de execução quando ela já existe. Caso contrário, no Windows e no Linux (inclusive WSL), procura os dados junto ao executável e cria `data/` na primeira gravação.


## Comandos

Digite um comando e pressione Enter. Maiúsculas e minúsculas funcionam da mesma forma.

| `w`, `a`, `s`, `d` | Andar para cima, esquerda, baixo ou direita. Tentar entrar na casa de um zumbi causa 1 de dano. |

| `fw`, `fa`, `fs`, `fd` | Disparar na direção indicada. O tiro gasta 1 munição, alcança até 6 casas e para na primeira parede ou no primeiro zumbi. |

| `c` | Consumir 1 comida e recuperar até 40 no indicador de fome. |

| `r` | Consumir 1 remédio e recuperar até 30 de vida. |

| `x` | Esperar um turno. |

| `g` | Gravar a partida em `data/save.txt`. |

| `?` | Mostrar as instruções. |

| `q` | Voltar ao menu principal. |

Gravar, pedir ajuda, voltar ao menu e inserir um comando inválido preservam o turno. A opção **2** do menu carrega a última partida gravada e a opção **5** encerra o programa.

O mapa usa `●` para o jogador, `☠` para zumbis, `■` para paredes, `░` para chão, `≋` para água e `✿` para árvores. Paredes, água e árvores bloqueiam passos e tiros. Os itens aparecem como estrelas amarelas (comida), vermelhas (remédio) e violetas (munição). Em terminais sem cor, os símbolos são `✦`, `✚` e `◆` respectivamente.

O painel abaixo do mapa mostra vida, fome, munição, os três contadores da mochila e o relógio da partida, que avança de 5:00 em diante com os turnos. A expressão do rosto muda a cada 24 turnos. O mapa ganha espaço entre as casas em terminais mais largos. O comando `?` abre o guia do jogador, o objetivo, os controles e a legenda.
A coleta é automática na casa em que o jogador termina a ação. A mochila tem limite de 9 por tipo. A caixa de munição contém quatro tiros e fica no chão se eles não couberem juntos.
A cada turno de ação, o indicador de fome perde 2 pontos. Se ele chegar a zero, você perde 3 de vida por turno. O heliponto aparece como `h` enquanto o resgate está a caminho e como `H` a partir do turno 60. O jogo termina se você ficar sem vida ou se o turno 120 acabar longe do heliponto.

Abates valem 10 pontos e itens valem 5. Uma vitória soma 500 pontos, mais 2 por ponto de vida restante e 1 por turno restante. Vitórias e derrotas disputam os cinco lugares do ranking em `data/ranking.txt`.


## Código e testes

| `src/` e `include/` | Seis módulos em C e seus cabeçalhos. |

| `data/` | Salvamento e ranking, criados durante o uso. |

| `tests/` | Testes do núcleo, testes de terminal e entradas reproduzíveis. |

| `docs/` | Relatório técnico e registro de contribuições. |

Execute `make test` em um ambiente com shell POSIX. No Windows com GNU Make do MinGW, `mingw32-make teste-nucleo` compila e executa os testes em C.

- [Casos de teste](tests/casos_de_teste.md)

- [Relatório técnico](docs/relatorio.md)

- [Registro de contribuições](docs/contribuicoes.md)