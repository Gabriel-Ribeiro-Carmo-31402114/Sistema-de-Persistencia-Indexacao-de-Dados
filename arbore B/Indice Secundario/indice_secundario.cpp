#include "indice_secundario.hpp"

#include <cstdio>
#include <cstring>

// Copia o valor da cor para o buffer de destino.
void is_extrair_chave_cor(const Carta &carta, char *destino, int capacidade) {
    carta_copiar_texto(destino, carta.cor, capacidade);
}

// Formata o custo em mana como string e copia para o destino.
void is_extrair_chave_cmc(const Carta &carta, char *destino, int capacidade) {
    std::snprintf(destino, static_cast<size_t>(capacidade), "%d", carta.custoEmMana);
}

// Cria os caminhos de arquivos para indexar cor.
IndiceSecundario is_criar_indice_cor() {
    IndiceSecundario indice;
    indice.caminhoIndice = "index_sec_cor.bin";
    indice.caminhoLista  = "lista_inv_cor.bin";
    indice.extrairChave  = is_extrair_chave_cor;
    return indice;
}

// Cria os caminhos de arquivos para indexar CMC.
IndiceSecundario is_criar_indice_cmc() {
    IndiceSecundario indice;
    indice.caminhoIndice = "index_sec_cmc.bin";
    indice.caminhoLista  = "lista_inv_cmc.bin";
    indice.extrairChave  = is_extrair_chave_cmc;
    return indice;
}

namespace {

// Abre ou cria um arquivo binario no modo de leitura/escrita.
FILE *abrir_ou_criar(const char *caminho) {
    FILE *arquivo = std::fopen(caminho, "rb+");
    if (arquivo == nullptr) {
        arquivo = std::fopen(caminho, "wb+");
    }
    return arquivo;
}

// Calcula o total de entradas no arquivo de indice.
long contar_entradas(FILE *arquivoIndice) {
    if (std::fseek(arquivoIndice, 0, SEEK_END) != 0) {
        return -1;
    }
    long tamanho = std::ftell(arquivoIndice);
    if (tamanho < 0) {
        return -1;
    }
    return tamanho / static_cast<long>(sizeof(EntradaIndice));
}

// Busca linearmente por uma chave no arquivo de indice.
long buscar_entrada(FILE *arquivoIndice, const char *chave, EntradaIndice &entrada) {
    long total = contar_entradas(arquivoIndice);
    if (total < 0) {
        return -1;
    }
    for (long i = 0; i < total; ++i) {
        if (std::fseek(arquivoIndice, i * static_cast<long>(sizeof(EntradaIndice)), SEEK_SET) != 0) {
            return -1;
        }
        if (std::fread(&entrada, sizeof(EntradaIndice), 1, arquivoIndice) != 1) {
            return -1;
        }
        if (std::strncmp(entrada.chave, chave, indice_sec_const::TAM_CHAVE) == 0) {
            return i;
        }
    }
    return -1;
}

// Grava uma entrada no indice em uma determinada posicao.
bool gravar_entrada(FILE *arquivoIndice, long pos, const EntradaIndice &entrada) {
    if (std::fseek(arquivoIndice, pos * static_cast<long>(sizeof(EntradaIndice)), SEEK_SET) != 0) {
        return false;
    }
    bool ok = std::fwrite(&entrada, sizeof(EntradaIndice), 1, arquivoIndice) == 1;
    std::fflush(arquivoIndice);
    return ok;
}

// Anexa um novo no no final do arquivo da lista invertida.
long anexar_no(FILE *arquivoLista, const NoListaInvertida &no) {
    if (std::fseek(arquivoLista, 0, SEEK_END) != 0) {
        return -1;
    }
    long tamanho = std::ftell(arquivoLista);
    if (tamanho < 0) {
        return -1;
    }
    long novoIndice = tamanho / static_cast<long>(sizeof(NoListaInvertida));
    if (std::fwrite(&no, sizeof(NoListaInvertida), 1, arquivoLista) != 1) {
        return -1;
    }
    std::fflush(arquivoLista);
    return novoIndice;
}

// Le um no da lista invertida na posicao informada.
bool ler_no(FILE *arquivoLista, long pos, NoListaInvertida &no) {
    if (std::fseek(arquivoLista, pos * static_cast<long>(sizeof(NoListaInvertida)), SEEK_SET) != 0) {
        return false;
    }
    return std::fread(&no, sizeof(NoListaInvertida), 1, arquivoLista) == 1;
}

// Grava as alteracoes em um no da lista invertida na posicao informada.
bool gravar_no(FILE *arquivoLista, long pos, const NoListaInvertida &no) {
    if (std::fseek(arquivoLista, pos * static_cast<long>(sizeof(NoListaInvertida)), SEEK_SET) != 0) {
        return false;
    }
    bool ok = std::fwrite(&no, sizeof(NoListaInvertida), 1, arquivoLista) == 1;
    std::fflush(arquivoLista);
    return ok;
}

} // namespace

