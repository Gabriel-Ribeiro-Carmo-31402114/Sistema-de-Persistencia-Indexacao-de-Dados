#include "operacoes_crud.hpp"

#include <cstdio>
#include <cstring>
#include "../Gerenciador De Arquivo/gerenciador_de_arquivo.hpp"
#include "../Led/led.hpp"
#include "../Indice Primario/indice_primario.hpp"

/*
 * MODULO Operacoes Crud — implementacao.
 * Cada operacao mutadora mantem a consistencia entre cartas.bin, a LED e as
 * duas listas invertidas. As chamadas ao indice primario (B+) sao stubs.
 */

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

void crud_fechar(CatalogoCartas &catalogo) {
    gda_fechar(catalogo.arquivoDados);
    catalogo.arquivoDados = nullptr;
}

int crud_criar(CatalogoCartas &catalogo, const Carta &carta) {
    // 1. Slot via LED (reuso do mais recente) ou append no fim do arquivo.
    bool reutilizado = false;
    int slot = led_obter_slot_livre(catalogo.arquivoDados, reutilizado);
    if (slot < 0) {
        return -1;
    }

    // 2. Garante que o registro gravado esteja ATIVO e sem ponteiro residual.
    Carta paraGravar = carta;
    paraGravar.flagRemovido = carta_const::FLAG_ATIVO;
    paraGravar.proximoLed   = carta_const::LED_NULO;
    if (!gda_gravar_registro(catalogo.arquivoDados, slot, paraGravar)) {
        return -1;
    }

    // 3. Indexacao secundaria (cor + cmc).
    if (!is_inserir(catalogo.indiceCor, paraGravar, slot)) {
        return -1;
    }
    if (!is_inserir(catalogo.indiceCmc, paraGravar, slot)) {
        return -1;
    }

    // 4. Indice primario (B+) — STUB: chamada feita, sem efeito por enquanto.
    {
        FILE *ip = ip_abrir_ou_criar("indice_primario.bin");
        ip_inserir(ip, paraGravar.id, slot); // retorna false (nao implementado)
        ip_fechar(ip);
    }

    return slot;
}

bool crud_ler_por_posicao(CatalogoCartas &catalogo, int indice, Carta &carta) {
    if (!gda_ler_registro(catalogo.arquivoDados, indice, carta)) {
        return false;
    }
    // Registro lapide conta como "inexistente" para o leitor.
    return carta.flagRemovido != carta_const::FLAG_REMOVIDO;
}

bool crud_atualizar(CatalogoCartas &catalogo, int indice, const Carta &novaCarta) {
    Carta antiga;
    if (!gda_ler_registro(catalogo.arquivoDados, indice, antiga)) {
        return false;
    }
    if (antiga.flagRemovido == carta_const::FLAG_REMOVIDO) {
        return false; // nao atualiza registro apagado
    }

    Carta paraGravar = novaCarta;
    paraGravar.flagRemovido = carta_const::FLAG_ATIVO;
    paraGravar.proximoLed   = carta_const::LED_NULO;

    if (!gda_gravar_registro(catalogo.arquivoDados, indice, paraGravar)) {
        return false;
    }

    // Manutencao das listas invertidas quando a chave secundaria muda.
    // COR: compara a string antiga com a nova.
    if (std::strncmp(antiga.cor, paraGravar.cor, sizeof(antiga.cor)) != 0) {
        is_remover(catalogo.indiceCor, antiga, indice);
        is_inserir(catalogo.indiceCor, paraGravar, indice);
    }
    // CMC: compara o inteiro antigo com o novo.
    if (antiga.custoEmMana != paraGravar.custoEmMana) {
        is_remover(catalogo.indiceCmc, antiga, indice);
        is_inserir(catalogo.indiceCmc, paraGravar, indice);
    }

    return true;
}

bool crud_remover(CatalogoCartas &catalogo, int indice) {
    Carta carta;
    if (!gda_ler_registro(catalogo.arquivoDados, indice, carta)) {
        return false;
    }
    if (carta.flagRemovido == carta_const::FLAG_REMOVIDO) {
        return true; // ja removido (idempotente)
    }

    // 1. Remove das listas invertidas ANTES de o registro virar lapide
    //    (precisamos da cor/cmc originais, ainda intactos em 'carta').
    is_remover(catalogo.indiceCor, carta, indice);
    is_remover(catalogo.indiceCmc, carta, indice);

    // 2. Indice primario (B+) — STUB.
    {
        FILE *ip = ip_abrir_ou_criar("indice_primario.bin");
        ip_remover(ip, carta.id); // nao implementado
        ip_fechar(ip);
    }

    // 3. Lapide + push na LED (marca flagRemovido, encadeia, atualiza cabecalho).
    return led_empilhar_slot(catalogo.arquivoDados, indice);
}

int crud_buscar_por_cor(CatalogoCartas &catalogo, const char *cor,
                        int *resultados, int capacidade) {
    return is_buscar_por_chave(catalogo.indiceCor, cor, resultados, capacidade);
}

int crud_buscar_por_cmc(CatalogoCartas &catalogo, int custoEmMana,
                        int *resultados, int capacidade) {
    char chave[indice_sec_const::TAM_CHAVE];
    std::snprintf(chave, sizeof(chave), "%d", custoEmMana);
    return is_buscar_por_chave(catalogo.indiceCmc, chave, resultados, capacidade);
}
