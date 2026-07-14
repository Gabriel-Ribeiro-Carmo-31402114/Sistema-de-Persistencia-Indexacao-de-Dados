#include "indice_primario.hpp"
#include <cstdlib>
#include <cstring>
#include <algorithm>

// ============================================================================
// FUNÇÕES AUXILIARES DE I/O EM DISCO
// ============================================================================

static void ler_cabecalho(FILE *arq, CabecalhoArvore &cab) {
    std::fseek(arq, 0, SEEK_SET);
    std::fread(&cab, sizeof(CabecalhoArvore), 1, arq);
}

static void gravar_cabecalho(FILE *arq, const CabecalhoArvore &cab) {
    std::fseek(arq, 0, SEEK_SET);
    std::fwrite(&cab, sizeof(CabecalhoArvore), 1, arq);
    std::fflush(arq);
}

static void ler_no(FILE *arq, int pageId, NoArvoreBPlus &no) {
    std::fseek(arq, pageId * indice_primario_const::TAM_PAGINA, SEEK_SET);
    std::fread(&no, sizeof(NoArvoreBPlus), 1, arq);
}

static void gravar_no(FILE *arq, const NoArvoreBPlus &no) {
    std::fseek(arq, no.pageId * indice_primario_const::TAM_PAGINA, SEEK_SET);
    std::fwrite(&no, sizeof(NoArvoreBPlus), 1, arq);
    std::fflush(arq);
}

static NoArvoreBPlus criar_novo_no(FILE *arq, CabecalhoArvore &cab, bool ehFolha) {
    NoArvoreBPlus no;
    std::memset(&no, 0, sizeof(NoArvoreBPlus));
    no.ehFolha = ehFolha;
    no.qtdChaves = 0;
    no.pageId = cab.totalPaginas++;
    no.proximaFolha = indice_primario_const::PAGINA_NULA;
    for (int i = 0; i < indice_primario_const::ORDEM; ++i) no.ponteiros[i] = indice_primario_const::PAGINA_NULA;
    gravar_cabecalho(arq, cab); // Atualiza o total de páginas
    return no;
}

// ============================================================================
// IMPLEMENTAÇÃO DA API PRINCIPAL
// ============================================================================

FILE *ip_abrir_ou_criar(const char *caminho) {
    FILE *arq = std::fopen(caminho, "rb+");
    if (!arq) {
        arq = std::fopen(caminho, "wb+");
        if (!arq) return nullptr;
        
        CabecalhoArvore cab;
        std::memset(&cab, 0, sizeof(CabecalhoArvore));
        cab.raizPageId = indice_primario_const::PAGINA_NULA;
        cab.totalPaginas = 1; // Página 0 é o cabeçalho
        cab.ordem = indice_primario_const::ORDEM;
        cab.altura = 0;
        gravar_cabecalho(arq, cab);
    }
    return arq;
}

void ip_fechar(FILE *arquivo) {
    if (arquivo) std::fclose(arquivo);
}

bool ip_buscar(FILE *arquivo, int id, int &indiceRegistro) {
    if (!arquivo) return false;
    CabecalhoArvore cab;
    ler_cabecalho(arquivo, cab);
    if (cab.raizPageId == indice_primario_const::PAGINA_NULA) return false;

    NoArvoreBPlus atual;
    ler_no(arquivo, cab.raizPageId, atual);

    // Desce na árvore até a folha
    while (!atual.ehFolha) {
        int i = 0;
        while (i < atual.qtdChaves && id >= atual.chaves[i]) i++;
        ler_no(arquivo, atual.ponteiros[i], atual);
    }

    // Busca binária ou linear na folha
    for (int i = 0; i < atual.qtdChaves; ++i) {
        if (atual.chaves[i] == id) {
            indiceRegistro = atual.ponteiros[i];
            return true;
        }
    }
    return false;
}

