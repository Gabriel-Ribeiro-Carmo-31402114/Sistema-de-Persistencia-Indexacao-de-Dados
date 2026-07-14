#ifndef OPERACOES_CRUD_HPP
#define OPERACOES_CRUD_HPP

#include <cstdio>
#include "../Carta/carta.hpp"
#include "../Indice Secundario/indice_secundario.hpp"

// Orquestra as operacoes CRUD no arquivo de dados e atualiza os indices secundarios.
struct CatalogoCartas {
    FILE *arquivoDados;
    const char *caminhoDados;
    IndiceSecundario indiceCor;
    IndiceSecundario indiceCmc;
};

// Abre ou cria o catalogo de cartas e inicializa as estruturas de dados.
bool crud_abrir(CatalogoCartas &catalogo, const char *caminhoDados);

// Fecha o catalogo e libera os recursos associados.
void crud_fechar(CatalogoCartas &catalogo);

// Insere uma nova carta no arquivo (usando LED para reuso ou fazendo append) e a indexa.
int crud_criar(CatalogoCartas &catalogo, const Carta &carta);

// Le uma carta ativa pelo seu indice de registro (RRN).
bool crud_ler_por_posicao(CatalogoCartas &catalogo, int indice, Carta &carta);

bool crud_buscar_por_id(CatalogoCartas &catalogo, int id, Carta &cartaResult);

// Atualiza os campos de uma carta existente, mantendo a Chave Primaria imutavel.
bool crud_atualizar(CatalogoCartas &catalogo, int indice, const Carta &novaCarta);

// Remove logicamente uma carta (marca lapide) e empilha seu slot na LED.
bool crud_remover(CatalogoCartas &catalogo, int indice);

// Retorna os indices de registros correspondentes a uma determinada cor.
int crud_buscar_por_cor(CatalogoCartas &catalogo, const char *cor,
                        int *resultados, int capacidade);

// Retorna os indices de registros correspondentes a um determinado custo de mana.
int crud_buscar_por_cmc(CatalogoCartas &catalogo, int custoEmMana,
                        int *resultados, int capacidade);

// Executa o desfragmentador (Vacuum): remove os registros marcados como excluídos, zera a LED e reconstrói todos os índices.
bool crud_vacuum(CatalogoCartas &catalogo);

#endif // OPERACOES_CRUD_HPP
