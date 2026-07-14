#include "menu.hpp"

#include <cstdio>
#include <cstring>
#include "../Gerenciador De Arquivo/gerenciador_de_arquivo.hpp"
#include "../Indice Primario/indice_primario.hpp"

MenuMTG::MenuMTG(const char *caminhoDados) : caminhoDados(caminhoDados) {
    catalogo.arquivoDados = nullptr;
}

MenuMTG::~MenuMTG() {
    if (catalogo.arquivoDados != nullptr) {
        crud_fechar(catalogo);
    }
}

bool MenuMTG::inicializar() {
    if (!crud_abrir(catalogo, caminhoDados)) {
        std::fprintf(stderr, "Erro ao abrir o catalogo de dados.\n");
        return false;
    }
    
    // Abre ou cria o arquivo de índice primário para garantir que ele exista
    FILE *ip = ip_abrir_ou_criar("arvore_primaria.bin");
    if (ip) {
        ip_fechar(ip);
    } else {
        std::printf("Aviso: Falha ao inicializar o arquivo de indice primario.\n");
    }
    return true;
}

// Lê um inteiro do terminal e descarta caracteres extras para evitar travamentos no buffer
int MenuMTG::ler_inteiro(const char *prompt) const {
    int valor;
    while (true) {
        std::printf("%s", prompt);
        if (std::scanf("%d", &valor) == 1) {
            int c;
            while ((c = std::getchar()) != '\n' && c != EOF); // Limpa caracteres extras da linha
            return valor;
        } else {
            std::printf("Entrada invalida. Digite um numero inteiro.\n");
            int c;
            while ((c = std::getchar()) != '\n' && c != EOF); // Limpa o buffer de entrada corrompido
        }
    }
}

// Lê uma string da entrada padrão e remove a quebra de linha final (\n) deixada pelo fgets
void MenuMTG::ler_string(const char *prompt, char *destino, int capacidade) const {
    std::printf("%s", prompt);
    std::fflush(stdout);
    if (std::fgets(destino, capacidade, stdin)) {
        size_t len = std::strlen(destino);
        if (len > 0 && destino[len - 1] == '\n') {
            destino[len - 1] = '\0';
        }
    }
}

// Imprime informações de depuração e faz a travessia lógica da LED
void MenuMTG::exibir_estado_sistema() {
    CabecalhoArquivo cab;
    if (!gda_ler_cabecalho(catalogo.arquivoDados, cab)) {
        std::printf("Erro ao ler cabecalho do arquivo de dados.\n");
        return;
    }
    std::printf("\n--- ESTATISTICAS DO SISTEMA ---\n");
    std::printf("Total de registros alocados em disco (incluindo removidos): %d\n", cab.totalRegistros);
    std::printf("Cabeca da LED (Lista de Espacos Disponives): %d\n", cab.ledCabeca);
    
    if (cab.ledCabeca == carta_const::LED_NULO) {
        std::printf("LED esta VAZIA (sem espacos reaproveitaveis no momento).\n");
    } else {
        // Percorre a lista encadeada da LED a partir da cabeça do cabeçalho
        std::printf("LED (Cadeia LIFO de reuso): ");
        int atual = cab.ledCabeca;
        int cont = 0;
        while (atual != carta_const::LED_NULO && cont < 100) {
            std::printf("[%d] -> ", atual);
            Carta c;
            if (gda_ler_registro(catalogo.arquivoDados, atual, c)) {
                atual = c.proximoLed; // Pula para o próximo offset livre da pilha
            } else {
                std::printf("ERRO ");
                break;
            }
            cont++;
        }
        std::printf("fim\n");
    }
    std::printf("--------------------------------\n\n");
}