// Insere a chave extraida da carta nos arquivos do indice secundario correspondente.
bool is_inserir(const IndiceSecundario &indice, const Carta &carta, int refRegistro) {
    char chave[indice_sec_const::TAM_CHAVE];
    indice.extrairChave(carta, chave, indice_sec_const::TAM_CHAVE);

    FILE *arquivoIndice = abrir_ou_criar(indice.caminhoIndice);
    FILE *arquivoLista  = abrir_ou_criar(indice.caminhoLista);
    if (arquivoIndice == nullptr || arquivoLista == nullptr) {
        if (arquivoIndice) std::fclose(arquivoIndice);
        if (arquivoLista)  std::fclose(arquivoLista);
        return false;
    }

    bool ok = false;
    EntradaIndice entrada;
    long posEntrada = buscar_entrada(arquivoIndice, chave, entrada);

    int cabecaAtual = (posEntrada >= 0) ? entrada.cabecaLista : carta_const::LED_NULO;

    // Novo no aponta para a cabeca atual (insercao LIFO)
    NoListaInvertida no;
    no.refRegistro = refRegistro;
    no.proximo     = cabecaAtual;
    long indiceNovoNo = anexar_no(arquivoLista, no);

    if (indiceNovoNo >= 0) {
        if (posEntrada >= 0) {
            entrada.cabecaLista = static_cast<int>(indiceNovoNo);
            ok = gravar_entrada(arquivoIndice, posEntrada, entrada);
        } else {
            EntradaIndice nova;
            std::memset(&nova, 0, sizeof(EntradaIndice));
            carta_copiar_texto(nova.chave, chave, indice_sec_const::TAM_CHAVE);
            nova.cabecaLista = static_cast<int>(indiceNovoNo);
            long total = contar_entradas(arquivoIndice);
            if (total >= 0) {
                ok = gravar_entrada(arquivoIndice, total, nova);
            }
        }
    }

    std::fclose(arquivoIndice);
    std::fclose(arquivoLista);
    return ok;
}

// Percorre a lista invertida associada a chave e retorna os RRNs encontrados.
int is_buscar_por_chave(const IndiceSecundario &indice, const char *chave,
                        int *resultados, int capacidade) {
    FILE *arquivoIndice = std::fopen(indice.caminhoIndice, "rb");
    if (arquivoIndice == nullptr) {
        return 0;
    }
    FILE *arquivoLista = std::fopen(indice.caminhoLista, "rb");
    if (arquivoLista == nullptr) {
        std::fclose(arquivoIndice);
        return 0;
    }

    int quantidade = 0;
    EntradaIndice entrada;
    long posEntrada = buscar_entrada(arquivoIndice, chave, entrada);
    if (posEntrada >= 0) {
        int atual = entrada.cabecaLista;
        int guarda = 0; // Evita loop infinito caso haja corrupcao
        while (atual != carta_const::LED_NULO &&
               quantidade < capacidade &&
               guarda < indice_sec_const::MAX_RESULTADOS_LOOKUP) {
            NoListaInvertida no;
            if (!ler_no(arquivoLista, atual, no)) {
                break;
            }
            resultados[quantidade++] = no.refRegistro;
            atual = no.proximo;
            ++guarda;
        }
    }

    std::fclose(arquivoIndice);
    std::fclose(arquivoLista);
    return quantidade;
}

