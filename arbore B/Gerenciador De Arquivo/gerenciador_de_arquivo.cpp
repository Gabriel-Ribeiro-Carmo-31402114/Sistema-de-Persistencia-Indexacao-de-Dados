#include "gerenciador_de_arquivo.hpp"

#include <cstdio>

/*
 * MODULO Gerenciador De Arquivo — implementacao.
 * Todo o I/O binario aqui usa fopen/fseek/fread/fwrite/fclose (C nativo).
 */

FILE *gda_abrir_ou_criar(const char *caminho) {
    // "rb+" exige arquivo existente. Tentamos abrir; se falhar, criamos com
    // "wb+" e gravamos o cabecalho inicial, depois reabrimos em "rb+".
    FILE *arquivo = std::fopen(caminho, "rb+");
    if (arquivo == nullptr) {
        arquivo = std::fopen(caminho, "wb+");
        if (arquivo == nullptr) {
            return nullptr;
        }
        CabecalhoArquivo cabecalho;
        cabecalho.ledCabeca      = carta_const::LED_NULO; // LED vazia
        cabecalho.totalRegistros = 0;
        if (!gda_gravar_cabecalho(arquivo, cabecalho)) {
            std::fclose(arquivo);
            return nullptr;
        }
        std::fflush(arquivo);
    }
    return arquivo;
}

void gda_fechar(FILE *arquivo) {
    if (arquivo != nullptr) {
        std::fclose(arquivo);
    }
}

bool gda_ler_cabecalho(FILE *arquivo, CabecalhoArquivo &cabecalho) {
    if (std::fseek(arquivo, 0, SEEK_SET) != 0) {
        return false;
    }
    return std::fread(&cabecalho, sizeof(CabecalhoArquivo), 1, arquivo) == 1;
}

bool gda_gravar_cabecalho(FILE *arquivo, const CabecalhoArquivo &cabecalho) {
    if (std::fseek(arquivo, 0, SEEK_SET) != 0) {
        return false;
    }
    bool ok = std::fwrite(&cabecalho, sizeof(CabecalhoArquivo), 1, arquivo) == 1;
    std::fflush(arquivo);
    return ok;
}

long gda_offset_do_indice(int indice) {
    return static_cast<long>(arquivo_const::TAM_CABECALHO) +
           static_cast<long>(indice) * static_cast<long>(carta_const::TAMANHO_REGISTRO);
}

bool gda_ler_registro(FILE *arquivo, int indice, Carta &carta) {
    if (indice < 0) {
        return false;
    }
    if (std::fseek(arquivo, gda_offset_do_indice(indice), SEEK_SET) != 0) {
        return false;
    }
    return std::fread(&carta, sizeof(Carta), 1, arquivo) == 1;
}

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
