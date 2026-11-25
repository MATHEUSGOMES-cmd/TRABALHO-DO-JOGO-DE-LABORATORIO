#!/bin/sh
set -eu

raiz=$(CDPATH='' cd -- "$(dirname -- "$0")/.." && pwd)
jogo="$raiz/sobrevivenciaz"
nucleo="$raiz/tests/teste_nucleo"
if [ ! -x "$jogo" ] && [ -x "$jogo.exe" ]; then
    jogo="$jogo.exe"
    nucleo="$nucleo.exe"
fi
entradas="$raiz/tests/entradas"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM

cd "$tmp"
mkdir data
"$nucleo"

executar() {
    "$jogo" "$1" > saida.txt
}

conferir() {
    if ! grep -Fq -- "$1" saida.txt; then
        printf 'Saída esperada ausente: %s\n' "$1" >&2
        tail -n 25 saida.txt >&2
        exit 1
    fi
}

preparar_save() {
    {
        printf 'SZ-SAVE 1\nturno %s\npontos 0\nestado 0\njogador %s\n' "$1" "$2"
        printf 'zumbis 0\nitens 0\nmapa 40 16\n'
        cat "$entradas/mapa_teste.txt"
    } > data/save.txt
}

executar 42 < "$entradas/comandos_invalidos.txt"
conferir 'Opção inválida.'
conferir 'Comando inválido.'
conferir 'Você está sem munição.'
conferir 'Você está sem comida.'
conferir 'Você está sem remédio.'
conferir 'Caminho bloqueado.'
conferir 'Gravar, abrir a ajuda e voltar ao menu não gastam turno.'
conferir 'Turno 2/120'
[ "$(grep -Fc '> Comando inválido.' saida.txt)" -eq 3 ]
! grep -Fq 'Turno 3/120' saida.txt || exit 1

executar 42 < "$entradas/partida_continua.txt"
cp data/save.txt continuo.txt
executar 42 < "$entradas/gravar_e_sair.txt"
conferir 'Partida gravada'
grep -Fxq 'turno 3' data/save.txt
executar 999 < "$entradas/retomar.txt"
cmp continuo.txt data/save.txt

rm data/save.txt
printf '2\n5\n' | executar 42
conferir 'Partida ausente ou inválida'
printf 'SZ-SAVE 1\nturno 20\npontos 0\nestado 0\njogador 1 14 100 100 0 0 0 0\nzumbis 999999999999999999999\n' > data/save.txt
printf '2\n5\n' | executar 42
conferir 'Partida ausente ou inválida'
preparar_save 20 '-1 14 100 100 0 0 0 0'
printf '2\n5\n' | executar 42
conferir 'Partida ausente ou inválida'

preparar_save 59 '38 1 100 24 0 0 0 0'
printf '2\nx\nAna Vitória\nn\n3\n5\n' | executar 42
conferir 'VITÓRIA! Você embarcou no helicóptero.'
conferir 'Turnos: 60   Vida: 100   Abates: 0   Pontos: 760'
grep -Fxq 'Ana_Vitória 760 60 1' data/ranking.txt

preparar_save 119 '37 1 100 24 0 0 0 0'
printf '2\nd\nÚltimo Turno\nn\n5\n' | executar 42
conferir 'VITÓRIA!'
conferir 'Turnos: 120   Vida: 100   Abates: 0   Pontos: 700'

preparar_save 119 '1 14 100 24 0 0 0 0'
printf '2\nx\nSem Resgate\ns\nq\n5\n' | executar 42
conferir 'DERROTA! O helicóptero partiu sem você.'
conferir 'Turno 0/120'
grep -Fxq 'Sem_Resgate 0 120 0' data/ranking.txt

preparar_save 59 '38 1 3 2 0 0 0 0'
printf '2\nx\nSem Comida\nn\n5\n' | executar 42
conferir 'DERROTA! Sua vida chegou a zero.'
conferir 'Turnos: 60   Vida: 0   Abates: 0   Pontos: 0'

printf '7 grupos de testes do terminal passaram.\n'
