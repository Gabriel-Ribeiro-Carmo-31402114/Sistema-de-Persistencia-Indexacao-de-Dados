#include "led.hpp"

// Marca um registro como removido logicamente e o insere no topo da LED.
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

    // O registro vira o novo topo da LED e aponta para o topo anterior (LIFO)
    registro.flagRemovido = carta_const::FLAG_REMOVIDO;
    registro.proximoLed   = cabecalho.ledCabeca;
    if (!gda_gravar_registro(arquivo, indice, registro)) {
        return false;
    }

    cabecalho.ledCabeca = indice;
    return gda_gravar_cabecalho(arquivo, cabecalho);
}

// Retorna o indice de um slot livre para gravação (reuso ou append).
int led_obter_slot_livre(FILE *arquivo, bool &reutilizado) {
    CabecalhoArquivo cabecalho;
    if (!gda_ler_cabecalho(arquivo, cabecalho)) {
        reutilizado = false;
        return -1;
    }

    // Se a LED nao estiver vazia, remove o elemento do topo (POP)
    if (cabecalho.ledCabeca != carta_const::LED_NULO) {
        int indiceReuso = cabecalho.ledCabeca;

        Carta registro;
        if (!gda_ler_registro(arquivo, indiceReuso, registro)) {
            reutilizado = false;
            return -1;
        }

        // O novo topo da LED passa a ser o proximo elemento da lista
        cabecalho.ledCabeca = registro.proximoLed;
        if (!gda_gravar_cabecalho(arquivo, cabecalho)) {
            reutilizado = false;
            return -1;
        }

        reutilizado = true;
        return indiceReuso;
    }

    // Caso contrario, aloca um novo slot no final do arquivo
    int novoIndice = cabecalho.totalRegistros;
    cabecalho.totalRegistros += 1;
    if (!gda_gravar_cabecalho(arquivo, cabecalho)) {
        reutilizado = false;
        return -1;
    }

    reutilizado = false;
    return novoIndice;
}
