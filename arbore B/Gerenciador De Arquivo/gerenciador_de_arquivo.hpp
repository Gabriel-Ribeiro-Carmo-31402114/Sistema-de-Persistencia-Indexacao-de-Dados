#ifndef GERENCIADOR_DE_ARQUIVO_HPP
#define GERENCIADOR_DE_ARQUIVO_HPP

#include <cstdio>
#include "../Carta/carta.hpp"

// Gerenciador de Arquivo de dados principal (cartas.bin).
// Organiza o arquivo com um cabecalho de metadados seguido por registros sequenciais de 200 bytes.

#pragma pack(push, 1)
struct CabecalhoArquivo {
    int ledCabeca;       // indice do registro no topo da LED; -1 = vazia
    int totalRegistros;  // numero de slots gravados no arquivo (inclui lapides)
};
#pragma pack(pop)

namespace arquivo_const {
    constexpr int TAM_CABECALHO = sizeof(CabecalhoArquivo); // 8 bytes
}

// Abre ou cria o arquivo de dados. Inicializa cabecalho se for novo.
FILE *gda_abrir_ou_criar(const char *caminho);

// Fecha o arquivo.
void gda_fechar(FILE *arquivo);

// Le o cabecalho a partir do offset 0.
bool gda_ler_cabecalho(FILE *arquivo, CabecalhoArquivo &cabecalho);

// Escreve o cabecalho no offset 0.
bool gda_gravar_cabecalho(FILE *arquivo, const CabecalhoArquivo &cabecalho);

// Calcula o offset de bytes para um determinado indice de registro.
long gda_offset_do_indice(int indice);

// Le o registro no indice especificado.
bool gda_ler_registro(FILE *arquivo, int indice, Carta &carta);

// Escreve o registro no indice especificado.
bool gda_gravar_registro(FILE *arquivo, int indice, const Carta &carta);

#endif // GERENCIADOR_DE_ARQUIVO_HPP
