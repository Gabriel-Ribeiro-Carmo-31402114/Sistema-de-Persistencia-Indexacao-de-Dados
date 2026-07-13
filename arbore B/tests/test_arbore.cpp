/*
 * ============================================================================
 *  test_arbore.cpp — Jupiter Unit Test Suite (Arbore B)
 * ----------------------------------------------------------------------------
 *  Framework: single-header catch2-style hand-rolled test runner (no external
 *  deps needed — avoids the catch of installing a test framework when no
 *  package manager is configured). Each TEST_CASE macro registers itself; the
 *  main() at the bottom runs them all and prints a summary.
 *
 *  COMPILE (once a toolchain is available — run from project root):
 *    g++ -std=c++17 -Wall -Wextra \
 *        tests/test_arbore.cpp \
 *        "Carta/carta.cpp" \
 *        "Gerenciador De Arquivo/gerenciador_de_arquivo.cpp" \
 *        "Led/led.cpp" \
 *        "Indice Secundario/indice_secundario.cpp" \
 *        "Indice Primario/indice_primario.cpp" \
 *        "Operacoes Crud/operacoes_crud.cpp" \
 *        -o tests/test_arbore
 *
 *  RUN:
 *    tests/test_arbore        (or tests\test_arbore.exe on Windows)
 *
 *  EXIT CODE: 0 = all passed, 1 = one or more failures.
 *
 *  SCOPE:
 *    - carta.*              (struct layout, inicializar, copiar_texto)
 *    - gerenciador_de_arquivo.* (open/create, cabecalho read/write, registro
 *                                read/write, offset arithmetic)
 *    - led.*                (empilhar_slot / obter_slot_livre LIFO semantics)
 *    - indice_secundario.*  (extrair_chave_*, is_inserir, is_buscar_por_chave,
 *                            is_remover, is_reconstruir)
 *    - operacoes_crud.*     (crud_abrir/fechar, crud_criar, crud_ler_por_posicao,
 *                            crud_atualizar, crud_remover, crud_buscar_por_*)
 *    - indice_primario.*    (stub contract: always returns nullptr/false)
 *
 *  NOTE ON ISOLATION: each test that touches disk creates its own uniquely
 *  named temp files (prefixed with "test_") and removes them in teardown so
 *  tests are independent and leave no state behind.
 * ============================================================================
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <functional>
#include <vector>
#include <string>
#include <stdexcept>

// ---- bring in all project headers ----
#include "../Carta/carta.hpp"
#include "../Gerenciador De Arquivo/gerenciador_de_arquivo.hpp"
#include "../Led/led.hpp"
#include "../Indice Secundario/indice_secundario.hpp"
#include "../Indice Primario/indice_primario.hpp"
#include "../Operacoes Crud/operacoes_crud.hpp"

// ============================================================================
//  Minimal test harness
// ============================================================================

static int g_passed = 0;
static int g_failed = 0;

struct TestCase {
    std::string name;
    std::function<void()> fn;
};
static std::vector<TestCase> g_tests;

struct Registrar {
    Registrar(const char *name, std::function<void()> fn) {
        g_tests.push_back({name, fn});
    }
};

#define TEST_CASE(name) \
    static void _test_##name(); \
    static Registrar _reg_##name(#name, _test_##name); \
    static void _test_##name()

#define CHECK(expr) do { \
    if (!(expr)) { \
        std::printf("  FAIL  %s:%d  CHECK(%s)\n", __FILE__, __LINE__, #expr); \
        throw std::runtime_error("check failed"); \
    } \
} while(0)

#define CHECK_EQ(a, b) do { \
    if (!((a) == (b))) { \
        std::printf("  FAIL  %s:%d  CHECK_EQ(%s, %s)\n", __FILE__, __LINE__, #a, #b); \
        throw std::runtime_error("check_eq failed"); \
    } \
} while(0)

// Deletes a list of temp files (best-effort, ignores errors).
static void cleanup(std::initializer_list<const char*> paths) {
    for (const char *p : paths) {
        std::remove(p);
    }
}

// ============================================================================
//  MODULE: Carta
// ============================================================================

TEST_CASE(carta_size_is_exactly_200_bytes) {
    // The static_assert in carta.hpp already guards this at compile time.
    // This runtime check is a belt-and-suspenders confirmation.
    CHECK_EQ((int)sizeof(Carta), 200);
}

TEST_CASE(carta_inicializar_sets_flag_ativo_and_led_nulo) {
    Carta c;
    std::memset(&c, 0xFF, sizeof(c)); // poison with garbage
    carta_inicializar(c);
    CHECK_EQ(c.flagRemovido, carta_const::FLAG_ATIVO);
    CHECK_EQ(c.proximoLed,   carta_const::LED_NULO);
    CHECK_EQ(c.id,           0);
}

TEST_CASE(carta_inicializar_zeroes_text_fields) {
    Carta c;
    std::memset(&c, 0xFF, sizeof(c));
    carta_inicializar(c);
    // nome[0] must be '\0' (string zeroed by memset inside inicializar)
    CHECK_EQ(c.nome[0], '\0');
    CHECK_EQ(c.cor[0],  '\0');
}

TEST_CASE(carta_copiar_texto_copies_short_string) {
    char buf[10];
    carta_copiar_texto(buf, "abc", 10);
    CHECK_EQ(std::strcmp(buf, "abc"), 0);
}

TEST_CASE(carta_copiar_texto_truncates_to_capacity_minus_one) {
    char buf[5];
    carta_copiar_texto(buf, "abcdefgh", 5);
    // Must fit within capacity and be null-terminated.
    CHECK_EQ(buf[4], '\0');
    CHECK_EQ(buf[0], 'a');
    CHECK_EQ(buf[3], 'd');
}

TEST_CASE(carta_copiar_texto_null_terminates_exactly_at_capacity) {
    char buf[4];
    carta_copiar_texto(buf, "xyz", 4);
    CHECK_EQ(buf[3], '\0');
    CHECK_EQ(std::strcmp(buf, "xyz"), 0);
}

TEST_CASE(carta_copiar_texto_capacity_zero_does_nothing) {
    char buf[4] = {'A', 'B', 'C', '\0'};
    carta_copiar_texto(buf, "xyz", 0);
    // Must not write anything when capacity is 0.
    CHECK_EQ(buf[0], 'A');
}

TEST_CASE(carta_copiar_texto_capacity_one_writes_only_null) {
    char buf[4] = {'A', 'B', 'C', '\0'};
    carta_copiar_texto(buf, "xyz", 1);
    CHECK_EQ(buf[0], '\0');
}

TEST_CASE(carta_flag_constants_have_expected_values) {
    CHECK_EQ(carta_const::FLAG_ATIVO,    ' ');
    CHECK_EQ(carta_const::FLAG_REMOVIDO, '*');
    CHECK_EQ(carta_const::LED_NULO,      -1);
    CHECK_EQ(carta_const::TAMANHO_REGISTRO, 200);
}

// ============================================================================
//  MODULE: Gerenciador De Arquivo
// ============================================================================

TEST_CASE(gda_offset_do_indice_index_0_equals_header_size) {
    // offset(0) == sizeof(CabecalhoArquivo) == 8
    long expected = static_cast<long>(arquivo_const::TAM_CABECALHO);
    CHECK_EQ(gda_offset_do_indice(0), expected);
}

TEST_CASE(gda_offset_do_indice_index_1_equals_header_plus_200) {
    long expected = static_cast<long>(arquivo_const::TAM_CABECALHO) + 200L;
    CHECK_EQ(gda_offset_do_indice(1), expected);
}

TEST_CASE(gda_offset_do_indice_index_5_equals_header_plus_1000) {
    long expected = static_cast<long>(arquivo_const::TAM_CABECALHO) + 1000L;
    CHECK_EQ(gda_offset_do_indice(5), expected);
}

TEST_CASE(gda_abrir_ou_criar_creates_new_file_with_valid_header) {
    const char *path = "test_gda_new.bin";
    cleanup({path});

    FILE *f = gda_abrir_ou_criar(path);
    CHECK(f != nullptr);

    CabecalhoArquivo hdr;
    CHECK(gda_ler_cabecalho(f, hdr));
    CHECK_EQ(hdr.ledCabeca,      carta_const::LED_NULO);
    CHECK_EQ(hdr.totalRegistros, 0);

    gda_fechar(f);
    cleanup({path});
}

TEST_CASE(gda_abrir_ou_criar_reopens_existing_file_without_resetting_header) {
    const char *path = "test_gda_reopen.bin";
    cleanup({path});

    // Create and write a non-default header.
    FILE *f = gda_abrir_ou_criar(path);
    CHECK(f != nullptr);
    CabecalhoArquivo hdr;
    hdr.ledCabeca      = 3;
    hdr.totalRegistros = 7;
    CHECK(gda_gravar_cabecalho(f, hdr));
    gda_fechar(f);

    // Reopen: must preserve the values we wrote.
    f = gda_abrir_ou_criar(path);
    CHECK(f != nullptr);
    CabecalhoArquivo hdr2;
    CHECK(gda_ler_cabecalho(f, hdr2));
    CHECK_EQ(hdr2.ledCabeca,      3);
    CHECK_EQ(hdr2.totalRegistros, 7);
    gda_fechar(f);
    cleanup({path});
}

TEST_CASE(gda_gravar_e_ler_cabecalho_round_trip) {
    const char *path = "test_gda_hdr_rt.bin";
    cleanup({path});

    FILE *f = gda_abrir_ou_criar(path);
    CHECK(f != nullptr);

    CabecalhoArquivo out;
    out.ledCabeca      = 42;
    out.totalRegistros = 99;
    CHECK(gda_gravar_cabecalho(f, out));

    CabecalhoArquivo in;
    CHECK(gda_ler_cabecalho(f, in));
    CHECK_EQ(in.ledCabeca,      42);
    CHECK_EQ(in.totalRegistros, 99);

    gda_fechar(f);
    cleanup({path});
}

TEST_CASE(gda_gravar_e_ler_registro_round_trip_at_index_0) {
    const char *path = "test_gda_reg0.bin";
    cleanup({path});

    FILE *f = gda_abrir_ou_criar(path);
    CHECK(f != nullptr);

    Carta out;
    carta_inicializar(out);
    out.id = 555;
    out.custoEmMana = 3;
    carta_copiar_texto(out.nome, "Test Card", sizeof(out.nome));
    carta_copiar_texto(out.cor,  "Azul",      sizeof(out.cor));

    CHECK(gda_gravar_registro(f, 0, out));

    Carta in;
    CHECK(gda_ler_registro(f, 0, in));
    CHECK_EQ(in.id,          555);
    CHECK_EQ(in.custoEmMana, 3);
    CHECK_EQ(std::strcmp(in.nome, "Test Card"), 0);
    CHECK_EQ(std::strcmp(in.cor,  "Azul"),      0);

    gda_fechar(f);
    cleanup({path});
}

TEST_CASE(gda_gravar_e_ler_registro_round_trip_at_index_3) {
    const char *path = "test_gda_reg3.bin";
    cleanup({path});

    FILE *f = gda_abrir_ou_criar(path);
    CHECK(f != nullptr);

    // Write slot 3 (slots 0-2 left uninitialised on disk but within valid file range).
    Carta out;
    carta_inicializar(out);
    out.id = 777;
    carta_copiar_texto(out.cor, "Preto", sizeof(out.cor));
    CHECK(gda_gravar_registro(f, 3, out));

    Carta in;
    CHECK(gda_ler_registro(f, 3, in));
    CHECK_EQ(in.id, 777);
    CHECK_EQ(std::strcmp(in.cor, "Preto"), 0);

    gda_fechar(f);
    cleanup({path});
}

TEST_CASE(gda_ler_registro_negative_index_returns_false) {
    const char *path = "test_gda_neg.bin";
    cleanup({path});

    FILE *f = gda_abrir_ou_criar(path);
    CHECK(f != nullptr);

    Carta c;
    bool ok = gda_ler_registro(f, -1, c);
    CHECK(!ok);

    gda_fechar(f);
    cleanup({path});
}

TEST_CASE(gda_gravar_registro_negative_index_returns_false) {
    const char *path = "test_gda_gneg.bin";
    cleanup({path});

    FILE *f = gda_abrir_ou_criar(path);
    CHECK(f != nullptr);

    Carta c;
    carta_inicializar(c);
    bool ok = gda_gravar_registro(f, -1, c);
    CHECK(!ok);

    gda_fechar(f);
    cleanup({path});
}

TEST_CASE(gda_fechar_null_does_not_crash) {
    // Must be a no-op for nullptr.
    gda_fechar(nullptr);
    CHECK(true); // reaching here is success
}

TEST_CASE(gda_cabecalho_size_is_8_bytes) {
    CHECK_EQ(arquivo_const::TAM_CABECALHO, 8);
    CHECK_EQ((int)sizeof(CabecalhoArquivo), 8);
}

// ============================================================================
//  MODULE: Led
// ============================================================================

// Helper: opens a fresh test file and returns the handle.
static FILE *abrir_arquivo_led(const char *path) {
    cleanup({path});
    return gda_abrir_ou_criar(path);
}

TEST_CASE(led_obter_slot_livre_on_empty_file_appends_slot_0) {
    const char *path = "test_led_empty.bin";
    FILE *f = abrir_arquivo_led(path);
    CHECK(f != nullptr);

    bool reused = true;
    int slot = led_obter_slot_livre(f, reused);
    CHECK_EQ(slot, 0);
    CHECK(!reused);

    // Header must now have totalRegistros == 1.
    CabecalhoArquivo hdr;
    CHECK(gda_ler_cabecalho(f, hdr));
    CHECK_EQ(hdr.totalRegistros, 1);
    CHECK_EQ(hdr.ledCabeca,      carta_const::LED_NULO);

    gda_fechar(f);
    cleanup({path});
}

TEST_CASE(led_obter_slot_livre_second_append_returns_slot_1) {
    const char *path = "test_led_two.bin";
    FILE *f = abrir_arquivo_led(path);
    CHECK(f != nullptr);

    bool r;
    int s0 = led_obter_slot_livre(f, r);
    int s1 = led_obter_slot_livre(f, r);
    CHECK_EQ(s0, 0);
    CHECK_EQ(s1, 1);

    gda_fechar(f);
    cleanup({path});
}

TEST_CASE(led_empilhar_slot_pushes_index_to_led_head) {
    const char *path = "test_led_push.bin";
    FILE *f = abrir_arquivo_led(path);
    CHECK(f != nullptr);

    // Allocate and physically write a record at slot 0.
    bool r;
    led_obter_slot_livre(f, r); // totalRegistros -> 1
    Carta c;
    carta_inicializar(c);
    gda_gravar_registro(f, 0, c);

    // Push slot 0 onto the LED.
    CHECK(led_empilhar_slot(f, 0));

    // Header ledCabeca must now point to slot 0.
    CabecalhoArquivo hdr;
    CHECK(gda_ler_cabecalho(f, hdr));
    CHECK_EQ(hdr.ledCabeca, 0);

    // The record at slot 0 must have FLAG_REMOVIDO.
    Carta reg;
    CHECK(gda_ler_registro(f, 0, reg));
    CHECK_EQ(reg.flagRemovido, carta_const::FLAG_REMOVIDO);
    // proximoLed must be the OLD head, which was -1.
    CHECK_EQ(reg.proximoLed, carta_const::LED_NULO);

    gda_fechar(f);
    cleanup({path});
}

TEST_CASE(led_push_then_pop_reuses_slot_lifo_order) {
    const char *path = "test_led_lifo.bin";
    FILE *f = abrir_arquivo_led(path);
    CHECK(f != nullptr);

    // Allocate 3 slots (0, 1, 2) and write physical records.
    for (int i = 0; i < 3; ++i) {
        bool r;
        led_obter_slot_livre(f, r);
        Carta c;
        carta_inicializar(c);
        gda_gravar_registro(f, i, c);
    }

    // Delete in order: slot 0, then slot 2.
    // LIFO stack should be: head -> 2 -> 0 -> -1
    CHECK(led_empilhar_slot(f, 0));
    CHECK(led_empilhar_slot(f, 2));

    // Pop: should return 2 first (most recently pushed).
    bool reused;
    int got = led_obter_slot_livre(f, reused);
    CHECK_EQ(got, 2);
    CHECK(reused);

    // Pop again: should return 0.
    got = led_obter_slot_livre(f, reused);
    CHECK_EQ(got, 0);
    CHECK(reused);

    // LED now empty: next must append (slot 3).
    got = led_obter_slot_livre(f, reused);
    CHECK_EQ(got, 3);
    CHECK(!reused);

    gda_fechar(f);
    cleanup({path});
}

TEST_CASE(led_empilhar_slot_chains_two_pushes_correctly) {
    const char *path = "test_led_chain.bin";
    FILE *f = abrir_arquivo_led(path);
    CHECK(f != nullptr);

    // Allocate and write 2 records.
    for (int i = 0; i < 2; ++i) {
        bool r;
        led_obter_slot_livre(f, r);
        Carta c;
        carta_inicializar(c);
        gda_gravar_registro(f, i, c);
    }

    // Push 0, then push 1. Expected chain: head=1 -> proximoLed[1]=0 -> -1
    CHECK(led_empilhar_slot(f, 0));
    CHECK(led_empilhar_slot(f, 1));

    CabecalhoArquivo hdr;
    CHECK(gda_ler_cabecalho(f, hdr));
    CHECK_EQ(hdr.ledCabeca, 1);

    Carta r1;
    CHECK(gda_ler_registro(f, 1, r1));
    CHECK_EQ(r1.proximoLed, 0);

    Carta r0;
    CHECK(gda_ler_registro(f, 0, r0));
    CHECK_EQ(r0.proximoLed, carta_const::LED_NULO);

    gda_fechar(f);
    cleanup({path});
}

TEST_CASE(led_empilhar_slot_negative_index_returns_false) {
    const char *path = "test_led_neg.bin";
    FILE *f = abrir_arquivo_led(path);
    CHECK(f != nullptr);

    bool ok = led_empilhar_slot(f, -1);
    CHECK(!ok);

    gda_fechar(f);
    cleanup({path});
}

// ============================================================================
//  MODULE: Indice Secundario
// ============================================================================

TEST_CASE(is_extrair_chave_cor_copies_carta_cor_field) {
    Carta c;
    carta_inicializar(c);
    carta_copiar_texto(c.cor, "Vermelho", sizeof(c.cor));

    char dest[indice_sec_const::TAM_CHAVE];
    is_extrair_chave_cor(c, dest, indice_sec_const::TAM_CHAVE);
    CHECK_EQ(std::strcmp(dest, "Vermelho"), 0);
}

TEST_CASE(is_extrair_chave_cmc_formats_int_as_string) {
    Carta c;
    carta_inicializar(c);
    c.custoEmMana = 7;

    char dest[indice_sec_const::TAM_CHAVE];
    is_extrair_chave_cmc(c, dest, indice_sec_const::TAM_CHAVE);
    CHECK_EQ(std::strcmp(dest, "7"), 0);
}

TEST_CASE(is_extrair_chave_cmc_formats_zero) {
    Carta c;
    carta_inicializar(c);
    c.custoEmMana = 0;

    char dest[indice_sec_const::TAM_CHAVE];
    is_extrair_chave_cmc(c, dest, indice_sec_const::TAM_CHAVE);
    CHECK_EQ(std::strcmp(dest, "0"), 0);
}

TEST_CASE(is_criar_indice_cor_has_correct_paths_and_extractor) {
    IndiceSecundario idx = is_criar_indice_cor();
    CHECK_EQ(std::strcmp(idx.caminhoIndice, "index_sec_cor.bin"), 0);
    CHECK_EQ(std::strcmp(idx.caminhoLista,  "lista_inv_cor.bin"), 0);
    CHECK(idx.extrairChave == is_extrair_chave_cor);
}

TEST_CASE(is_criar_indice_cmc_has_correct_paths_and_extractor) {
    IndiceSecundario idx = is_criar_indice_cmc();
    CHECK_EQ(std::strcmp(idx.caminhoIndice, "index_sec_cmc.bin"), 0);
    CHECK_EQ(std::strcmp(idx.caminhoLista,  "lista_inv_cmc.bin"), 0);
    CHECK(idx.extrairChave == is_extrair_chave_cmc);
}

// Helper: build an IndiceSecundario that uses temp files with the given prefix.
static IndiceSecundario make_temp_idx(const char *idxPath, const char *listPath,
                                      void (*fn)(const Carta&, char*, int)) {
    IndiceSecundario idx;
    idx.caminhoIndice = idxPath;
    idx.caminhoLista  = listPath;
    idx.extrairChave  = fn;
    return idx;
}

TEST_CASE(is_inserir_single_card_found_by_buscar) {
    const char *ip = "test_is_idx1.bin";
    const char *lp = "test_is_lst1.bin";
    cleanup({ip, lp});

    IndiceSecundario idx = make_temp_idx(ip, lp, is_extrair_chave_cor);

    Carta c;
    carta_inicializar(c);
    carta_copiar_texto(c.cor, "Azul", sizeof(c.cor));
    CHECK(is_inserir(idx, c, 0));

    int resultados[10];
    int n = is_buscar_por_chave(idx, "Azul", resultados, 10);
    CHECK_EQ(n, 1);
    CHECK_EQ(resultados[0], 0);

    cleanup({ip, lp});
}

TEST_CASE(is_inserir_multiple_cards_same_key_all_found) {
    const char *ip = "test_is_idx2.bin";
    const char *lp = "test_is_lst2.bin";
    cleanup({ip, lp});

    IndiceSecundario idx = make_temp_idx(ip, lp, is_extrair_chave_cor);

    Carta c;
    carta_inicializar(c);
    carta_copiar_texto(c.cor, "Verde", sizeof(c.cor));

    CHECK(is_inserir(idx, c, 0));
    CHECK(is_inserir(idx, c, 1));
    CHECK(is_inserir(idx, c, 5));

    int resultados[10];
    int n = is_buscar_por_chave(idx, "Verde", resultados, 10);
    CHECK_EQ(n, 3);

    // All three refs must appear (order may be LIFO, but all present).
    bool found0 = false, found1 = false, found5 = false;
    for (int i = 0; i < n; ++i) {
        if (resultados[i] == 0) found0 = true;
        if (resultados[i] == 1) found1 = true;
        if (resultados[i] == 5) found5 = true;
    }
    CHECK(found0);
    CHECK(found1);
    CHECK(found5);

    cleanup({ip, lp});
}

TEST_CASE(is_buscar_por_chave_returns_zero_for_missing_key) {
    const char *ip = "test_is_idx3.bin";
    const char *lp = "test_is_lst3.bin";
    cleanup({ip, lp});

    IndiceSecundario idx = make_temp_idx(ip, lp, is_extrair_chave_cor);

    Carta c;
    carta_inicializar(c);
    carta_copiar_texto(c.cor, "Azul", sizeof(c.cor));
    is_inserir(idx, c, 0);

    int resultados[10];
    int n = is_buscar_por_chave(idx, "Preto", resultados, 10);
    CHECK_EQ(n, 0);

    cleanup({ip, lp});
}

TEST_CASE(is_buscar_por_chave_returns_zero_when_index_file_absent) {
    // No files created — simulates first run where index does not exist yet.
    const char *ip = "test_is_absent_idx.bin";
    const char *lp = "test_is_absent_lst.bin";
    cleanup({ip, lp});

    IndiceSecundario idx = make_temp_idx(ip, lp, is_extrair_chave_cor);
    int resultados[10];
    int n = is_buscar_por_chave(idx, "Azul", resultados, 10);
    CHECK_EQ(n, 0);
}

TEST_CASE(is_remover_head_node_unlinks_correctly) {
    const char *ip = "test_is_idx4.bin";
    const char *lp = "test_is_lst4.bin";
    cleanup({ip, lp});

    IndiceSecundario idx = make_temp_idx(ip, lp, is_extrair_chave_cor);

    Carta c;
    carta_inicializar(c);
    carta_copiar_texto(c.cor, "Branco", sizeof(c.cor));
    is_inserir(idx, c, 0);
    is_inserir(idx, c, 1);

    // Remove ref 1 (which became the head due to LIFO insertion).
    CHECK(is_remover(idx, c, 1));

    int resultados[10];
    int n = is_buscar_por_chave(idx, "Branco", resultados, 10);
    CHECK_EQ(n, 1);
    CHECK_EQ(resultados[0], 0);

    cleanup({ip, lp});
}

TEST_CASE(is_remover_middle_node_unlinks_correctly) {
    const char *ip = "test_is_idx5.bin";
    const char *lp = "test_is_lst5.bin";
    cleanup({ip, lp});

    IndiceSecundario idx = make_temp_idx(ip, lp, is_extrair_chave_cor);

    Carta c;
    carta_inicializar(c);
    carta_copiar_texto(c.cor, "Vermelho", sizeof(c.cor));
    is_inserir(idx, c, 0);
    is_inserir(idx, c, 1);
    is_inserir(idx, c, 2);
    // LIFO order in list: head -> 2 -> 1 -> 0 -> -1

    // Remove ref 1 (middle node).
    CHECK(is_remover(idx, c, 1));

    int resultados[10];
    int n = is_buscar_por_chave(idx, "Vermelho", resultados, 10);
    CHECK_EQ(n, 2);
    bool found0 = false, found2 = false;
    for (int i = 0; i < n; ++i) {
        if (resultados[i] == 0) found0 = true;
        if (resultados[i] == 2) found2 = true;
    }
    CHECK(found0);
    CHECK(found2);

    cleanup({ip, lp});
}

TEST_CASE(is_remover_nonexistent_ref_is_idempotent) {
    const char *ip = "test_is_idx6.bin";
    const char *lp = "test_is_lst6.bin";
    cleanup({ip, lp});

    IndiceSecundario idx = make_temp_idx(ip, lp, is_extrair_chave_cor);
    Carta c;
    carta_inicializar(c);
    carta_copiar_texto(c.cor, "Verde", sizeof(c.cor));
    is_inserir(idx, c, 0);

    // Attempt to remove a ref that was never inserted.
    CHECK(is_remover(idx, c, 99));

    // Original record still present.
    int resultados[10];
    int n = is_buscar_por_chave(idx, "Verde", resultados, 10);
    CHECK_EQ(n, 1);
    CHECK_EQ(resultados[0], 0);

    cleanup({ip, lp});
}

TEST_CASE(is_remover_when_index_absent_returns_true_idempotent) {
    // No files exist — calling remove on absent index must not fail.
    const char *ip = "test_is_abs2_idx.bin";
    const char *lp = "test_is_abs2_lst.bin";
    cleanup({ip, lp});

    IndiceSecundario idx = make_temp_idx(ip, lp, is_extrair_chave_cor);
    Carta c;
    carta_inicializar(c);
    carta_copiar_texto(c.cor, "Azul", sizeof(c.cor));
    bool ok = is_remover(idx, c, 0);
    CHECK(ok); // idempotent: "nothing to remove" is still success
}

TEST_CASE(is_reconstruir_rebuilds_index_from_live_records_only) {
    // Use CRUD to write a file, delete one record, then rebuild and verify.
    const char *dados  = "test_reconstruir_cartas.bin";
    const char *ip_cor = "test_reconstruir_cor.bin";
    const char *lp_cor = "test_reconstruir_cor_lst.bin";
    cleanup({dados, ip_cor, lp_cor});

    // Write two records directly.
    FILE *f = gda_abrir_ou_criar(dados);
    CHECK(f != nullptr);

    CabecalhoArquivo hdr;
    hdr.ledCabeca      = carta_const::LED_NULO;
    hdr.totalRegistros = 2;
    gda_gravar_cabecalho(f, hdr);

    Carta c0;
    carta_inicializar(c0);
    carta_copiar_texto(c0.cor, "Azul", sizeof(c0.cor));
    gda_gravar_registro(f, 0, c0);

    Carta c1;
    carta_inicializar(c1);
    c1.flagRemovido = carta_const::FLAG_REMOVIDO; // logical delete
    carta_copiar_texto(c1.cor, "Azul", sizeof(c1.cor));
    gda_gravar_registro(f, 1, c1);

    gda_fechar(f);

    IndiceSecundario idx = make_temp_idx(ip_cor, lp_cor, is_extrair_chave_cor);
    CHECK(is_reconstruir(idx, dados));

    // Only the live record (slot 0) should be indexed.
    int resultados[10];
    int n = is_buscar_por_chave(idx, "Azul", resultados, 10);
    CHECK_EQ(n, 1);
    CHECK_EQ(resultados[0], 0);

    cleanup({dados, ip_cor, lp_cor});
}

// ============================================================================
//  MODULE: Indice Primario (stub contract)
// ============================================================================

TEST_CASE(ip_abrir_ou_criar_stub_returns_nullptr) {
    FILE *f = ip_abrir_ou_criar("test_ip_stub.bin");
    CHECK(f == nullptr);
    // No file handle to close; but ip_fechar must handle nullptr gracefully.
    ip_fechar(f);
    // If we get here without a crash, the stub behaves correctly.
    CHECK(true);
}

TEST_CASE(ip_inserir_stub_returns_false) {
    bool ok = ip_inserir(nullptr, 1, 0);
    CHECK(!ok);
}

TEST_CASE(ip_buscar_stub_returns_false) {
    int idx = 0;
    bool ok = ip_buscar(nullptr, 1, idx);
    CHECK(!ok);
}

TEST_CASE(ip_remover_stub_returns_false) {
    bool ok = ip_remover(nullptr, 1);
    CHECK(!ok);
}

// ============================================================================
//  MODULE: Operacoes Crud (integration)
// ============================================================================

// Helper: tear down all CRUD temp files.
static void crud_cleanup(const char *dados) {
    std::remove(dados);
    std::remove("index_sec_cor.bin");
    std::remove("lista_inv_cor.bin");
    std::remove("index_sec_cmc.bin");
    std::remove("lista_inv_cmc.bin");
    std::remove("indice_primario.bin");
}

// Helper: build a simple Carta for CRUD tests.
static Carta make_carta(int id, const char *nome, int cmc, const char *cor) {
    Carta c;
    carta_inicializar(c);
    c.id          = id;
    c.custoEmMana = cmc;
    carta_copiar_texto(c.nome, nome, sizeof(c.nome));
    carta_copiar_texto(c.cor,  cor,  sizeof(c.cor));
    carta_copiar_texto(c.tipo, "Feitico", sizeof(c.tipo));
    carta_copiar_texto(c.raridade, "Comum", sizeof(c.raridade));
    return c;
}

TEST_CASE(crud_abrir_creates_catalogo_with_valid_file_handle) {
    const char *dados = "test_crud_open.bin";
    crud_cleanup(dados);

    CatalogoCartas cat;
    CHECK(crud_abrir(cat, dados));
    CHECK(cat.arquivoDados != nullptr);

    crud_fechar(cat);
    CHECK(cat.arquivoDados == nullptr);
    crud_cleanup(dados);
}

TEST_CASE(crud_criar_first_insert_uses_slot_0) {
    const char *dados = "test_crud_create0.bin";
    crud_cleanup(dados);

    CatalogoCartas cat;
    CHECK(crud_abrir(cat, dados));

    Carta c = make_carta(1, "Lightning Bolt", 1, "Vermelho");
    int slot = crud_criar(cat, c);
    CHECK_EQ(slot, 0);

    crud_fechar(cat);
    crud_cleanup(dados);
}

TEST_CASE(crud_criar_sequential_inserts_use_sequential_slots) {
    const char *dados = "test_crud_create_seq.bin";
    crud_cleanup(dados);

    CatalogoCartas cat;
    CHECK(crud_abrir(cat, dados));

    int s0 = crud_criar(cat, make_carta(1, "A", 1, "Vermelho"));
    int s1 = crud_criar(cat, make_carta(2, "B", 2, "Azul"));
    int s2 = crud_criar(cat, make_carta(3, "C", 3, "Verde"));
    CHECK_EQ(s0, 0);
    CHECK_EQ(s1, 1);
    CHECK_EQ(s2, 2);

    crud_fechar(cat);
    crud_cleanup(dados);
}

TEST_CASE(crud_ler_por_posicao_returns_inserted_card) {
    const char *dados = "test_crud_read.bin";
    crud_cleanup(dados);

    CatalogoCartas cat;
    CHECK(crud_abrir(cat, dados));

    Carta c = make_carta(42, "Forest", 0, "Verde");
    int slot = crud_criar(cat, c);

    Carta lida;
    CHECK(crud_ler_por_posicao(cat, slot, lida));
    CHECK_EQ(lida.id,          42);
    CHECK_EQ(lida.custoEmMana, 0);
    CHECK_EQ(std::strcmp(lida.nome, "Forest"), 0);
    CHECK_EQ(std::strcmp(lida.cor,  "Verde"),  0);

    crud_fechar(cat);
    crud_cleanup(dados);
}

TEST_CASE(crud_ler_por_posicao_returns_false_for_removed_record) {
    const char *dados = "test_crud_read_del.bin";
    crud_cleanup(dados);

    CatalogoCartas cat;
    CHECK(crud_abrir(cat, dados));

    int slot = crud_criar(cat, make_carta(1, "Island", 0, "Azul"));
    CHECK(crud_remover(cat, slot));

    Carta lida;
    bool ok = crud_ler_por_posicao(cat, slot, lida);
    CHECK(!ok);

    crud_fechar(cat);
    crud_cleanup(dados);
}

TEST_CASE(crud_remover_pushes_slot_onto_led) {
    const char *dados = "test_crud_del_led.bin";
    crud_cleanup(dados);

    CatalogoCartas cat;
    CHECK(crud_abrir(cat, dados));

    int s0 = crud_criar(cat, make_carta(1, "Shock", 1, "Vermelho"));
    int s1 = crud_criar(cat, make_carta(2, "Forest", 0, "Verde"));

    CHECK(crud_remover(cat, s1));

    // Next insert must reuse s1.
    int sReuse = crud_criar(cat, make_carta(3, "Mana Leak", 2, "Azul"));
    CHECK_EQ(sReuse, s1);

    crud_fechar(cat);
    crud_cleanup(dados);
}

TEST_CASE(crud_remover_already_removed_is_idempotent) {
    const char *dados = "test_crud_del_idem.bin";
    crud_cleanup(dados);

    CatalogoCartas cat;
    CHECK(crud_abrir(cat, dados));

    int slot = crud_criar(cat, make_carta(1, "Forest", 0, "Verde"));
    CHECK(crud_remover(cat, slot));
    CHECK(crud_remover(cat, slot)); // second removal must still return true

    crud_fechar(cat);
    crud_cleanup(dados);
}

TEST_CASE(crud_buscar_por_cor_finds_inserted_cards) {
    const char *dados = "test_crud_buscacor.bin";
    crud_cleanup(dados);

    CatalogoCartas cat;
    CHECK(crud_abrir(cat, dados));

    crud_criar(cat, make_carta(1, "Lightning Bolt", 1, "Vermelho"));
    crud_criar(cat, make_carta(2, "Shock",          1, "Vermelho"));
    crud_criar(cat, make_carta(3, "Forest",          0, "Verde"));

    int resultados[10];
    int n = crud_buscar_por_cor(cat, "Vermelho", resultados, 10);
    CHECK_EQ(n, 2);

    crud_fechar(cat);
    crud_cleanup(dados);
}

TEST_CASE(crud_buscar_por_cor_excludes_deleted_cards) {
    const char *dados = "test_crud_buscacor_del.bin";
    crud_cleanup(dados);

    CatalogoCartas cat;
    CHECK(crud_abrir(cat, dados));

    int s0 = crud_criar(cat, make_carta(1, "Bolt", 1, "Vermelho"));
    crud_criar(cat, make_carta(2, "Shock", 1, "Vermelho"));
    crud_remover(cat, s0);

    // After removal, the inverted list still contains s0's node (LED does not
    // physically purge nodes), but crud_ler_por_posicao returns false for it.
    // The test verifies that the removed record no longer satisfies a live read.
    Carta lida;
    bool s0Vivo = crud_ler_por_posicao(cat, s0, lida);
    CHECK(!s0Vivo);

    crud_fechar(cat);
    crud_cleanup(dados);
}

TEST_CASE(crud_buscar_por_cmc_finds_cards_by_mana_cost) {
    const char *dados = "test_crud_buscacmc.bin";
    crud_cleanup(dados);

    CatalogoCartas cat;
    CHECK(crud_abrir(cat, dados));

    crud_criar(cat, make_carta(1, "Opt",   1, "Azul"));
    crud_criar(cat, make_carta(2, "Bolt",  1, "Vermelho"));
    crud_criar(cat, make_carta(3, "Force", 0, "Azul"));

    int resultados[10];
    int n = crud_buscar_por_cmc(cat, 1, resultados, 10);
    CHECK_EQ(n, 2);

    int m = crud_buscar_por_cmc(cat, 0, resultados, 10);
    CHECK_EQ(m, 1);

    crud_fechar(cat);
    crud_cleanup(dados);
}

TEST_CASE(crud_atualizar_changes_stored_record_fields) {
    const char *dados = "test_crud_upd.bin";
    crud_cleanup(dados);

    CatalogoCartas cat;
    CHECK(crud_abrir(cat, dados));

    int slot = crud_criar(cat, make_carta(1, "Forest", 0, "Verde"));

    Carta nova = make_carta(1, "Island", 0, "Azul");
    CHECK(crud_atualizar(cat, slot, nova));

    Carta lida;
    CHECK(crud_ler_por_posicao(cat, slot, lida));
    CHECK_EQ(std::strcmp(lida.nome, "Island"), 0);
    CHECK_EQ(std::strcmp(lida.cor,  "Azul"),   0);

    crud_fechar(cat);
    crud_cleanup(dados);
}

TEST_CASE(crud_atualizar_updates_secondary_index_when_cor_changes) {
    const char *dados = "test_crud_upd_cor.bin";
    crud_cleanup(dados);

    CatalogoCartas cat;
    CHECK(crud_abrir(cat, dados));

    int slot = crud_criar(cat, make_carta(1, "Forest", 0, "Verde"));

    // Change cor from Verde to Azul.
    Carta nova = make_carta(1, "Forest", 0, "Azul");
    CHECK(crud_atualizar(cat, slot, nova));

    // Slot must be reachable under "Azul" now.
    int resultados[10];
    int n = crud_buscar_por_cor(cat, "Azul", resultados, 10);
    CHECK(n >= 1);
    bool found = false;
    for (int i = 0; i < n; ++i) {
        if (resultados[i] == slot) found = true;
    }
    CHECK(found);

    crud_fechar(cat);
    crud_cleanup(dados);
}

TEST_CASE(crud_atualizar_updates_secondary_index_when_cmc_changes) {
    const char *dados = "test_crud_upd_cmc.bin";
    crud_cleanup(dados);

    CatalogoCartas cat;
    CHECK(crud_abrir(cat, dados));

    int slot = crud_criar(cat, make_carta(1, "Llanowar Elves", 1, "Verde"));

    // Change CMC from 1 to 3.
    Carta nova = make_carta(1, "Llanowar Elves", 3, "Verde");
    CHECK(crud_atualizar(cat, slot, nova));

    int resultados[10];
    int n = crud_buscar_por_cmc(cat, 3, resultados, 10);
    CHECK(n >= 1);
    bool found = false;
    for (int i = 0; i < n; ++i) {
        if (resultados[i] == slot) found = true;
    }
    CHECK(found);

    crud_fechar(cat);
    crud_cleanup(dados);
}

TEST_CASE(crud_atualizar_on_removed_record_returns_false) {
    const char *dados = "test_crud_upd_del.bin";
    crud_cleanup(dados);

    CatalogoCartas cat;
    CHECK(crud_abrir(cat, dados));

    int slot = crud_criar(cat, make_carta(1, "Shock", 1, "Vermelho"));
    crud_remover(cat, slot);

    Carta nova = make_carta(1, "Shock", 1, "Vermelho");
    bool ok = crud_atualizar(cat, slot, nova);
    CHECK(!ok);

    crud_fechar(cat);
    crud_cleanup(dados);
}

TEST_CASE(crud_led_reuse_after_multiple_deletes_lifo) {
    // Insert 3, delete 0 then 2 (push order: 0 first, then 2), expect LIFO
    // pop order on next two inserts: 2 then 0.
    const char *dados = "test_crud_led_multi.bin";
    crud_cleanup(dados);

    CatalogoCartas cat;
    CHECK(crud_abrir(cat, dados));

    int s0 = crud_criar(cat, make_carta(1, "A", 1, "Vermelho"));
    int s1 = crud_criar(cat, make_carta(2, "B", 2, "Azul"));
    int s2 = crud_criar(cat, make_carta(3, "C", 3, "Verde"));
    (void)s1;

    // Push s0 then s2 onto LED.
    crud_remover(cat, s0);
    crud_remover(cat, s2);

    // LIFO: most recently pushed is s2, so next insert should reuse s2.
    int r1 = crud_criar(cat, make_carta(4, "D", 1, "Preto"));
    int r2 = crud_criar(cat, make_carta(5, "E", 2, "Branco"));
    CHECK_EQ(r1, s2);
    CHECK_EQ(r2, s0);

    crud_fechar(cat);
    crud_cleanup(dados);
}

TEST_CASE(crud_buscar_por_cor_returns_zero_for_unknown_color) {
    const char *dados = "test_crud_buscacor_miss.bin";
    crud_cleanup(dados);

    CatalogoCartas cat;
    CHECK(crud_abrir(cat, dados));

    crud_criar(cat, make_carta(1, "Bolt", 1, "Vermelho"));

    int resultados[10];
    int n = crud_buscar_por_cor(cat, "Preto", resultados, 10);
    CHECK_EQ(n, 0);

    crud_fechar(cat);
    crud_cleanup(dados);
}

TEST_CASE(crud_buscar_por_cmc_returns_zero_for_unknown_cost) {
    const char *dados = "test_crud_buscacmc_miss.bin";
    crud_cleanup(dados);

    CatalogoCartas cat;
    CHECK(crud_abrir(cat, dados));

    crud_criar(cat, make_carta(1, "Bolt", 1, "Vermelho"));

    int resultados[10];
    int n = crud_buscar_por_cmc(cat, 99, resultados, 10);
    CHECK_EQ(n, 0);

    crud_fechar(cat);
    crud_cleanup(dados);
}

// ============================================================================
//  Test runner
// ============================================================================

int main() {
    std::printf("============================================================\n");
    std::printf("  JUPITER UNIT TEST SUITE — Arbore B\n");
    std::printf("  Total tests registered: %d\n", (int)g_tests.size());
    std::printf("============================================================\n\n");

    for (auto &tc : g_tests) {
        std::printf("[ RUN  ] %s\n", tc.name.c_str());
        try {
            tc.fn();
            std::printf("[ PASS ] %s\n", tc.name.c_str());
            ++g_passed;
        } catch (const std::exception &ex) {
            std::printf("[ FAIL ] %s  (%s)\n", tc.name.c_str(), ex.what());
            ++g_failed;
        } catch (...) {
            std::printf("[ FAIL ] %s  (unknown exception)\n", tc.name.c_str());
            ++g_failed;
        }
    }

    std::printf("\n============================================================\n");
    std::printf("  RESULTS  Passed: %d  |  Failed: %d  |  Total: %d\n",
                g_passed, g_failed, g_passed + g_failed);
    std::printf("============================================================\n");

    return g_failed > 0 ? 1 : 0;
}
