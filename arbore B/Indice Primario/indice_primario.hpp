#ifndef INDICE_PRIMARIO_HPP
#define INDICE_PRIMARIO_HPP

#include <cstdio>

/*
 * ============================================================================
 *  MODULO: Indice Primario  (Arvore B+ — id -> offset)  [STUB / NAO IMPLEMENTADO]
 * ----------------------------------------------------------------------------
 *  ESTE MODULO EH APENAS UMA INTERFACE/ESBOCO. Os corpos das funcoes estao
 *  marcados com // TODO e NAO foram implementados nesta etapa (fora do escopo
 *  desta entrega — vale 3.0 pts na comanda principal e +0.5 como B+ em disco).
 *
 *  ----------------------------------------------------------------------------
 *  PLANEJAMENTO DO LAYOUT EM DISCO (indice_primario.bin) — INTENCAO DE PROJETO
 *  ----------------------------------------------------------------------------
 *  Objetivo: indexar a CHAVE PRIMARIA 'id' mapeando id -> offset(/indice) do
 *  registro em cartas.bin, com busca O(log n) e bom desempenho em disco.
 *
 *  Arquivo organizado em PAGINAS (nos) de tamanho fixo TAM_PAGINA (ex.: 4096B),
 *  uma pagina por no, enderecadas por numero de pagina (pageId).
 *
 *    [ CABECALHO ]  pagina 0:
 *        int  raizPageId;     // pagina da raiz; -1 se arvore vazia
 *        int  totalPaginas;   // numero de paginas alocadas
 *        int  ordem;          // ordem (m) da B+ (max. de filhos por no interno)
 *        int  alturaArvore;   // profundidade atual (opcional/diagnostico)
 *
 *    NO INTERNO (apenas roteia; nao guarda offsets de dados):
 *        char ehFolha = 0;
 *        int  numChaves;
 *        int  chaves[ORDEM - 1];        // ids separadores, ordenados
 *        int  filhos[ORDEM];            // pageIds dos filhos
 *
 *    NO FOLHA (guarda os pares id->offset E encadeia folhas — caracteristica
 *    distintiva da B+ frente a B):
 *        char ehFolha = 1;
 *        int  numChaves;
 *        int  chaves[ORDEM - 1];        // ids, ordenados
 *        long offsets[ORDEM - 1];       // offset (ou indice) em cartas.bin por id
 *        int  proximaFolha;             // pageId da folha seguinte; -1 no fim
 *                                       //  -> permite range-scan/varredura ordenada
 *
 *  REGRAS B+ a implementar futuramente:
 *    - Todas as chaves de dados vivem nas FOLHAS; nos internos so roteiam.
 *    - Folhas ligadas em lista (proximaFolha) para varredura sequencial por id.
 *    - Split: ao estourar, divide o no e promove uma chave separadora ao pai
 *      (na folha, a chave separadora eh COPIADA para cima; no interno, MOVIDA).
 *    - Merge/redistribuicao no remove para manter a ocupacao minima (>= teto/2).
 *    - Cada operacao le/grava paginas inteiras via fseek(pageId*TAM_PAGINA).
 *
 *  INTEGRACAO COM O CRUD (futuro):
 *    - create: ip_inserir(id, indiceRegistro) apos gravar a carta.
 *    - read:   ip_buscar(id) -> indice do registro -> gda_ler_registro(...).
 *    - delete: ip_remover(id).
 *  Hoje o CRUD chama estes stubs (que retornam "nao implementado") e cai num
 *  fallback de varredura linear para a busca por id.
 * ============================================================================
 */

namespace indice_primario_const {
    constexpr int TAM_PAGINA = 4096;  // tamanho de pagina/no planejado (futuro)
    constexpr int PAGINA_NULA = -1;   // sentinela de pageId inexistente
}

// Abre/cria o arquivo de indice primario. STUB.
FILE *ip_abrir_ou_criar(const char *caminho);

// Fecha o arquivo de indice primario. STUB.
void ip_fechar(FILE *arquivo);

// Insere o par (id -> indiceRegistro) na B+. STUB — retorna false.
bool ip_inserir(FILE *arquivo, int id, int indiceRegistro);

// Busca um id; em sucesso devolve true e preenche 'indiceRegistro'. STUB.
bool ip_buscar(FILE *arquivo, int id, int &indiceRegistro);

// Remove um id da B+. STUB — retorna false.
bool ip_remover(FILE *arquivo, int id);

#endif // INDICE_PRIMARIO_HPP
