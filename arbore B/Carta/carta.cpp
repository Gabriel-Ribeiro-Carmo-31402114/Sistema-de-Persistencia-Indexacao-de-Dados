#include "carta.hpp"

#include <cstdio>
#include <cstring>


void carta_inicializar(Carta &carta) {
    // Zera o registro inteiro (campos textuais ficam com '\0', ints com 0).
    std::memset(&carta, 0, sizeof(Carta));
    carta.flagRemovido = carta_const::FLAG_ATIVO;
    carta.proximoLed   = carta_const::LED_NULO;
    carta.id           = 0;
}

void carta_copiar_texto(char *destino, const char *origem, int capacidade) {
    if (capacidade <= 0) {
        return;
    }
    // Copia no maximo (capacidade - 1) bytes e garante terminacao nula.
    std::strncpy(destino, origem, static_cast<size_t>(capacidade) - 1);
    destino[capacidade - 1] = '\0';
}

void carta_imprimir(const Carta &carta) {
    std::printf(
        "  [flag=%c] id=%d  col=%d  nome=\"%s\"  cmc=%d  cor=\"%s\"  tipo=\"%s\"  rar=\"%s\"\n",
        (carta.flagRemovido == '\0' ? ' ' : carta.flagRemovido),
        carta.id,
        carta.numeroDeColecao,
        carta.nome,
        carta.custoEmMana,
        carta.cor,
        carta.tipo,
        carta.raridade);
}