void MenuMTG::inserir_nova_carta() {
    std::printf("\n--- INSERIR NOVA CARTA ---\n");
    int id = ler_inteiro("Digite o ID unico da carta: ");
    
    // Evita chaves duplicadas no índice primário
    Carta temp;
    if (crud_buscar_por_id(catalogo, id, temp)) {
        std::printf("Erro: Ja existe uma carta com o ID %d!\n", id);
        return;
    }
    
    Carta nova;
    carta_inicializar(nova);
    nova.id = id;
    
    nova.numeroDeColecao = ler_inteiro("Digite o numero de colecao: ");
    ler_string("Digite o nome da carta: ", nova.nome, sizeof(nova.nome));
    nova.custoEmMana = ler_inteiro("Digite o custo de mana (CMC): ");
    ler_string("Digite a cor principal: ", nova.cor, sizeof(nova.cor));
    ler_string("Digite o tipo da carta: ", nova.tipo, sizeof(nova.tipo));
    ler_string("Digite a raridade: ", nova.raridade, sizeof(nova.raridade));
    carta_copiar_texto(nova.linkImagem, "https://scryfall.com/", sizeof(nova.linkImagem));
    
    // Escreve em disco e insere nas árvores e listas secundárias
    int slot = crud_criar(catalogo, nova);
    if (slot >= 0) {
        std::printf("Carta inserida com sucesso no slot %d!\n", slot);
    } else {
        std::printf("Erro ao inserir a carta.\n");
    }
}

void MenuMTG::buscar_por_id() {
    std::printf("\n--- BUSCAR POR ID (ARVORE B+) ---\n");
    int id = ler_inteiro("Digite o ID da carta: ");
    Carta encontrada;
    // Realiza a busca no arquivo arvore_primaria.bin
    if (crud_buscar_por_id(catalogo, id, encontrada)) {
        std::printf("Carta encontrada!\n");
        carta_imprimir(encontrada);
    } else {
        std::printf("Carta com o ID %d nao foi encontrada no indice primario.\n", id);
    }
}

void MenuMTG::buscar_por_cor() {
    std::printf("\n--- BUSCAR POR COR ---\n");
    char cor[15];
    ler_string("Digite a cor para busca: ", cor, sizeof(cor));
    
    int posicoes[128];
    // Recupera os offsets usando o índice secundário e lista invertida de cores
    int n = crud_buscar_por_cor(catalogo, cor, posicoes, 128);
    std::printf("%d resultado(s) encontrado(s):\n", n);
    for (int i = 0; i < n; ++i) {
        Carta c;
        if (crud_ler_por_posicao(catalogo, posicoes[i], c)) {
            std::printf("  [Slot %d] ->", posicoes[i]);
            carta_imprimir(c);
        }
    }
}

void MenuMTG::buscar_por_cmc() {
    std::printf("\n--- BUSCAR POR CMC (CUSTO DE MANA) ---\n");
    int cmc = ler_inteiro("Digite o custo de mana (CMC): ");
    
    int posicoes[128];
    // Recupera os offsets usando o índice secundário e lista invertida de CMC
    int n = crud_buscar_por_cmc(catalogo, cmc, posicoes, 128);
    std::printf("%d resultado(s) encontrado(s):\n", n);
    for (int i = 0; i < n; ++i) {
        Carta c;
        if (crud_ler_por_posicao(catalogo, posicoes[i], c)) {
            std::printf("  [Slot %d] ->", posicoes[i]);
            carta_imprimir(c);
        }
    }
}

void MenuMTG::atualizar_carta() {
    std::printf("\n--- ATUALIZAR DADOS DA CARTA ---\n");
    int id = ler_inteiro("Digite o ID da carta que deseja atualizar: ");
    
    // Obtém o RRN do registro buscando pela árvore B+
    FILE *ip = ip_abrir_ou_criar("arvore_primaria.bin");
    int rrn = -1;
    bool achou = ip_buscar(ip, id, rrn);
    ip_fechar(ip);
    
    Carta antiga;
    if (achou && crud_ler_por_posicao(catalogo, rrn, antiga)) {
        std::printf("Carta atual encontrada no slot %d:\n", rrn);
        carta_imprimir(antiga);
        
        std::printf("Digite os novos dados (o ID continuara sendo %d):\n", id);
        Carta nova = antiga;
        
        nova.numeroDeColecao = ler_inteiro("Novo numero de colecao: ");
        ler_string("Novo nome da carta: ", nova.nome, sizeof(nova.nome));
        nova.custoEmMana = ler_inteiro("Novo custo de mana (CMC): ");
        ler_string("Nova cor principal: ", nova.cor, sizeof(nova.cor));
        ler_string("Novo tipo da carta: ", nova.tipo, sizeof(nova.tipo));
        ler_string("Nova raridade: ", nova.raridade, sizeof(nova.raridade));
        
        // Sobrescreve o registro e atualiza os ponteiros de índices se houver mudança de chaves secundárias
        if (crud_atualizar(catalogo, rrn, nova)) {
            std::printf("Carta atualizada com sucesso!\n");
        } else {
            std::printf("Erro ao atualizar a carta.\n");
        }
    } else {
        std::printf("Carta com o ID %d nao encontrada ou removida.\n", id);
    }
}

