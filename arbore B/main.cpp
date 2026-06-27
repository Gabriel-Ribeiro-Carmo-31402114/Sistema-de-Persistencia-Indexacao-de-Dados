/*
 * ============================================================================
 *  main.cpp — driver/demo do Sistema de Persistencia e Indexacao (catalogo MTG)
 * ----------------------------------------------------------------------------
 *  COMPILACAO (linha unica, a partir da raiz do projeto):
 *
 *    g++ -std=c++17 -Wall -Wextra -o arbore \
 *        "main.cpp" \
 *        "Carta/carta.cpp" \
 *        "Gerenciador De Arquivo/gerenciador_de_arquivo.cpp" \
 *        "Led/led.cpp" \
 *        "Indice Secundario/indice_secundario.cpp" \
 *        "Indice Primario/indice_primario.cpp" \
 *        "Operacoes Crud/operacoes_crud.cpp"
 *
 *  ou simplesmente:  make
 *
 *  O demo: insere algumas cartas, remove uma, mostra o REUSO de slot pela LED
 *  e faz buscas por chave secundaria (cor e cmc) via lista invertida.
 * ============================================================================
 */

#include <cstdio>
#include <cstring>

#include "Carta/carta.hpp"
#include "Gerenciador De Arquivo/gerenciador_de_arquivo.hpp"
#include "Operacoes Crud/operacoes_crud.hpp"

// Helper local do demo: monta uma Carta a partir de campos soltos.
static Carta montar_carta(int id, int numColecao, const char *nome,
                          int cmc, const char *cor, const char *tipo,
                          const char *raridade) {
    Carta carta;
    carta_inicializar(carta);
    carta.id              = id;
    carta.numeroDeColecao = numColecao;
    carta.custoEmMana     = cmc;
    carta_copiar_texto(carta.nome, nome, sizeof(carta.nome));
    carta_copiar_texto(carta.cor, cor, sizeof(carta.cor));
    carta_copiar_texto(carta.tipo, tipo, sizeof(carta.tipo));
    carta_copiar_texto(carta.raridade, raridade, sizeof(carta.raridade));
    carta_copiar_texto(carta.linkImagem, "https://scryfall.com/", sizeof(carta.linkImagem));
    return carta;
}

static void mostrar_busca_cor(CatalogoCartas &catalogo, const char *cor) {
    int posicoes[64];
    int n = crud_buscar_por_cor(catalogo, cor, posicoes, 64);
    std::printf("Busca por cor=\"%s\": %d resultado(s)\n", cor, n);
    for (int i = 0; i < n; ++i) {
        Carta c;
        if (crud_ler_por_posicao(catalogo, posicoes[i], c)) {
            std::printf("   slot %d ->", posicoes[i]);
            carta_imprimir(c);
        }
    }
}

static void mostrar_busca_cmc(CatalogoCartas &catalogo, int cmc) {
    int posicoes[64];
    int n = crud_buscar_por_cmc(catalogo, cmc, posicoes, 64);
    std::printf("Busca por cmc=%d: %d resultado(s)\n", cmc, n);
    for (int i = 0; i < n; ++i) {
        Carta c;
        if (crud_ler_por_posicao(catalogo, posicoes[i], c)) {
            std::printf("   slot %d ->", posicoes[i]);
            carta_imprimir(c);
        }
    }
}

int main() {
    std::printf("=== Catalogo MTG — demo CRUD + LED + Indice Secundario ===\n");
    std::printf("sizeof(Carta) = %zu bytes (deve ser 200)\n\n", sizeof(Carta));

    const char *caminhoDados = "cartas.bin";

    // Comeca de um estado limpo para o demo ser deterministico.
    std::remove(caminhoDados);
    std::remove("index_sec_cor.bin");
    std::remove("lista_inv_cor.bin");
    std::remove("index_sec_cmc.bin");
    std::remove("lista_inv_cmc.bin");

    CatalogoCartas catalogo;
    if (!crud_abrir(catalogo, caminhoDados)) {
        std::fprintf(stderr, "Falha ao abrir o catalogo.\n");
        return 1;
    }

    // --- CREATE: insere 4 cartas (slots 0..3) ---
    std::printf("[CREATE] inserindo 4 cartas...\n");
    int s0 = crud_criar(catalogo, montar_carta(101, 1, "Lightning Bolt", 1, "Vermelho", "Feitico",  "Comum"));
    int s1 = crud_criar(catalogo, montar_carta(102, 2, "Forest",         0, "Verde",    "Terreno",  "Comum"));
    int s2 = crud_criar(catalogo, montar_carta(103, 3, "Shock",          1, "Vermelho", "Feitico",  "Comum"));
    int s3 = crud_criar(catalogo, montar_carta(104, 4, "Llanowar Elves", 1, "Verde",    "Criatura", "Comum"));
    std::printf("   slots usados: %d %d %d %d\n\n", s0, s1, s2, s3);

    // --- buscas por chave secundaria ---
    mostrar_busca_cor(catalogo, "Vermelho");
    mostrar_busca_cmc(catalogo, 1);
    std::printf("\n");

    // --- DELETE: remove a carta no slot 1 (Forest) -> vai para a LED ---
    std::printf("[DELETE] removendo slot %d (Forest)...\n", s1);
    crud_remover(catalogo, s1);

    // Mostra que a busca por cor=Verde agora retorna so a Llanowar Elves.
    mostrar_busca_cor(catalogo, "Verde");
    std::printf("\n");

    // --- CREATE de novo: deve REUSAR o slot 1 liberado pela LED ---
    std::printf("[CREATE] inserindo nova carta — espera-se reuso do slot %d...\n", s1);
    int sReuso = crud_criar(catalogo, montar_carta(105, 5, "Giant Growth", 1, "Verde", "Feitico", "Comum"));
    std::printf("   slot retornado pela LED: %d  (esperado: %d -> %s)\n\n",
                sReuso, s1, (sReuso == s1 ? "REUSO OK" : "append"));

    // --- UPDATE: muda a cor de uma carta e confirma manutencao do indice ---
    std::printf("[UPDATE] mudando cor da carta no slot %d de Vermelho->Azul...\n", s0);
    Carta atualizada = montar_carta(101, 1, "Lightning Bolt", 1, "Azul", "Feitico", "Comum");
    crud_atualizar(catalogo, s0, atualizada);
    mostrar_busca_cor(catalogo, "Vermelho");
    mostrar_busca_cor(catalogo, "Azul");
    std::printf("\n");

    // --- REBUILD: reconstroi os indices a partir de cartas.bin ---
    std::printf("[REBUILD] reconstruindo indices secundarios a partir de cartas.bin...\n");
    bool okCor = is_reconstruir(catalogo.indiceCor, caminhoDados);
    bool okCmc = is_reconstruir(catalogo.indiceCmc, caminhoDados);
    std::printf("   rebuild cor=%s  cmc=%s\n", okCor ? "ok" : "falha", okCmc ? "ok" : "falha");
    mostrar_busca_cmc(catalogo, 1);

    crud_fechar(catalogo);
    std::printf("\n=== fim do demo ===\n");
    return 0;
}
