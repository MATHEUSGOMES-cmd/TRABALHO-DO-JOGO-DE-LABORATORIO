CC = gcc
CPPFLAGS += -Iinclude
CFLAGS ?= -Wall -Wextra -std=c11

ifeq ($(OS),Windows_NT)
EXT = .exe
endif

JOGO = sobrevivenciaz$(EXT)
TESTE = tests/teste_nucleo$(EXT)
FONTES = $(wildcard src/*.c)
CABECALHOS = $(wildcard include/*.h)
FONTES_TESTE = src/mapa.c src/entidades.c src/regras.c src/arquivos.c

.PHONY: all test teste-nucleo clean

all: $(JOGO)

$(JOGO): $(FONTES) $(CABECALHOS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(FONTES) $(LDFLAGS) $(LDLIBS) -o $@

$(TESTE): tests/teste_nucleo.c $(FONTES_TESTE) $(CABECALHOS)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/teste_nucleo.c $(FONTES_TESTE) $(LDFLAGS) $(LDLIBS) -o $@

teste-nucleo: $(TESTE)
	./$(TESTE)

test: $(JOGO) $(TESTE)
	sh tests/testar.sh

clean:
	$(RM) $(JOGO) $(TESTE)
