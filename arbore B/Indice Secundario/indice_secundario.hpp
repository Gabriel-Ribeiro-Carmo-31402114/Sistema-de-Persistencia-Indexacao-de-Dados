#ifndef INDICE_SECUNDARIO_HPP
#define INDICE_SECUNDARIO_HPP

#include <cstdio>
#include "../Carta/carta.hpp"
#include "../Gerenciador De Arquivo/gerenciador_de_arquivo.hpp"

/*
 * ============================================================================
 *  MODULO: Indice Secundario  (indice secundario + lista invertida)
 * ----------------------------------------------------------------------------
 *  Subsistema GENERICO de indexacao secundaria, reaproveitado por DUAS chaves:
 *      - cor          (string)  -> index_sec_cor.bin + lista_inv_cor.bin
 *      - custoEmMana  (int/CMC) -> index_sec_cmc.bin + lista_inv_cmc.bin
 *
 *  Para manter DRY, a chave eh sempre normalizada para um buffer de texto de
 *  tamanho fixo (TAM_CHAVE). Para 'cor' usamos a propria string; para 'cmc'
 *  formatamos o inteiro em texto ("3", "10", ...). Assim o MESMO codigo de
 *  index/lista serve as duas chaves; o "tipo" da chave so muda na extracao
 *  (ver is_extrair_chave_*), nao na mecanica de disco.
 *
 *  Cada chave usa DOIS arquivos, ambos com I/O nativo C:
 *
 *  (1) ARQUIVO DE INDICE  (index_sec_*.bin) — mapeia valor-de-chave -> cabeca
 *      da lista invertida. Layout: sequencia de EntradaIndice gravadas em
 *      ordem de criacao (busca linear; simples e suficiente para o escopo).
 *
 *        struct EntradaIndice {
 *            char chave[TAM_CHAVE];  // valor da chave (texto, terminado em \0)
 *            int  cabecaLista;       // indice do 1o no na lista_inv_*; -1 vazio
 *        }
 *
 *  (2) ARQUIVO DA LISTA INVERTIDA (lista_inv_*.bin) — nos encadeados; cada no
 *      guarda a REFERENCIA do registro (indice em cartas.bin) que possui a
 *      chave, e o ponteiro para o proximo no que compartilha a MESMA chave.
 *
 *        struct NoListaInvertida {
 *            int refRegistro;   // indice do registro em cartas.bin
 *            int proximo;       // indice do proximo no nesta lista_inv; -1 fim
 *        }
 *
 *      Os nos sao gravados por append no fim do arquivo; o "indice de no" eh a
 *      posicao do no dentro de lista_inv_*.bin (no_offset = no_indice * 8).
 *
 *  SENTINELA: -1 = "lista vazia" (em cabecaLista) e "sem proximo" (em proximo).
 *
 *  OPERACOES: build (rebuild varrendo cartas.bin), insert (encadeia ref na
 *  cabeca da lista da chave), lookup (devolve todas as refs de uma chave) e
 *  remove (desencadeia uma ref especifica da lista da chave no delete).
 * ============================================================================
 */

namespace indice_sec_const {
    constexpr int TAM_CHAVE = 16;   // cabe "cor" (<=14) e CMC formatado
    constexpr int MAX_RESULTADOS_LOOKUP = 4096; // teto defensivo para lookup
}

#pragma pack(push, 1)
struct EntradaIndice {
    char chave[indice_sec_const::TAM_CHAVE];
    int  cabecaLista;   // indice do 1o no em lista_inv_*; -1 = vazia
};

struct NoListaInvertida {
    int refRegistro;    // indice do registro em cartas.bin
    int proximo;        // indice do proximo no na lista_inv_*; -1 = fim
};
#pragma pack(pop)

/*
 * IndiceSecundario amarra os DOIS arquivos de uma chave (indice + lista inv.)
 * e as funcoes de extracao/normalizacao da chave a partir de um Carta. Uma
 * instancia para 'cor', outra para 'custoEmMana'.
 */
struct IndiceSecundario {
    const char *caminhoIndice; // ex.: "index_sec_cor.bin"
    const char *caminhoLista;  // ex.: "lista_inv_cor.bin"
    // Extrai a chave de um Carta e a escreve normalizada (texto) em 'destino'.
    void (*extrairChave)(const Carta &carta, char *destino, int capacidade);
};

// Extratores de chave (normalizam para texto em buffer de tamanho fixo).
void is_extrair_chave_cor(const Carta &carta, char *destino, int capacidade);
void is_extrair_chave_cmc(const Carta &carta, char *destino, int capacidade);

// Fabricas das duas instancias padrao.
IndiceSecundario is_criar_indice_cor();
IndiceSecundario is_criar_indice_cmc();

// Insere a chave de 'carta' (na posicao 'refRegistro' de cartas.bin) no indice:
// localiza/cria a EntradaIndice da chave e empilha um novo no na cabeca da
// lista invertida. Retorna true em sucesso.
bool is_inserir(const IndiceSecundario &indice, const Carta &carta, int refRegistro);

// Busca por valor de chave (texto ja normalizado). Preenche 'resultados' com
// os indices de registro encontrados (ate 'capacidade'). Retorna a quantidade,
// ou -1 em erro.
int is_buscar_por_chave(const IndiceSecundario &indice, const char *chave,
                        int *resultados, int capacidade);

// Remove a referencia 'refRegistro' da lista invertida da chave de 'carta'
// (usado no delete). Desencadeia o no correspondente. Retorna true em sucesso
// (inclusive se nao havia o que remover — operacao idempotente).
bool is_remover(const IndiceSecundario &indice, const Carta &carta, int refRegistro);

// (Re)constroi os arquivos de indice/lista a partir de uma varredura completa
// de cartas.bin, considerando apenas registros vivos. Recria os arquivos do
// zero. Retorna true em sucesso.
bool is_reconstruir(const IndiceSecundario &indice, const char *caminhoCartas);

#endif // INDICE_SECUNDARIO_HPP
