#include "indice_primario.hpp"

/*
 * MODULO Indice Primario (Arvore B+) — STUB.
 *
 * NENHUMA destas funcoes esta implementada nesta etapa. Os corpos abaixo sao
 * deliberadamente vazios/placeholder, conforme o escopo da entrega. Ver o
 * bloco de planejamento de layout em disco no cabecalho (indice_primario.hpp).
 */

FILE *ip_abrir_ou_criar(const char *caminho) {
    // TODO: abrir/criar indice_primario.bin; se novo, gravar o CABECALHO
    //       (raizPageId = -1, totalPaginas = 1, ordem = ..., altura = 0).
    (void)caminho;
    return nullptr;
}

void ip_fechar(FILE *arquivo) {
    // TODO: fclose(arquivo) quando o arquivo passar a ser realmente aberto.
    (void)arquivo;
}

bool ip_inserir(FILE *arquivo, int id, int indiceRegistro) {
    // TODO: descer ate a folha correta, inserir (id -> indiceRegistro)
    //       ordenado; se a folha estourar, fazer split e promover separador
    //       ao pai (copia na folha), propagando splits ate a raiz.
    (void)arquivo; (void)id; (void)indiceRegistro;
    return false; // nao implementado
}

bool ip_buscar(FILE *arquivo, int id, int &indiceRegistro) {
    // TODO: navegar da raiz ate a folha seguindo os separadores; se achar o
    //       id, devolver true e preencher indiceRegistro com offset/indice.
    (void)arquivo; (void)id; (void)indiceRegistro;
    return false; // nao implementado
}

bool ip_remover(FILE *arquivo, int id) {
    // TODO: localizar a folha, remover a chave e tratar underflow via
    //       redistribuicao/merge com irmaos, ajustando os separadores no pai.
    (void)arquivo; (void)id;
    return false; // nao implementado
}
