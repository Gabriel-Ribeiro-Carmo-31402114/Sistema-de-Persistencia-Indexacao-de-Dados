#ifndef LED_HPP
#define LED_HPP

#include <cstdio>
#include "../Carta/carta.hpp"
#include "../Gerenciador De Arquivo/gerenciador_de_arquivo.hpp"

/*
 * ============================================================================
 *  MODULO: Led  (Lista de Espacos Disponiveis)
 * ----------------------------------------------------------------------------
 *  Gerenciador de espaco livre do arquivo cartas.bin. Implementa uma lista
 *  ligada SIMPLES de slots apagados (lapides), no estilo PILHA (LIFO):
 *  reaproveita sempre o slot liberado MAIS RECENTE.
 *
 *  ESTRUTURA EM DISCO (nao ha arquivo proprio da LED — ela vive embutida):
 *    - A CABECA da lista fica no campo CabecalhoArquivo.ledCabeca (cartas.bin).
 *    - O encadeamento usa o campo Carta.proximoLed de cada registro lapide.
 *    - Sentinela -1 (carta_const::LED_NULO) = "lista vazia" / "sem proximo".
 *
 *  EXEMPLO (push do slot 5, depois do slot 2):
 *      antes:  ledCabeca = -1
 *      del(5): registro[5].proximoLed = -1 ; ledCabeca = 5
 *      del(2): registro[2].proximoLed = 5  ; ledCabeca = 2
 *      lista:  ledCabeca -> 2 -> 5 -> (-1)
 *
 *      insert: pop -> retorna 2, ledCabeca passa a 5 (proximoLed de 2).
 *
 *  REMOCAO (push / led_empilhar_slot):
 *    1. marca registro.flagRemovido = FLAG_REMOVIDO ('*')
 *    2. registro.proximoLed = ledCabeca (antiga cabeca)
 *    3. ledCabeca = indice do registro removido
 *
 *  INSERCAO (pop / led_obter_slot_livre):
 *    - se ledCabeca != -1: desempilha a cabeca (reuso) e a cabeca passa a
 *      ser o proximoLed do slot reaproveitado;
 *    - senao: devolve um novo indice no fim do arquivo (append) e incrementa
 *      totalRegistros no cabecalho.
 * ============================================================================
 */

// Empilha (push) um slot na LED: marca o registro como removido, encadeia-o
// na cabeca e atualiza o cabecalho. 'indice' eh a posicao do registro a apagar.
// Retorna true em sucesso. (O modulo CRUD chama isto no delete.)
bool led_empilhar_slot(FILE *arquivo, int indice);

// Obtem um slot livre para uma nova insercao:
//   - se houver lapide reutilizavel, desempilha (pop) a cabeca da LED e
//     devolve esse indice (reuso do mais recente);
//   - senao, devolve um novo indice ao fim do arquivo (append) e ajusta o
//     totalRegistros do cabecalho.
// Em 'reutilizado' devolve true se o slot veio da LED, false se foi append.
// Retorna o indice do slot, ou -1 em erro.
int led_obter_slot_livre(FILE *arquivo, bool &reutilizado);

#endif // LED_HPP
