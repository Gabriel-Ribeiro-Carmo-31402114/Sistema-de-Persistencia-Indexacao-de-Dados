# Sistema-de-Persistência-Indexação-de-Dados
Destinado como um dos requisitos de avaliação da disciplina de Organização e Recuperação da Informação, este projeto presta-se a implementar uma camada de persistência e indexação de dados. (trecho a seguir ainda estah a ser definido) 
Orientada ao domínio de um catálogo simples para o TCG "Magic: The Gathering", realiza busca, inserção e remoção de registros contendo dados de cartas.
---
## Integrantes do Grupo
* **Pessoa 1** - [Link do GitHub]
* **Pessoa 2** - [Link do GitHub]
* **Pessoa 3** - [Link do GitHub]
* **Pessoa 4** - [Link do GitHub]
---
# Padronização de Código
Para evitar conflitos de nomenclatura e manter a consistência de todo o ecossistema do projeto, o grupo adotou estritamente as seguintes convenções de estilo.
> **Exemplo Base de Referência:** *Fulano Ciclano Corre*

| Elemento de Código | Padrão Adotado | Exemplo Prático |
| :--- | :--- | :--- |
| **Diretório** | `PascalCase` (Iniciais Maiúsculas) | `Fulano Ciclano Corre/` |
| **Arquivo** | `snake_case` (Minúsculas com underline) | `fulano_ciclano_corre.cpp` |
| **Variável** | `camelCase` (Primeira minúscula, restantes maiúsculas) | `fulanoCiclanoCorre` |
| **Função** | `snake_case()` (Minúsculas com parênteses) | `fulano_ciclano_corre()` |
---
## Tecnologias e Compilação

O projeto foi construído utilizando apenas as bibliotecas nativas da linguagem C++ para manipulação binária de arquivos em disco (`fseek`, `fread`, `fwrite`).
---
