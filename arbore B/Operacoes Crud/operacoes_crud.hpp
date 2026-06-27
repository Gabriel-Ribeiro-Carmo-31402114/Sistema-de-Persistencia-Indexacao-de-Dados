#ifndef OPERACOES_CRUD_HPP
#define OPERACOES_CRUD_HPP

#include <cstdio>
#include "../Carta/carta.hpp"
#include "../Indice Secundario/indice_secundario.hpp"

/*
 * ============================================================================
 *  MODULO: Operacoes Crud
 * ----------------------------------------------------------------------------
 *  Orquestra o CRUD sobre cartas.bin integrando os subsistemas:
 *    - LED          (reuso/append de slots)
 *    - Indice Sec.  (listas invertidas de 'cor' e 'custoEmMana')
 *    - Indice Prim. (B+ — apenas chamadas-stub por enquanto)
 *
 *  CatalogoCartas agrupa o handle do arquivo de dados e as duas instancias de
 *  indice secundario, para nao re-derivar caminhos em cada operacao.
 * ============================================================================
 */

struct CatalogoCartas {
    FILE *arquivoDados;            // cartas.bin aberto (rb+)
    const char *caminhoDados;      // caminho de cartas.bin (para rebuild)
    IndiceSecundario indiceCor;    // chave secundaria 'cor'
    IndiceSecundario indiceCmc;    // chave secundaria 'custoEmMana'
};

// Abre/cria o catalogo (cartas.bin + instancias dos indices secundarios).
// Retorna true em sucesso; preenche 'catalogo'.
bool crud_abrir(CatalogoCartas &catalogo, const char *caminhoDados);

// Fecha o catalogo (fecha cartas.bin).
void crud_fechar(CatalogoCartas &catalogo);

// CREATE: escolhe slot via LED (reuso) ou append, grava o registro e insere
// nas DUAS listas invertidas (cor, cmc). Tambem chama o stub do indice
// primario. Devolve o indice do slot usado, ou -1 em erro.
int crud_criar(CatalogoCartas &catalogo, const Carta &carta);

// READ por posicao: le o registro no 'indice'. Retorna true se existe e esta
// vivo; preenche 'carta'. (flagRemovido == '*' conta como inexistente.)
bool crud_ler_por_posicao(CatalogoCartas &catalogo, int indice, Carta &carta);

// UPDATE: sobrescreve o registro no 'indice' com 'novaCarta'. Se uma chave
// secundaria (cor/cmc) mudou, mantem as listas invertidas (remove a antiga,
// insere a nova). Retorna true em sucesso.
bool crud_atualizar(CatalogoCartas &catalogo, int indice, const Carta &novaCarta);

// DELETE: lapide + push na LED + remocao das DUAS listas invertidas.
// Retorna true em sucesso.
bool crud_remover(CatalogoCartas &catalogo, int indice);

// Busca todas as posicoes de registros com determinada 'cor' (lista invertida).
int crud_buscar_por_cor(CatalogoCartas &catalogo, const char *cor,
                        int *resultados, int capacidade);

// Busca todas as posicoes de registros com determinado CMC (lista invertida).
int crud_buscar_por_cmc(CatalogoCartas &catalogo, int custoEmMana,
                        int *resultados, int capacidade);

#endif // OPERACOES_CRUD_HPP