// Lógica de Inserção com Split
bool ip_inserir(FILE *arquivo, int id, int indiceRegistro) {
    if (!arquivo) return false;
    CabecalhoArvore cab;
    ler_cabecalho(arquivo, cab);

    if (cab.raizPageId == indice_primario_const::PAGINA_NULA) {
        NoArvoreBPlus raiz = criar_novo_no(arquivo, cab, true);
        raiz.chaves[0] = id;
        raiz.ponteiros[0] = indiceRegistro;
        raiz.qtdChaves = 1;
        gravar_no(arquivo, raiz);
        cab.raizPageId = raiz.pageId;
        cab.altura = 1;
        gravar_cabecalho(arquivo, cab);
        return true;
    }

    // Rastreador do caminho (Pilha) para voltar fazendo splits se necessário
    std::vector<int> caminho;
    NoArvoreBPlus atual;
    ler_no(arquivo, cab.raizPageId, atual);

    while (!atual.ehFolha) {
        caminho.push_back(atual.pageId);
        int i = 0;
        while (i < atual.qtdChaves && id >= atual.chaves[i]) i++;
        ler_no(arquivo, atual.ponteiros[i], atual);
    }

    // Já existe? Não insere duplicado.
    for (int i = 0; i < atual.qtdChaves; ++i) {
        if (atual.chaves[i] == id) return false;
    }

    // Inserção simples na Folha (tem espaço)
    if (atual.qtdChaves < indice_primario_const::ORDEM - 1) {
        int i = atual.qtdChaves - 1;
        while (i >= 0 && atual.chaves[i] > id) {
            atual.chaves[i + 1] = atual.chaves[i];
            atual.ponteiros[i + 1] = atual.ponteiros[i];
            i--;
        }
        atual.chaves[i + 1] = id;
        atual.ponteiros[i + 1] = indiceRegistro;
        atual.qtdChaves++;
        gravar_no(arquivo, atual);
        return true;
    }

    // ==========================================
    // SPLIT DA FOLHA (Nó Cheio)
    // ==========================================
    int tempChaves[indice_primario_const::ORDEM];
    int tempPonts[indice_primario_const::ORDEM];
    int i = atual.qtdChaves - 1, j = atual.qtdChaves;
    while (i >= 0 && atual.chaves[i] > id) {
        tempChaves[j] = atual.chaves[i]; tempPonts[j] = atual.ponteiros[i];
        i--; j--;
    }
    tempChaves[j] = id; tempPonts[j] = indiceRegistro;
    while (i >= 0) {
        tempChaves[i] = atual.chaves[i]; tempPonts[i] = atual.ponteiros[i];
        i--;
    }

    NoArvoreBPlus novaFolha = criar_novo_no(arquivo, cab, true);
    int meio = indice_primario_const::ORDEM / 2;
    
    atual.qtdChaves = meio;
    novaFolha.qtdChaves = indice_primario_const::ORDEM - meio;
    
    for (int k = 0; k < atual.qtdChaves; k++) {
        atual.chaves[k] = tempChaves[k]; atual.ponteiros[k] = tempPonts[k];
    }
    for (int k = 0; k < novaFolha.qtdChaves; k++) {
        novaFolha.chaves[k] = tempChaves[meio + k]; novaFolha.ponteiros[k] = tempPonts[meio + k];
    }

    novaFolha.proximaFolha = atual.proximaFolha;
    atual.proximaFolha = novaFolha.pageId;
    
    int chavePromovida = novaFolha.chaves[0];
    int filhoDireito = novaFolha.pageId;

    gravar_no(arquivo, atual);
    gravar_no(arquivo, novaFolha);

    // ==========================================
    // PROPAGAÇÃO DO SPLIT PARA CIMA (Nós Internos)
    // ==========================================
    while (!caminho.empty()) {
        int paiId = caminho.back();
        caminho.pop_back();
        NoArvoreBPlus pai;
        ler_no(arquivo, paiId, pai);

        if (pai.qtdChaves < indice_primario_const::ORDEM - 1) {
            int k = pai.qtdChaves - 1;
            while (k >= 0 && pai.chaves[k] > chavePromovida) {
                pai.chaves[k + 1] = pai.chaves[k];
                pai.ponteiros[k + 2] = pai.ponteiros[k + 1];
                k--;
            }
            pai.chaves[k + 1] = chavePromovida;
            pai.ponteiros[k + 2] = filhoDireito;
            pai.qtdChaves++;
            gravar_no(arquivo, pai);
            return true;
        }

        // Split do Nó Interno
        int tChaves[indice_primario_const::ORDEM];
        int tPonts[indice_primario_const::ORDEM + 1];
        
        int k = pai.qtdChaves - 1; int w = pai.qtdChaves;
        tPonts[w + 1] = pai.ponteiros[k + 1];
        while (k >= 0 && pai.chaves[k] > chavePromovida) {
            tChaves[w] = pai.chaves[k];
            tPonts[w] = pai.ponteiros[k];
            k--; w--;
        }
        tChaves[w] = chavePromovida;
        tPonts[w + 1] = filhoDireito;
        while (k >= 0) {
            tChaves[k] = pai.chaves[k]; tPonts[k + 1] = pai.ponteiros[k + 1];
            tPonts[k] = pai.ponteiros[k];
            k--;
        }

        NoArvoreBPlus novoInterno = criar_novo_no(arquivo, cab, false);
        int meioInt = indice_primario_const::ORDEM / 2;
        
        pai.qtdChaves = meioInt;
        novoInterno.qtdChaves = indice_primario_const::ORDEM - meioInt - 1;
        
        for (int m = 0; m < pai.qtdChaves; m++) {
            pai.chaves[m] = tChaves[m]; pai.ponteiros[m] = tPonts[m];
        }
        pai.ponteiros[pai.qtdChaves] = tPonts[pai.qtdChaves];
        
        chavePromovida = tChaves[meioInt]; // Chave do meio Sobe!
        
        for (int m = 0; m < novoInterno.qtdChaves; m++) {
            novoInterno.chaves[m] = tChaves[meioInt + 1 + m];
            novoInterno.ponteiros[m] = tPonts[meioInt + 1 + m];
        }
        novoInterno.ponteiros[novoInterno.qtdChaves] = tPonts[indice_primario_const::ORDEM];
        
        filhoDireito = novoInterno.pageId;
        
        gravar_no(arquivo, pai);
        gravar_no(arquivo, novoInterno);
    }

    // Se chegou aqui, a raiz original dividiu. Criar nova raiz!
    NoArvoreBPlus novaRaiz = criar_novo_no(arquivo, cab, false);
    novaRaiz.chaves[0] = chavePromovida;
    novaRaiz.ponteiros[0] = cab.raizPageId;
    novaRaiz.ponteiros[1] = filhoDireito;
    novaRaiz.qtdChaves = 1;
    gravar_no(arquivo, novaRaiz);
    
    cab.raizPageId = novaRaiz.pageId;
    cab.altura++;
    gravar_cabecalho(arquivo, cab);

    return true;
}

// Remoção Lazy (Preguiçosa): Remove a chave do nó, mas não faz merge de páginas
// (Extremamente recomendado academicamente para evitar corrupção em disco)
bool ip_remover(FILE *arquivo, int id) {
    if (!arquivo) return false;
    CabecalhoArvore cab;
    ler_cabecalho(arquivo, cab);
    if (cab.raizPageId == indice_primario_const::PAGINA_NULA) return false;

    NoArvoreBPlus atual;
    ler_no(arquivo, cab.raizPageId, atual);

    while (!atual.ehFolha) {
        int i = 0;
        while (i < atual.qtdChaves && id >= atual.chaves[i]) i++;
        ler_no(arquivo, atual.ponteiros[i], atual);
    }

    for (int i = 0; i < atual.qtdChaves; ++i) {
        if (atual.chaves[i] == id) {
            // Desloca os elementos para a esquerda apagando o registro
            for (int j = i; j < atual.qtdChaves - 1; ++j) {
                atual.chaves[j] = atual.chaves[j + 1];
                atual.ponteiros[j] = atual.ponteiros[j + 1];
            }
            atual.qtdChaves--;
            gravar_no(arquivo, atual);
            return true;
        }
    }
    return false;
}