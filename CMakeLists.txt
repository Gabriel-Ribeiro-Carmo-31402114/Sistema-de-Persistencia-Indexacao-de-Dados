cmake_minimum_required(VERSION 3.20)
project(SistemaMTG CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Inclui os cabeçalhos da pasta 'arbore B'
include_directories("arbore B")

# Fontes do executável de demonstração principal
set(SOURCES
    "arbore B/main.cpp"
    "arbore B/Carta/carta.hpp"
    "arbore B/Carta/carta.cpp"
    "arbore B/Gerenciador De Arquivo/gerenciador_de_arquivo.hpp"
    "arbore B/Gerenciador De Arquivo/gerenciador_de_arquivo.cpp"
    "arbore B/Led/led.hpp"
    "arbore B/Led/led.cpp"
    "arbore B/Indice Primario/indice_primario.hpp"
    "arbore B/Indice Primario/indice_primario.cpp"
    "arbore B/Indice Secundario/indice_secundario.hpp"
    "arbore B/Indice Secundario/indice_secundario.cpp"
    "arbore B/Operacoes Crud/operacoes_crud.hpp"
    "arbore B/Operacoes Crud/operacoes_crud.cpp"
    "arbore B/Menu/menu.hpp"
    "arbore B/Menu/menu.cpp"
)

# Alvo do executável principal
add_executable(arbore ${SOURCES})

# Fontes do executável de testes unitários
set(TEST_SOURCES
    "arbore B/tests/test_arbore.cpp"
    "arbore B/Carta/carta.hpp"
    "arbore B/Carta/carta.cpp"
    "arbore B/Gerenciador De Arquivo/gerenciador_de_arquivo.hpp"
    "arbore B/Gerenciador De Arquivo/gerenciador_de_arquivo.cpp"
    "arbore B/Led/led.hpp"
    "arbore B/Led/led.cpp"
    "arbore B/Indice Primario/indice_primario.hpp"
    "arbore B/Indice Primario/indice_primario.cpp"
    "arbore B/Indice Secundario/indice_secundario.hpp"
    "arbore B/Indice Secundario/indice_secundario.cpp"
    "arbore B/Operacoes Crud/operacoes_crud.hpp"
    "arbore B/Operacoes Crud/operacoes_crud.cpp"
)

# Alvo de testes
add_executable(test_arbore ${TEST_SOURCES})
