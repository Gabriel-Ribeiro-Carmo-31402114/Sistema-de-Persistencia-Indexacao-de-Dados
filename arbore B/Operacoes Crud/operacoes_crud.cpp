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

    // Stub do indice primario (B+)
    {
        FILE *ip = ip_abrir_ou_criar("indice_primario.bin");
        ip_inserir(ip, paraGravar.id, slot);
        ip_fechar(ip);
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

// Sobrescreve o registro em disco e atualiza os indices secundarios se houver mudanca de valores.
bool crud_atualizar(CatalogoCartas &catalogo, int indice, const Carta &novaCarta) {
    Carta antiga;
    if (!gda_ler_registro(catalogo.arquivoDados, indice, antiga)) {
        return false;
    }
    if (antiga.flagRemovido == carta_const::FLAG_REMOVIDO) {
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

    // Stub do indice primario (B+)
    {
        FILE *ip = ip_abrir_ou_criar("indice_primario.bin");
        ip_remover(ip, carta.id);
        ip_fechar(ip);
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
