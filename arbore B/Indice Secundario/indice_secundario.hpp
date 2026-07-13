#ifndef INDICE_SECUNDARIO_HPP
#define INDICE_SECUNDARIO_HPP

#include <cstdio>
#include "../Carta/carta.hpp"
#include "../Gerenciador De Arquivo/gerenciador_de_arquivo.hpp"

// Subsistema generico para indexacao de chaves secundarias usando lista invertida.
// Utilizado para 'cor' e 'custoEmMana' (CMC).

namespace indice_sec_const {
    constexpr int TAM_CHAVE = 16;
    constexpr int MAX_RESULTADOS_LOOKUP = 4096;
}

#pragma pack(push, 1)
struct EntradaIndice {
    char chave[indice_sec_const::TAM_CHAVE]; // Valor textual da chave
    int  cabecaLista;                        // Indice do primeiro no na lista invertida
};

struct NoListaInvertida {
    int refRegistro;                         // RRN da carta em cartas.bin
    int proximo;                             // Indice do proximo no na lista invertida
};
#pragma pack(pop)

// Abstracao que associa o indice secundario aos caminhos de arquivos e extrator da chave.
struct IndiceSecundario {
    const char *caminhoIndice;
    const char *caminhoLista;
    void (*extrairChave)(const Carta &carta, char *destino, int capacidade);
};

// Extrai a chave de cor de um registro.
void is_extrair_chave_cor(const Carta &carta, char *destino, int capacidade);

// Extrai a chave de custo em mana de um registro.
void is_extrair_chave_cmc(const Carta &carta, char *destino, int capacidade);

// Retorna uma instancia configurada para indexacao de cor.
IndiceSecundario is_criar_indice_cor();

// Retorna uma instancia configurada para indexacao de custo em mana (CMC).
IndiceSecundario is_criar_indice_cmc();

// Insere a referencia de um registro nos arquivos de indice secundario e lista invertida.
bool is_inserir(const IndiceSecundario &indice, const Carta &carta, int refRegistro);

// Procura por uma chave e preenche os resultados com as referencias encontradas.
int is_buscar_por_chave(const IndiceSecundario &indice, const char *chave,
                        int *resultados, int capacidade);

// Desencadeia a referencia de um registro da lista invertida associada a chave.
bool is_remover(const IndiceSecundario &indice, const Carta &carta, int refRegistro);

// Reconstroi os indices secundarios varrendo cartas.bin e ignorando lapides.
bool is_reconstruir(const IndiceSecundario &indice, const char *caminhoCartas);

#endif // INDICE_SECUNDARIO_HPP
