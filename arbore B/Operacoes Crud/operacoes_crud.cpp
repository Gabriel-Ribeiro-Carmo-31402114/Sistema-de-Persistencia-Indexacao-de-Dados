#include "operacoes_crud.hpp"

#include <cstdio>
#include <cstring>
#include "../Gerenciador De Arquivo/gerenciador_de_arquivo.hpp"
#include "../Led/led.hpp"
#include "../Indice Primario/indice_primario.hpp"

// Abre o arquivo de dados principal e inicia os indices secundarios de Cor e CMC.
bool crud_abrir(CatalogoCartas &catalogo, const char *caminhoDados) {
    catalogo.caminhoDados = caminhoDados;
    catalogo.arquivoDados = gda_abrir_ou_criar(caminhoDados);
    if (catalogo.arquivoDados == nullptr) {
        return false;
    }
    catalogo.indiceCor = is_criar_indice_cor();
    catalogo.indiceCmc = is_criar_indice_cmc();
    return true;
}

// Fecha o arquivo de dados.
void crud_fechar(CatalogoCartas &catalogo) {
    gda_fechar(catalogo.arquivoDados);
    catalogo.arquivoDados = nullptr;
}

// Insere uma nova carta obtendo slot livre da LED ou usando o fim do arquivo.
int crud_criar(CatalogoCartas &catalogo, const Carta &carta) {
    // Evita duplicacao de ID
    Carta temp;
    if (crud_buscar_por_id(catalogo, carta.id, temp)) {
        return -1;
    }

    bool reutilizado = false;
    int slot = led_obter_slot_livre(catalogo.arquivoDados, reutilizado);
    if (slot < 0) {
        return -1;
    }

    Carta paraGravar = carta;
    paraGravar.flagRemovido = carta_const::FLAG_ATIVO;
    paraGravar.proximoLed   = carta_const::LED_NULO;
    if (!gda_gravar_registro(catalogo.arquivoDados, slot, paraGravar)) {
        return -1;
    }

    // Insere nos indices secundarios
    if (!is_inserir(catalogo.indiceCor, paraGravar, slot)) {
        return -1;
    }
    if (!is_inserir(catalogo.indiceCmc, paraGravar, slot)) {
        return -1;
    }

    // Registro no indice primario (B+) 
    {
        FILE *ip = ip_abrir_ou_criar("arvore_primaria.bin");
        if (ip) {
            ip_inserir(ip, carta.id, slot); 
            ip_fechar(ip);
        }
    }

    return slot; 
}

// Le o registro no indice indicado e valida se esta ativo.
bool crud_ler_por_posicao(CatalogoCartas &catalogo, int indice, Carta &carta) {
    if (!gda_ler_registro(catalogo.arquivoDados, indice, carta)) {
        return false;
    }
    return carta.flagRemovido != carta_const::FLAG_REMOVIDO;
}


bool crud_buscar_por_id(CatalogoCartas &catalogo, int id, Carta &cartaResult) {
    FILE *ip = ip_abrir_ou_criar("arvore_primaria.bin"); // Ajustado de "indice_primario.bin" para "arvore_primaria.bin"
    int posicaoDisco = -1;
    bool achou = ip_buscar(ip, id, posicaoDisco);
    ip_fechar(ip);

    if (achou) {
        if (gda_ler_registro(catalogo.arquivoDados, posicaoDisco, cartaResult)) {
            return cartaResult.flagRemovido != carta_const::FLAG_REMOVIDO;
        }
    }
    return false;
}


// Sobrescreve o registro em disco e atualiza os indices secundarios se houver mudanca de valores.
bool crud_atualizar(CatalogoCartas &catalogo, int indice, const Carta &novaCarta) {
    Carta antiga;
    if (!gda_ler_registro(catalogo.arquivoDados, indice, antiga)) {
        return false;
    }
    if (antiga.flagRemovido == carta_const::FLAG_REMOVIDO) {
        return false;
    }
    // Garante que a Chave Primaria (ID) é imutavel
    if (antiga.id != novaCarta.id) {
        return false;
    }

    Carta paraGravar = novaCarta;
    paraGravar.flagRemovido = carta_const::FLAG_ATIVO;
    paraGravar.proximoLed   = carta_const::LED_NULO;

    if (!gda_gravar_registro(catalogo.arquivoDados, indice, paraGravar)) {
        return false;
    }

    // Atualiza indice de cor se o valor mudou
    if (std::strncmp(antiga.cor, paraGravar.cor, sizeof(antiga.cor)) != 0) {
        is_remover(catalogo.indiceCor, antiga, indice);
        is_inserir(catalogo.indiceCor, paraGravar, indice);
    }
    // Atualiza indice de CMC se o valor mudou
    if (antiga.custoEmMana != paraGravar.custoEmMana) {
        is_remover(catalogo.indiceCmc, antiga, indice);
        is_inserir(catalogo.indiceCmc, paraGravar, indice);
    }

    return true;
}

