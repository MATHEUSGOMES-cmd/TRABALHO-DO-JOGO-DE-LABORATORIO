# Casos de teste

Execute `make test` na raiz do projeto. O roteiro compila o jogo e os testes em C por meio do Makefile e executa os casos num diretório temporário. Os arquivos de uma partida real ficam disponíveis em `data/` no diretório onde ela foi jogada.

## Testes do núcleo em C

| Caso | Preparação e ação | Resultado esperado |
| --- | --- | --- |
| Mapa inicial e limites | Criar jogo e percorrer o terreno a partir de `@`. Consultar coordenadas negativas e além da largura. | Grade 40 × 16, oito zumbis, quatro itens de cada tipo e heliponto alcançável. As consultas externas retornam parede. |
| Ação inválida | Tentar andar para a parede inicial, atirar, comer e usar remédio com mochila vazia. | Turno, vida, fome, posição e pontos iniciais são preservados. |
| Tiro, distância e parede | Atirar em um zumbi a seis casas com outro atrás. Depois atirar com um alvo a sete casas e com uma parede entre o jogador e o alvo. | O primeiro tiro elimina apenas o primeiro zumbi. Os demais tiros gastam munição e preservam o zumbi. |
| Soco e zumbis | Dar três golpes em um zumbi adjacente. Em outra arena, bloquear o eixo prioritário da perseguição. | Três golpes abatem o zumbi e somam 10 pontos. O zumbi morde em fase ativa, tenta o eixo alternativo se necessário e fica parado nos turnos ímpares do contador. |
| Recursos e mochila | Coletar comida e remédio. Usar os dois. Passar sobre uma caixa de munição com seis tiros e disparar para liberar espaço. | Itens somam 5 pontos. Vida e fome respeitam o máximo de 100. A caixa fica disponível até caberem os quatro tiros. |
| Resgate, tempo e vida | Esperar no heliponto até o turno 60. Chegar no turno 120. Esperar até 120 fora do heliponto. Zerar a vida no H no turno 60. | Vitória nos turnos 60 e 120 com 760 e 700 pontos de bônus, respectivamente, quando a vida está cheia. Derrota por tempo longe de H e por vida quando a saúde zera. |
| Salvar e recuperar | Comparar uma execução contínua com outra que salva e recarrega. Tentar carregar uma contagem de 33 zumbis sobre um jogo válido. | Saves finais idênticos após a retomada. O save inválido falha e o jogo válido permanece idêntico. |
| Ranking | Consultar arquivo ausente, inserir nove registros e recarregar os dados. | A lista começa vazia e conserva os cinco maiores resultados em ordem decrescente. |
| Vetores dinâmicos | Adicionar 33 zumbis e 33 itens a uma arena vazia. | Ambos os vetores crescem e preservam as posições inseridas. |

## Testes de terminal

| Caso | Entrada ou preparação | Resultado esperado |
| --- | --- | --- |
| Menu e comandos | `entradas/comandos_invalidos.txt` | Mensagens para opção inválida, linha vazia, letra desconhecida, linha longa, mochila vazia e parede. Ajuda sem turno. Apenas `W` e `x` avançam até o turno 2. |
| Retomada | `partida_continua.txt` e `gravar_e_sair.txt` → `retomar.txt` | Mesmo save final, inclusive com uma semente diferente no programa que recarrega. |
| Save ausente ou inválido | Apagar o save de teste, escrever contagem fora do intervalo ou usar coordenada negativa. | O jogo informa o problema e volta ao menu. |
| Vitória no primeiro turno de resgate | Partir do turno 59 sobre o H e esperar. Informar `Ana Vitória`. | Vitória no turno 60 com 760 pontos e o registro `Ana_Vitória 760 60 1`. |
| Vitória no último turno | Partir do turno 119 ao lado do H e entrar. | Vitória no turno 120 com 700 pontos. |
| Derrota por tempo e reinício | Esperar no turno 119 fora do H e responder `s` após registrar o resultado. | Derrota por tempo, registro no ranking e novo mapa no turno 0. |
| Derrota por fome | Começar no H no turno 59 com 3 de vida e fome 2, depois esperar. | A vida zera no turno 60 e a derrota tem prioridade. |

Para reproduzir os quatro desfechos em uma cópia do projeto, monte um arquivo `data/save.txt` com o cabeçalho `SZ-SAVE 1`, os campos do cenário e o terreno de `entradas/mapa_teste.txt`. O roteiro `testar.sh` mostra os valores e a ordem das linhas.