void MenuMTG::remover_carta() {
    std::printf("\n--- REMOVER CARTA (DELETE) ---\n");
    int id = ler_inteiro("Digite o ID da carta que deseja remover: ");
    
    // Busca a posição física antes de proceder com a exclusão
    FILE *ip = ip_abrir_ou_criar("arvore_primaria.bin");
    int rrn = -1;
    bool achou = ip_buscar(ip, id, rrn);
    ip_fechar(ip);
    
    Carta c;
    if (achou && crud_ler_por_posicao(catalogo, rrn, c)) {
        std::printf("Carta encontrada no slot %d:\n", rrn);
        carta_imprimir(c);
        
        char confirmacao[10];
        ler_string("Tem certeza que deseja remover esta carta? (s/n): ", confirmacao, sizeof(confirmacao));
        if (confirmacao[0] == 's' || confirmacao[0] == 'S') {
            // Executa a remoção lógica, atualiza índices secundários, remove da árvore primária e insere na LED
            if (crud_remover(catalogo, rrn)) {
                std::printf("Carta removida com sucesso!\n");
            } else {
                std::printf("Erro ao remover a carta.\n");
            }
        } else {
            std::printf("Remocao cancelada.\n");
        }
    } else {
        std::printf("Carta com o ID %d nao encontrada ou ja removida.\n", id);
    }
}

void MenuMTG::reconstruir_indices() {
    std::printf("\n--- RECONSTRUIR INDICES SECUNDARIOS ---\n");
    // Reconstrói as listas invertidas varrendo recursivamente o cartas.bin
    bool okCor = is_reconstruir(catalogo.indiceCor, caminhoDados);
    bool okCmc = is_reconstruir(catalogo.indiceCmc, caminhoDados);
    std::printf("Reconstrucao do indice de Cores: %s\n", okCor ? "SUCESSO" : "FALHA");
    std::printf("Reconstrucao do indice de CMC (Mana): %s\n", okCmc ? "SUCESSO" : "FALHA");
}

// Varre sequencialmente o arquivo cartas.bin exibindo os registros que não possuem flag de removido
void MenuMTG::listar_todas_cartas() {
    std::printf("\n--- LISTAR TODAS AS CARTAS ---\n");
    CabecalhoArquivo cab;
    if (!gda_ler_cabecalho(catalogo.arquivoDados, cab)) {
        std::printf("Erro ao ler cabecalho do arquivo de dados.\n");
        return;
    }
    
    int ativas = 0;
    for (int i = 0; i < cab.totalRegistros; ++i) {
        Carta c;
        if (gda_ler_registro(catalogo.arquivoDados, i, c)) {
            if (c.flagRemovido != carta_const::FLAG_REMOVIDO) {
                std::printf("  [Slot %d] ->", i);
                carta_imprimir(c);
                ativas++;
            }
        }
    }
    
    if (ativas == 0) {
        std::printf("Nenhuma carta ativa cadastrada no momento.\n");
    } else {
        std::printf("Total de cartas ativas exibidas: %d\n", ativas);
    }
}

void MenuMTG::executar_vacuum() {
    std::printf("\n--- EXECUTAR VACUUM (DESFRAGMENTADOR) ---\n");
    std::printf("Reorganizando o disco e removendo fisicamente registros excluídos...\n");
    if (crud_vacuum(catalogo)) {
        std::printf("Vacuum concluido com SUCESSO! Arquivo compactado e indices reconstruidos.\n");
    } else {
        std::printf("Falha ao executar o Vacuum.\n");
    }
}

