# Sistema-de-Persistência-Indexação-de-Dados
Destinado como um dos requisitos de avaliação da disciplina de Organização e Recuperação da Informação, este projeto presta-se a implementar uma camada de persistência e indexação de dados. Orientada ao domínio de um catálogo simples para o TCG "Magic: The Gathering", realiza busca, inserção e remoção de registros contendo dados de cartas.

<br>

## Integrantes do Grupo
* **Diego Armando Enriquez Martinez** - [805070]
* **Gabriel Ribeiro Almeida do Carmo** - [845242]
* **Rafael Jeronimo Nunes Vasconcelo** - [795416]
* **Salvatore Silva Costanzo** - [771576]

<br>

## Descrição do Trabalho
* **Tema da Aplicação:** Catálogo de Cartas de Magic através de uma Árvore B+
* **Estrutura de Dados:** Cada "struct Carta" na estrutura de arquivos possui tamanho fixo de exatamente 200 bytes
  
| tipo | campo | descrição |
| :--- | :--- | :--- |
| char | `flagRemovido` | gravestone do arquivo |
| int | `proximoLed` | próximo na lista de espaços disponíveis |
| int | `id` | número identificador único da carta |
| int | `numeroDeColecao` | Número da Carta na Coleção |
| char | `nome[50]` | nome da carta |
| int | `custoEmMana` | cmc: custo em mana |
| char | `cor[15]` | cor principal da arte da carta |
| char | `tipo[30]` | ex: Terreno, ou Criatura, ou Feitiço, ou Artefato ... |
| char | `raridade[12]` | raridade da carte |
| char | `linkImagem[76]` | link para o scryfall, onde está a imagem da carta |


* **Chave Primária:** campo "id"
* **Chaves Secundárias:** campos "cor" e "tipo"
* **Descrição dos Arquivos:**
  1. `cartas.bin`: Arquivo de dados principal que armazena os registros de 200 bytes das cartas
  1. `indice_primario.bin`: Armazena a estrutura de árvore com nós folha (conteúdo "id" e "offset") apontando para o arquivo principal ("cartas.bin").
  1. `index_sec_cor.bin` e `lista_inv_cor.bin`: Arquivo da chave secundário e da lista invertida de encadeamento para buscar a carte pela "cor".
  1. `index_sec_cmc.bin` e `lista_inv_cmc.bin`: Arquivo da chave secundária e da lista invertida para buscar pelo "cmc".


<br>

## Padronização de Código
Para evitar conflitos de nomenclatura e manter a consistência no projeto, adotou-se as seguintes convenções:
> **Exemplo Base de Referência:** *Fulano Ciclano Corre*

| Elemento de Código | Exemplo Prático |
| :--- | :--- |
| Diretório | `Fulano Ciclano Corre/` |
| Arquivo | `fulano_ciclano_corre.cpp` |
| Variável | `fulanoCiclanoCorre` |
| Funções | `fulano_ciclano_corre()` |

<br>

## Tecnologias e Compilação
O projeto foi construído utilizando as bibliotecas nativas da linguagem C++ para manipulação de arquivos em disco (`fseek`, `fread`, `fwrite`).

<br>

## Comanda Principal do Trabalho
> **Abaixo estão os tópicos de avaliação atendidos pela equipe :**

| Métrica Avaliada | Valor em Pontos | Foi Realizado? |
| :--- | :--- | :--- |
| Operações de CRUD Completas em Disco | 3.0 pts | não |
| Gerenciamento de Espaço (LED) | 1.0 pts | não |
| Indexação Secundária e Lista Invertida | 3.0 pts | não |
| Indexação Primária com Árvore B | 3.0 pts | não |

<br>

## Comanda Secundária: Pts Extras
> **Abaixo estão os tópicos de avaliação atendidos pela equipe :**

| Métrica Avaliada | Valor em Pontos | Foi Realizado? |
| :--- | :--- | :--- |
| Registros de Tamanho Variável | 2.0 pts | não |
| Árvore B+ em Disco | 0.5 pts | não |
| Vacuum (Desfragmentador) | 0.5 pts | não |


