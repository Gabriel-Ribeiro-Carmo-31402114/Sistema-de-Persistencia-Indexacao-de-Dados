#include "indice_secundario.hpp"

#include <cstdio>
#include <cstring>

/*
 * MODULO Indice Secundario — implementacao do indice + lista invertida.
 * I/O nativo C apenas. Reaproveitado por 'cor' e 'custoEmMana'.
 *
 * NOTA DE PROJETO: os arquivos de indice/lista nao tem cabecalho proprio.
 *   - No de lista: a posicao (indice) de um no eh derivada do tamanho do
 *     arquivo no momento do append (offset / sizeof(NoListaInvertida)).
 *   - Entrada de indice: idem, busca linear varrendo o arquivo inteiro.
 * Isso mantem o subsistema simples e dentro do escopo pedido.
 */

// ----------------------------------------------------------------------------
// Extratores de chave
// ----------------------------------------------------------------------------

void is_extrair_chave_cor(const Carta &carta, char *destino, int capacidade) {
    carta_copiar_texto(destino, carta.cor, capacidade);
}

void is_extrair_chave_cmc(const Carta &carta, char *destino, int capacidade) {
    std::snprintf(destino, static_cast<size_t>(capacidade), "%d", carta.custoEmMana);
}

IndiceSecundario is_criar_indice_cor() {
    IndiceSecundario indice;
    indice.caminhoIndice = "index_sec_cor.bin";
    indice.caminhoLista  = "lista_inv_cor.bin";
    indice.extrairChave  = is_extrair_chave_cor;
    return indice;
}

IndiceSecundario is_criar_indice_cmc() {
    IndiceSecundario indice;
    indice.caminhoIndice = "index_sec_cmc.bin";
    indice.caminhoLista  = "lista_inv_cmc.bin";
    indice.extrairChave  = is_extrair_chave_cmc;
    return indice;
}

// ----------------------------------------------------------------------------
// Helpers internos de arquivo (anonimos a este modulo)
// ----------------------------------------------------------------------------
namespace {

// Abre (ou cria, se nao existir) um arquivo binario para leitura/escrita.
FILE *abrir_ou_criar(const char *caminho) {
    FILE *arquivo = std::fopen(caminho, "rb+");
    if (arquivo == nullptr) {
        arquivo = std::fopen(caminho, "wb+");
    }
    return arquivo;
}

// Conta quantas EntradaIndice existem no arquivo de indice (ja aberto).
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

// Procura a EntradaIndice com 'chave'. Se achar, devolve seu indice (>=0) e
// preenche 'entrada'; senao devolve -1.
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

// Grava uma EntradaIndice na posicao 'pos' do arquivo de indice.
bool gravar_entrada(FILE *arquivoIndice, long pos, const EntradaIndice &entrada) {
    if (std::fseek(arquivoIndice, pos * static_cast<long>(sizeof(EntradaIndice)), SEEK_SET) != 0) {
        return false;
    }
    bool ok = std::fwrite(&entrada, sizeof(EntradaIndice), 1, arquivoIndice) == 1;
    std::fflush(arquivoIndice);
    return ok;
}

// Anexa um no na lista invertida; devolve o indice do no (>=0) ou -1 em erro.
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

// Le o no na posicao 'pos' da lista invertida.
bool ler_no(FILE *arquivoLista, long pos, NoListaInvertida &no) {
    if (std::fseek(arquivoLista, pos * static_cast<long>(sizeof(NoListaInvertida)), SEEK_SET) != 0) {
        return false;
    }
    return std::fread(&no, sizeof(NoListaInvertida), 1, arquivoLista) == 1;
}

// Grava o no na posicao 'pos' da lista invertida.
bool gravar_no(FILE *arquivoLista, long pos, const NoListaInvertida &no) {
    if (std::fseek(arquivoLista, pos * static_cast<long>(sizeof(NoListaInvertida)), SEEK_SET) != 0) {
        return false;
    }
    bool ok = std::fwrite(&no, sizeof(NoListaInvertida), 1, arquivoLista) == 1;
    std::fflush(arquivoLista);
    return ok;
}

} // namespace anonimo

// ----------------------------------------------------------------------------
// Operacoes publicas
// ----------------------------------------------------------------------------

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

    // A nova insercao vira a nova CABECA da lista invertida da chave (LIFO).
    int cabecaAtual = (posEntrada >= 0) ? entrada.cabecaLista : carta_const::LED_NULO;

    NoListaInvertida no;
    no.refRegistro = refRegistro;
    no.proximo     = cabecaAtual;
    long indiceNovoNo = anexar_no(arquivoLista, no);

    if (indiceNovoNo >= 0) {
        if (posEntrada >= 0) {
            // Atualiza a cabeca da entrada existente.
            entrada.cabecaLista = static_cast<int>(indiceNovoNo);
            ok = gravar_entrada(arquivoIndice, posEntrada, entrada);
        } else {
            // Cria nova entrada de indice ao fim do arquivo.
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

int is_buscar_por_chave(const IndiceSecundario &indice, const char *chave,
                        int *resultados, int capacidade) {
    FILE *arquivoIndice = std::fopen(indice.caminhoIndice, "rb");
    if (arquivoIndice == nullptr) {
        return 0; // sem indice => nenhum resultado
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
        int guarda = 0; // protecao anti-loop
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

bool is_remover(const IndiceSecundario &indice, const Carta &carta, int refRegistro) {
    char chave[indice_sec_const::TAM_CHAVE];
    indice.extrairChave(carta, chave, indice_sec_const::TAM_CHAVE);

    FILE *arquivoIndice = std::fopen(indice.caminhoIndice, "rb+");
    FILE *arquivoLista  = std::fopen(indice.caminhoLista, "rb+");
    if (arquivoIndice == nullptr || arquivoLista == nullptr) {
        if (arquivoIndice) std::fclose(arquivoIndice);
        if (arquivoLista)  std::fclose(arquivoLista);
        return true; // nada para remover (idempotente)
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
                // Desencadeia este no.
                if (anterior == carta_const::LED_NULO) {
                    // Era a cabeca: aponta a entrada para o proximo.
                    entrada.cabecaLista = no.proximo;
                    ok = gravar_entrada(arquivoIndice, posEntrada, entrada);
                } else {
                    // No meio: o no anterior salta este.
                    NoListaInvertida noAnterior;
                    if (ler_no(arquivoLista, anterior, noAnterior)) {
                        noAnterior.proximo = no.proximo;
                        ok = gravar_no(arquivoLista, anterior, noAnterior);
                    } else {
                        ok = false;
                    }
                }
                break; // remove apenas a primeira ocorrencia
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

bool is_reconstruir(const IndiceSecundario &indice, const char *caminhoCartas) {
    // Recria os arquivos do zero ("wb" trunca) e fecha-os logo em seguida; o
    // is_inserir reabre-os em modo append conforme percorre cartas.bin.
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
        return false; // sem dados, indice fica vazio
    }

    CabecalhoArquivo cabecalho;
    bool ok = true;
    if (gda_ler_cabecalho(arquivoCartas, cabecalho)) {
        for (int i = 0; i < cabecalho.totalRegistros && ok; ++i) {
            Carta carta;
            if (!gda_ler_registro(arquivoCartas, i, carta)) {
                continue;
            }
            // Reindexa apenas registros vivos.
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
