#include "indice_primario.hpp"

// Abre ou cria o arquivo de indice primario. (Stub - Nao implementado)
FILE *ip_abrir_ou_criar(const char *caminho) {
    (void)caminho;
    return nullptr;
}

// Fecha o arquivo do indice primario. (Stub - Nao implementado)
void ip_fechar(FILE *arquivo) {
    (void)arquivo;
}

// Insere a chave primaria na Arvore B+. (Stub - Nao implementado)
bool ip_inserir(FILE *arquivo, int id, int indiceRegistro) {
    (void)arquivo; (void)id; (void)indiceRegistro;
    return false;
}

// Busca por uma chave primaria na Arvore B+. (Stub - Nao implementado)
bool ip_buscar(FILE *arquivo, int id, int &indiceRegistro) {
    (void)arquivo; (void)id; (void)indiceRegistro;
    return false;
}

// Remove uma chave primaria da Arvore B+. (Stub - Nao implementado)
bool ip_remover(FILE *arquivo, int id) {
    (void)arquivo; (void)id;
    return false;
}
