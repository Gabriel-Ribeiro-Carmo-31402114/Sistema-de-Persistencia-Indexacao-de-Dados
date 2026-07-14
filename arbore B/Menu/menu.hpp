#ifndef MENU_HPP
#define MENU_HPP

#include "../Operacoes Crud/operacoes_crud.hpp"

class MenuMTG {
private:
    CatalogoCartas catalogo;
    const char *caminhoDados;

    // Métodos para leitura segura de entrada do terminal (tratam buffers e entradas inválidas)
    int ler_inteiro(const char *prompt) const;
    void ler_string(const char *prompt, char *destino, int capacidade) const;
    
    // Mostra o total de registros e percorre a LED (Offsets disponíveis para reuso)
    void exibir_estado_sistema();

    // Funções internas que realizam cada uma das operações do menu interativo
    void inserir_nova_carta();
    void buscar_por_id();
    void buscar_por_cor();
    void buscar_por_cmc();
    void atualizar_carta();
    void remover_carta();
    void reconstruir_indices();
    void listar_todas_cartas();
    void executar_vacuum();
    void povoar_cartas_exemplo();

public:
    MenuMTG(const char *caminhoDados);
    ~MenuMTG();

    // Inicializa o CRUD e verifica se o arquivo da Árvore B+ existe/foi criado
    bool inicializar();

    // Loop de controle que exibe o menu e direciona as opções escolhidas
    void executar();
};

#endif // MENU_HPP