// Insere automaticamente 10 cartas famosas de Magic no banco se elas não existirem
void MenuMTG::povoar_cartas_exemplo() {
    std::printf("\n--- POVOAR BANCO COM CARTAS DE EXEMPLO ---\n");
    std::printf("Inserindo cartas famosas de Magic: The Gathering...\n");

    struct Exemplo {
        int id;
        int numColecao;
        const char *nome;
        int cmc;
        const char *cor;
        const char *tipo;
        const char *raridade;
    };

    Exemplo exemplos[] = {
        {1, 1, "Black Lotus", 0, "Incolor", "Artefato", "Rara"},
        {2, 2, "Ancestral Recall", 1, "Azul", "Feitico", "Rara"},
        {3, 3, "Time Walk", 2, "Azul", "Feitico", "Rara"},
        {4, 4, "Lightning Bolt", 1, "Vermelho", "Instantanea", "Comum"},
        {5, 5, "Counterspell", 2, "Azul", "Instantanea", "Incomum"},
        {6, 6, "Llanowar Elves", 1, "Verde", "Criatura", "Comum"},
        {7, 7, "Sol Ring", 1, "Incolor", "Artefato", "Incomum"},
        {8, 8, "Swords to Plowshares", 1, "Branco", "Instantanea", "Incomum"},
        {9, 9, "Shivan Dragon", 6, "Vermelho", "Criatura", "Rara"},
        {10, 10, "Giant Growth", 1, "Verde", "Instantanea", "Comum"}
    };

    int inseridas = 0;
    int puladas = 0;

    for (const auto &ex : exemplos) {
        Carta temp;
        // Evita duplicatas com cartas já cadastradas
        if (crud_buscar_por_id(catalogo, ex.id, temp)) {
            puladas++;
            continue;
        }

        Carta nova;
        carta_inicializar(nova);
        nova.id = ex.id;
        nova.numeroDeColecao = ex.numColecao;
        nova.custoEmMana = ex.cmc;
        carta_copiar_texto(nova.nome, ex.nome, sizeof(nova.nome));
        carta_copiar_texto(nova.cor, ex.cor, sizeof(nova.cor));
        carta_copiar_texto(nova.tipo, ex.tipo, sizeof(nova.tipo));
        carta_copiar_texto(nova.raridade, ex.raridade, sizeof(nova.raridade));
        carta_copiar_texto(nova.linkImagem, "https://scryfall.com/", sizeof(nova.linkImagem));

        if (crud_criar(catalogo, nova) >= 0) {
            std::printf("  [+] Inserida: %s (ID: %d)\n", ex.nome, ex.id);
            inseridas++;
        } else {
            std::printf("  [!] Falha ao inserir: %s\n", ex.nome);
        }
    }

    std::printf("\nProcesso concluido!\n");
    std::printf("  Cartas inseridas: %d\n", inseridas);
    std::printf("  Cartas puladas (ja existentes): %d\n", puladas);
}

void MenuMTG::executar() {
    std::printf("=== Catalogo MTG - Sistema de Persistencia e Indexacao ===\n");
    std::printf("Tamanho do registro em disco: %zu bytes\n", sizeof(Carta));
    
    while (true) {
        std::printf("\n=========================================\n");
        std::printf("               MENU MTG\n");
        std::printf("=========================================\n");
        std::printf("1. Inserir Nova Carta (Create)\n");
        std::printf("2. Buscar Carta por ID (Read - Arvore B+)\n");
        std::printf("3. Buscar Cartas por Cor (Read - Secundario)\n");
        std::printf("4. Buscar Cartas por CMC (Read - Secundario)\n");
        std::printf("5. Atualizar Dados da Carta (Update)\n");
        std::printf("6. Remover Carta (Delete)\n");
        std::printf("7. Reconstruir Indices Secundarios\n");
        std::printf("8. Exibir Estado do Sistema (Debug LED)\n");
        std::printf("9. Listar Todas as Cartas (Sequential Scan)\n");
        std::printf("10. Executar Vacuum (Desfragmentador)\n");
        std::printf("11. Povoar Banco com Cartas de Exemplo\n");
        std::printf("12. Sair\n");
        std::printf("=========================================\n");
        
        int opcao = ler_inteiro("Escolha uma opcao: ");
        
        if (opcao == 12) {
            std::printf("Saindo do programa. Ate logo!\n");
            break;
        }
        
        switch (opcao) {
            case 1: inserir_nova_carta(); break;
            case 2: buscar_por_id(); break;
            case 3: buscar_por_cor(); break;
            case 4: buscar_por_cmc(); break;
            case 5: atualizar_carta(); break;
            case 6: remover_carta(); break;
            case 7: reconstruir_indices(); break;
            case 8: exibir_estado_sistema(); break;
            case 9: listar_todas_cartas(); break;
            case 10: executar_vacuum(); break;
            case 11: povoar_cartas_exemplo(); break;
            default:
                std::printf("Opcao invalida. Escolha um numero de 1 a 12.\n");
                break;
        }
    }
}
