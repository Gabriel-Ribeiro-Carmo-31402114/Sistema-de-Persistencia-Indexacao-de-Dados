#ifndef CARTA_HPP
#define CARTA_HPP

#include <cstdint>

/*
 * ============================================================================
 *  MODULO: Carta
 * ----------------------------------------------------------------------------
 *  Define o registro de dominio "Carta" (uma carta de Magic: The Gathering)
 *  gravado no arquivo de dados principal (cartas.bin).
 *
 *  INVARIANTE CRITICA: sizeof(Carta) == 200 bytes, EXATAMENTE, com a ordem
 *  de campos abaixo preservada.
 *
 *  POR QUE #pragma pack(push, 1):
 *  ------------------------------------------------------------------------
 *  A soma natural dos campos eh:
 *      1 (flagRemovido) + 4 (proximoLed) + 4 (id) + 4 (numeroDeColecao)
 *    + 50 (nome) + 4 (custoEmMana) + 15 (cor) + 30 (tipo) + 12 (raridade)
 *    + 76 (linkImagem) = 200 bytes.
 *
 *  Porem, o alinhamento padrao do compilador (ABI) insere PADDING: o campo
 *  'char flagRemovido' antes do 'int proximoLed' forcaria o int a um offset
 *  multiplo de 4, gerando 3 bytes de preenchimento (1 -> 4), e o struct
 *  passaria de 200 para 203/204 bytes (e ainda seria arredondado para cima).
 *  Isso quebraria a aritmetica de offsets em disco (offset = indice * 200).
 *
 *  #pragma pack(push, 1) forca alinhamento de 1 byte (sem padding) enquanto
 *  o struct eh declarado, e #pragma pack(pop) restaura o alinhamento normal
 *  do compilador depois. Assim o layout em memoria == layout em disco == 200B.
 *
 *  O static_assert no fim trava a invariante EM TEMPO DE COMPILACAO: se algum
 *  dia um campo mudar de tamanho, o build falha em vez de corromper o arquivo.
 * ============================================================================
 */

#pragma pack(push, 1)
struct Carta {
    char flagRemovido;        // gravestone: '*' = removido logicamente, '\0'/' ' = ativo
    int  proximoLed;          // proximo nó na LED (lista de espaços disponiveis); -1 = fim
    int  id;                  // chave primaria unica
    int  numeroDeColecao;     // numero da carta na colecao
    char nome[50];            // nome da carta
    int  custoEmMana;         // CMC: custo em mana — CHAVE SECUNDARIA
    char cor[15];             // cor principal da arte — CHAVE SECUNDARIA
    char tipo[30];            // ex.: Terreno, Criatura, Feitico, Artefato
    char raridade[12];        // raridade da carta
    char linkImagem[76];      // link do scryfall para a imagem
};
#pragma pack(pop)

// Trava a invariante de 200 bytes em tempo de compilacao.
static_assert(sizeof(Carta) == 200,
    "Carta DEVE ter exatamente 200 bytes — verifique o #pragma pack e a ordem dos campos.");

// Constantes de serializacao / dominio.
namespace carta_const {
    constexpr int  TAMANHO_REGISTRO   = 200;   // sizeof(Carta), em bytes
    constexpr char FLAG_ATIVO         = ' ';   // marcador de registro vivo
    constexpr char FLAG_REMOVIDO      = '*';   // gravestone (registro logicamente apagado)
    constexpr int  LED_NULO           = -1;    // sentinela: "sem proximo" / lista vazia
}

// Inicializa um Carta zerado/ativo com os campos textuais terminados em '\0'.
// (definido em carta.cpp)
void carta_inicializar(Carta &carta);

// Copia uma string C para um buffer de tamanho fixo garantindo terminacao nula
// e truncando com seguranca (usado para nome/cor/tipo/raridade/linkImagem).
void carta_copiar_texto(char *destino, const char *origem, int capacidade);

// Imprime um registro de forma legivel (debug / demo).
void carta_imprimir(const Carta &carta);

#endif // CARTA_HPP
