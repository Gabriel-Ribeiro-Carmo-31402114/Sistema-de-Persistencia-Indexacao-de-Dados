# Sistema de Persistência e Indexação de Dados

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
* **Estrutura de Dados:** Cada `struct Carta` na estrutura de arquivos possui tamanho fixo de exatamente **200 bytes**, facilitando o cálculo de offsets diretos em disco:

| tipo | campo | descrição | tamanho |
| :--- | :--- | :--- | :--- |
| `char` | `flagRemovido` | Gravestone do arquivo | 1 byte |
| `int` | `proximoLed` | Guarda o offset do próximo espaço disponível na LED | 4 bytes |
| `int` | `id` | Número identificador único da carta | 4 bytes |
| `int` | `numeroDeColecao` | Número da carta na coleção física | 4 bytes |
| `char` | `nome[50]` | Nome da carta | 50 bytes |
| `int` | `custoEmMana` | Custo de Mana Convertido (CMC) | 4 bytes |
| `char` | `cor[15]` | cor principal da carta | 15 bytes |
| `char` | `tipo[30]` | Categoria da carta (ex: Criatura, Feitiço) | 30 bytes |
| `char` | `raridade[12]` | Nível de raridade da carta | 12 bytes |
| `char` | `linkImagem[76]` | URL para o scryfall, onde está a imagem da carta | 76 bytes |

* **Chave Primária:** Campo "id" 
* **Chaves Secundárias:** Campos "cor" e "tipo"

* **Descrição dos Arquivos do Systema:**
  1. `cartas.bin: Arquivo de dados principal que armazena os registros de 200 bytes das cartas.
  2. `indice_primario.bin` armazena a estrutura da árvore b+ cujo nós folhas contêm o "id" e o "offset", apontando para o arquivo principal.
  3. `index_sec_cor.bin` e `lista_inv_cor.bin`: Arquivo da chave secundária e da lista invertida para busca pela cor da carta.
  4. `index_sec_tipo.bin` e `lista_inv_tipo.bin`: Arquivo da chave secundária o e da lista invertida para busca por tipos de carta.

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
