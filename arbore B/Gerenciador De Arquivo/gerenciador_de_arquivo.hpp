#ifndef GERENCIADOR_DE_ARQUIVO_HPP
#define GERENCIADOR_DE_ARQUIVO_HPP

#include <cstdio>
#include "../Carta/carta.hpp"

/*
 * ============================================================================
 *  MODULO: Gerenciador De Arquivo
 * ----------------------------------------------------------------------------
 *  Responsavel pelo arquivo de dados principal (cartas.bin):
 *    - abrir/criar o arquivo;
 *    - ler/gravar o CABECALHO (header) que guarda a cabeca da LED;
 *    - ler/gravar registros Carta por POSICAO (indice de registro).
 *
 *  Usa EXCLUSIVAMENTE I/O nativo de C (fopen/fseek/fread/fwrite/fclose).
 *
 *  LAYOUT DO ARQUIVO cartas.bin
 *  ------------------------------------------------------------------------
 *    [ CABECALHO : CabecalhoArquivo ]  <- offset 0
 *    [ Registro 0 : Carta (200B)    ]  <- offset = TAM_CABECALHO + 0*200
 *    [ Registro 1 : Carta (200B)    ]  <- offset = TAM_CABECALHO + 1*200
 *    [ Registro 2 : Carta (200B)    ]  ...
 *
 *  O CABECALHO (CabecalhoArquivo) guarda:
 *    - ledCabeca       : indice do registro no topo da LED; -1 = LED vazia.
 *    - totalRegistros  : quantidade de slots ja gravados (vivos + lapides).
 *
 *  Posicao (indice) de um registro -> offset em bytes:
 *      offset = TAM_CABECALHO + indice * sizeof(Carta)
 *  Por isso o registro precisa ter EXATAMENTE 200 bytes (ver Carta).
 *
 *  SENTINELA DA LED: -1 (carta_const::LED_NULO) significa "lista vazia"
 *  no campo ledCabeca, e "sem proximo" no campo proximoLed de cada Carta.
 * ============================================================================
 */

#pragma pack(push, 1)
struct CabecalhoArquivo {
    int ledCabeca;       // indice do registro no topo da LED; -1 = vazia
    int totalRegistros;  // numero de slots gravados no arquivo (inclui lapides)
};
#pragma pack(pop)

namespace arquivo_const {
    constexpr int TAM_CABECALHO = sizeof(CabecalhoArquivo); // 8 bytes
}

// Abre cartas.bin para leitura/escrita binaria; se nao existir, cria com um
// cabecalho inicial (ledCabeca = -1, totalRegistros = 0). Retorna o FILE* ou
// nullptr em caso de falha. O chamador deve fechar com gda_fechar().
FILE *gda_abrir_ou_criar(const char *caminho);

// Fecha o arquivo aberto por gda_abrir_ou_criar().
void gda_fechar(FILE *arquivo);

// Le o cabecalho (offset 0) para 'cabecalho'. Retorna true em sucesso.
bool gda_ler_cabecalho(FILE *arquivo, CabecalhoArquivo &cabecalho);

// Grava o cabecalho no offset 0. Retorna true em sucesso.
bool gda_gravar_cabecalho(FILE *arquivo, const CabecalhoArquivo &cabecalho);

// Converte indice de registro -> offset em bytes dentro de cartas.bin.
long gda_offset_do_indice(int indice);

// Le o registro na posicao 'indice' para 'carta'. Retorna true em sucesso.
bool gda_ler_registro(FILE *arquivo, int indice, Carta &carta);

// Grava 'carta' na posicao 'indice'. Retorna true em sucesso.
bool gda_gravar_registro(FILE *arquivo, int indice, const Carta &carta);

#endif // GERENCIADOR_DE_ARQUIVO_HPP
