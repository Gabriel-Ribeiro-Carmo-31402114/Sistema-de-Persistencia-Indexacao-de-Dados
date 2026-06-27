#include "led.hpp"

/*
 * MODULO Led — implementacao da Lista de Espacos Disponiveis (free list LIFO).
 * A cabeca da lista vive no cabecalho de cartas.bin; o encadeamento usa o
 * campo proximoLed de cada registro lapide.
 */

bool led_empilhar_slot(FILE *arquivo, int indice) {
    if (indice < 0) {
        return false;
    }

    CabecalhoArquivo cabecalho;
    if (!gda_ler_cabecalho(arquivo, cabecalho)) {
        return false;
    }

    Carta registro;
    if (!gda_ler_registro(arquivo, indice, registro)) {
        return false;
    }

    // PUSH: o registro removido aponta para a antiga cabeca; vira a nova cabeca.
    registro.flagRemovido = carta_const::FLAG_REMOVIDO; // '*'
    registro.proximoLed   = cabecalho.ledCabeca;        // encadeia
    if (!gda_gravar_registro(arquivo, indice, registro)) {
        return false;
    }

    cabecalho.ledCabeca = indice; // nova cabeca da LED
    return gda_gravar_cabecalho(arquivo, cabecalho);
}

int led_obter_slot_livre(FILE *arquivo, bool &reutilizado) {
    CabecalhoArquivo cabecalho;
    if (!gda_ler_cabecalho(arquivo, cabecalho)) {
        reutilizado = false;
        return -1;
    }

    if (cabecalho.ledCabeca != carta_const::LED_NULO) {
        // REUSO (pop): desempilha a cabeca da LED.
        int indiceReuso = cabecalho.ledCabeca;

        Carta registro;
        if (!gda_ler_registro(arquivo, indiceReuso, registro)) {
            reutilizado = false;
            return -1;
        }

        // A nova cabeca da LED passa a ser o proximo do slot reaproveitado.
        cabecalho.ledCabeca = registro.proximoLed;
        if (!gda_gravar_cabecalho(arquivo, cabecalho)) {
            reutilizado = false;
            return -1;
        }

        reutilizado = true;
        return indiceReuso;
    }

    // APPEND: nenhuma lapide disponivel -> novo slot no fim do arquivo.
    int novoIndice = cabecalho.totalRegistros;
    cabecalho.totalRegistros += 1;
    if (!gda_gravar_cabecalho(arquivo, cabecalho)) {
        reutilizado = false;
        return -1;
    }

    reutilizado = false;
    return novoIndice;
}