// Remove logicamente o registro e o encadeia na LED.
bool crud_remover(CatalogoCartas &catalogo, int indice) {
    Carta carta;
    if (!gda_ler_registro(catalogo.arquivoDados, indice, carta)) {
        return false;
    }
    if (carta.flagRemovido == carta_const::FLAG_REMOVIDO) {
        return true;
    }

    // Remove referências dos índices secundários
    is_remover(catalogo.indiceCor, carta, indice);
    is_remover(catalogo.indiceCmc, carta, indice);

    // Remove do indice primario (B+)
    {
        FILE *ip = ip_abrir_ou_criar("arvore_primaria.bin");
        if (ip) {
            ip_remover(ip, carta.id);
            ip_fechar(ip);
        }
    }

    // Marca lapide e envia o slot para a LED
    return led_empilhar_slot(catalogo.arquivoDados, indice);
}

// Busca correspondencias no indice secundario de cor.
int crud_buscar_por_cor(CatalogoCartas &catalogo, const char *cor,
                        int *resultados, int capacidade) {
    return is_buscar_por_chave(catalogo.indiceCor, cor, resultados, capacidade);
}

// Busca correspondencias no indice secundario de custo de mana.
int crud_buscar_por_cmc(CatalogoCartas &catalogo, int custoEmMana,
                        int *resultados, int capacidade) {
    char chave[indice_sec_const::TAM_CHAVE];
    std::snprintf(chave, sizeof(chave), "%d", custoEmMana);
    return is_buscar_por_chave(catalogo.indiceCmc, chave, resultados, capacidade);
}

bool crud_vacuum(CatalogoCartas &catalogo) {
    if (catalogo.arquivoDados == nullptr) return false;

    // 1. Fecha o arquivo principal atual
    gda_fechar(catalogo.arquivoDados);
    catalogo.arquivoDados = nullptr;

    // 2. Abre o original em modo leitura e o temporário em escrita/leitura
    FILE *origem = std::fopen(catalogo.caminhoDados, "rb");
    if (!origem) return false;

    FILE *destino = std::fopen("cartas_temp.bin", "wb+");
    if (!destino) {
        std::fclose(origem);
        return false;
    }

    // 3. Lê o cabeçalho original
    CabecalhoArquivo cabOrigem;
    if (std::fread(&cabOrigem, sizeof(CabecalhoArquivo), 1, origem) != 1) {
        std::fclose(origem);
        std::fclose(destino);
        return false;
    }

    // 4. Escreve cabeçalho inicial zerado no temporário
    CabecalhoArquivo cabDestino;
    cabDestino.ledCabeca = carta_const::LED_NULO;
    cabDestino.totalRegistros = 0;
    std::fwrite(&cabDestino, sizeof(CabecalhoArquivo), 1, destino);

    // 5. Copia registros ativos sequencialmente
    for (int i = 0; i < cabOrigem.totalRegistros; ++i) {
        std::fseek(origem, sizeof(CabecalhoArquivo) + i * carta_const::TAMANHO_REGISTRO, SEEK_SET);
        Carta carta;
        if (std::fread(&carta, sizeof(Carta), 1, origem) == 1) {
            if (carta.flagRemovido != carta_const::FLAG_REMOVIDO) {
                std::fseek(destino, sizeof(CabecalhoArquivo) + cabDestino.totalRegistros * carta_const::TAMANHO_REGISTRO, SEEK_SET);
                std::fwrite(&carta, sizeof(Carta), 1, destino);
                cabDestino.totalRegistros++;
            }
        }
    }

    // 6. Atualiza o cabeçalho no destino
    std::fseek(destino, 0, SEEK_SET);
    std::fwrite(&cabDestino, sizeof(CabecalhoArquivo), 1, destino);

    std::fclose(origem);
    std::fclose(destino);

    // 7. Substitui o arquivo original pelo temporário desfragmentado
    std::remove(catalogo.caminhoDados);
    if (std::rename("cartas_temp.bin", catalogo.caminhoDados) != 0) {
        return false;
    }

    // 8. Reabre o arquivo de dados desfragmentado
    catalogo.arquivoDados = gda_abrir_ou_criar(catalogo.caminhoDados);
    if (catalogo.arquivoDados == nullptr) return false;

    // 9. Reconstrói o Índice Primário (Árvore B+) do zero, pois os offsets (RRNs) mudaram
    std::remove("arvore_primaria.bin");
    FILE *ip = ip_abrir_ou_criar("arvore_primaria.bin");
    if (!ip) return false;

    for (int i = 0; i < cabDestino.totalRegistros; ++i) {
        Carta carta;
        if (gda_ler_registro(catalogo.arquivoDados, i, carta)) {
            ip_inserir(ip, carta.id, i);
        }
    }
    ip_fechar(ip);

    // 10. Reconstrói os Índices Secundários do zero
    is_reconstruir(catalogo.indiceCor, catalogo.caminhoDados);
    is_reconstruir(catalogo.indiceCmc, catalogo.caminhoDados);

    return true;
}
