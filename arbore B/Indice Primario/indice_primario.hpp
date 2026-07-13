#ifndef INDICE_PRIMARIO_HPP
#define INDICE_PRIMARIO_HPP

#include <cstdio>

// Modulo de Indice Primario (Arvore B+ em disco) - Atualmente com stubs para futura implementacao.

namespace indice_primario_const {
    constexpr int TAM_PAGINA = 4096;
    constexpr int PAGINA_NULA = -1;
}

// Abre ou cria o arquivo de indice primario.
FILE *ip_abrir_ou_criar(const char *caminho);

// Fecha o arquivo do indice primario.
void ip_fechar(FILE *arquivo);

// Insere a chave primaria e a referencia do registro na Arvore B+.
bool ip_inserir(FILE *arquivo, int id, int indiceRegistro);

// Busca por um ID na Arvore B+ e retorna a referencia encontrada.
bool ip_buscar(FILE *arquivo, int id, int &indiceRegistro);

// Remove uma chave primaria da Arvore B+.
bool ip_remover(FILE *arquivo, int id);

#endif // INDICE_PRIMARIO_HPP