// Remove a referencia do registro da lista invertida daquela chave.
bool is_remover(const IndiceSecundario &indice, const Carta &carta, int refRegistro) {
    char chave[indice_sec_const::TAM_CHAVE];
    indice.extrairChave(carta, chave, indice_sec_const::TAM_CHAVE);

    FILE *arquivoIndice = std::fopen(indice.caminhoIndice, "rb+");
    FILE *arquivoLista  = std::fopen(indice.caminhoLista, "rb+");
    if (arquivoIndice == nullptr || arquivoLista == nullptr) {
        if (arquivoIndice) std::fclose(arquivoIndice);
        if (arquivoLista)  std::fclose(arquivoLista);
        return true;
    }

    bool ok = true;
    EntradaIndice entrada;
    long posEntrada = buscar_entrada(arquivoIndice, chave, entrada);
    if (posEntrada >= 0) {
        int atual = entrada.cabecaLista;
        int anterior = carta_const::LED_NULO;
        int guarda = 0;
        while (atual != carta_const::LED_NULO &&
               guarda < indice_sec_const::MAX_RESULTADOS_LOOKUP) {
            NoListaInvertida no;
            if (!ler_no(arquivoLista, atual, no)) {
                ok = false;
                break;
            }
            if (no.refRegistro == refRegistro) {
                // Ajusta os ponteiros para pular o elemento removido
                if (anterior == carta_const::LED_NULO) {
                    entrada.cabecaLista = no.proximo;
                    ok = gravar_entrada(arquivoIndice, posEntrada, entrada);
                } else {
                    NoListaInvertida noAnterior;
                    if (ler_no(arquivoLista, anterior, noAnterior)) {
                        noAnterior.proximo = no.proximo;
                        ok = gravar_no(arquivoLista, anterior, noAnterior);
                    } else {
                        ok = false;
                    }
                }
                break;
            }
            anterior = atual;
            atual = no.proximo;
            ++guarda;
        }
    }

    std::fclose(arquivoIndice);
    std::fclose(arquivoLista);
    return ok;
}

// Trunca os arquivos de indice existentes e reinsere todas as cartas ativas.
bool is_reconstruir(const IndiceSecundario &indice, const char *caminhoCartas) {
    FILE *arqIndice = std::fopen(indice.caminhoIndice, "wb");
    FILE *arqLista  = std::fopen(indice.caminhoLista, "wb");
    if (arqIndice == nullptr || arqLista == nullptr) {
        if (arqIndice) std::fclose(arqIndice);
        if (arqLista)  std::fclose(arqLista);
        return false;
    }
    std::fclose(arqIndice);
    std::fclose(arqLista);

    FILE *arquivoCartas = std::fopen(caminhoCartas, "rb");
    if (arquivoCartas == nullptr) {
        return false;
    }

    CabecalhoArquivo cabecalho;
    bool ok = true;
    if (gda_ler_cabecalho(arquivoCartas, cabecalho)) {
        // Percorre todos os registros do arquivo de dados e reindexa os ativos
        for (int i = 0; i < cabecalho.totalRegistros && ok; ++i) {
            Carta carta;
            if (!gda_ler_registro(arquivoCartas, i, carta)) {
                continue;
            }
            if (carta.flagRemovido != carta_const::FLAG_REMOVIDO) {
                ok = is_inserir(indice, carta, i);
            }
        }
    } else {
        ok = false;
    }

    std::fclose(arquivoCartas);
    return ok;
}
