#ifndef INDICE_PRIMARIO_HPP
#define INDICE_PRIMARIO_HPP

#include <cstdio>
#include <vector>

namespace indice_primario_const {
    constexpr int TAM_PAGINA = 4096;
    constexpr int PAGINA_NULA = -1;
    // Ordem 100 é um número excelente e seguro para caber nos 4096 bytes folgadamente.
    constexpr int ORDEM = 100; 
}

#pragma pack(push, 1)
// Estrutura que fica na Página 0 do arquivo de índice
struct CabecalhoArvore {
    int raizPageId;
    int totalPaginas;
    int ordem;
    int altura;
    // Preenchimento para garantir que o cabeçalho ocupe 1 página inteira (4096 bytes)
    char padding[indice_primario_const::TAM_PAGINA - (4 * sizeof(int))]; 
};

// Estrutura de um Nó Genérico (Folha ou Interno) que morará nas Páginas 1, 2, 3...
struct NoArvoreBPlus {
    bool ehFolha;
    int qtdChaves;
    int pageId;
    int chaves[indice_primario_const::ORDEM - 1];
    int ponteiros[indice_primario_const::ORDEM]; // Se interno: pageIds. Se folha: offsets/índices
    int proximaFolha; // Exclusivo para folhas (encadeamento B+)
    
    // Padding calculado para cravar a página em 4096 bytes
    char padding[indice_primario_const::TAM_PAGINA - (
        sizeof(bool) + (2 * sizeof(int)) + 
        (sizeof(int) * (indice_primario_const::ORDEM - 1)) + 
        (sizeof(int) * indice_primario_const::ORDEM) + 
        sizeof(int)
    )];
};
#pragma pack(pop)

// Garante que as estruturas do índice primário ocupam exatamente 1 página física de 4096 bytes.
static_assert(sizeof(CabecalhoArvore) == 4096, "CabecalhoArvore deve ter exatamente 4096 bytes");
static_assert(sizeof(NoArvoreBPlus) == 4096, "NoArvoreBPlus deve ter exatamente 4096 bytes");

// Funções base (API)
FILE *ip_abrir_ou_criar(const char *caminho);
void ip_fechar(FILE *arquivo);
bool ip_inserir(FILE *arquivo, int id, int indiceRegistro);
bool ip_buscar(FILE *arquivo, int id, int &indiceRegistro);
bool ip_remover(FILE *arquivo, int id);

#endif