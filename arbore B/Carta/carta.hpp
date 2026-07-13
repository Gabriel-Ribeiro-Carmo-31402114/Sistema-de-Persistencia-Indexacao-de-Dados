#ifndef CARTA_HPP
#define CARTA_HPP

#include <cstdint>

// Estrutura de dados para representar uma carta de Magic com tamanho fixo de 200 bytes.
// O #pragma pack garante que nao havera padding, mantendo o alinhamento correto em disco.

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
