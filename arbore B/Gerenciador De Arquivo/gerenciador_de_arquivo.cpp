#include "gerenciador_de_arquivo.hpp"

#include <cstdio>

// Abre o arquivo para leitura/escrita. Se nao existir, cria e grava o cabecalho inicial.
FILE *gda_abrir_ou_criar(const char *caminho) {
    FILE *arquivo = std::fopen(caminho, "rb+");
    if (arquivo == nullptr) {
        arquivo = std::fopen(caminho, "wb+");
        if (arquivo == nullptr) {
            return nullptr;
        }
        CabecalhoArquivo cabecalho;
        cabecalho.ledCabeca      = carta_const::LED_NULO;
        cabecalho.totalRegistros = 0;
        if (!gda_gravar_cabecalho(arquivo, cabecalho)) {
            std::fclose(arquivo);
            return nullptr;
        }
        std::fflush(arquivo);
    }
    return arquivo;
}

// Fecha o arquivo se estiver aberto.
void gda_fechar(FILE *arquivo) {
    if (arquivo != nullptr) {
        std::fclose(arquivo);
    }
}

// Move o cursor para o offset 0 e le o cabecalho.
bool gda_ler_cabecalho(FILE *arquivo, CabecalhoArquivo &cabecalho) {
    if (std::fseek(arquivo, 0, SEEK_SET) != 0) {
        return false;
    }
    return std::fread(&cabecalho, sizeof(CabecalhoArquivo), 1, arquivo) == 1;
}

// Move o cursor para o offset 0 e grava o cabecalho.
bool gda_gravar_cabecalho(FILE *arquivo, const CabecalhoArquivo &cabecalho) {
    if (std::fseek(arquivo, 0, SEEK_SET) != 0) {
        return false;
    }
    bool ok = std::fwrite(&cabecalho, sizeof(CabecalhoArquivo), 1, arquivo) == 1;
    std::fflush(arquivo);
    return ok;
}

// Retorna o deslocamento em bytes correspondente ao indice do registro.
long gda_offset_do_indice(int indice) {
    return static_cast<long>(arquivo_const::TAM_CABECALHO) +
           static_cast<long>(indice) * static_cast<long>(carta_const::TAMANHO_REGISTRO);
}

// Move o cursor para o offset calculado e le um registro de 200 bytes.
bool gda_ler_registro(FILE *arquivo, int indice, Carta &carta) {
    if (indice < 0) {
        return false;
    }
    if (std::fseek(arquivo, gda_offset_do_indice(indice), SEEK_SET) != 0) {
        return false;
    }
    return std::fread(&carta, sizeof(Carta), 1, arquivo) == 1;
}

// Move o cursor para o offset calculado e grava um registro de 200 bytes.
bool gda_gravar_registro(FILE *arquivo, int indice, const Carta &carta) {
    if (indice < 0) {
        return false;
    }
    if (std::fseek(arquivo, gda_offset_do_indice(indice), SEEK_SET) != 0) {
        return false;
    }
    bool ok = std::fwrite(&carta, sizeof(Carta), 1, arquivo) == 1;
    std::fflush(arquivo);
    return ok;
}
