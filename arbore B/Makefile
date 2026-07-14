# ============================================================================
#  Makefile — Sistema de Persistencia e Indexacao (catalogo MTG)
# ----------------------------------------------------------------------------
#  Uso:
#     make          -> compila o executavel 'arbore'
#     make run      -> compila e executa o demo
#     make clean    -> remove o binario e os arquivos .bin gerados
#
#  Os nomes de diretorio tem espacos (convencao "Title Case With Spaces/"),
#  por isso TODOS os caminhos de fonte ficam entre aspas.
# ============================================================================

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra
ALVO     := arbore

FONTES := main.cpp \
          "Carta/carta.cpp" \
          "Gerenciador De Arquivo/gerenciador_de_arquivo.cpp" \
          "Led/led.cpp" \
          "Indice Secundario/indice_secundario.cpp" \
          "Indice Primario/indice_primario.cpp" \
          "Operacoes Crud/operacoes_crud.cpp" \
          "Menu/menu.cpp"

.PHONY: all run clean

all:
	$(CXX) $(CXXFLAGS) -o $(ALVO) $(FONTES)

run: all
	./$(ALVO)

clean:
	rm -f $(ALVO) $(ALVO).exe cartas.bin index_sec_cor.bin lista_inv_cor.bin \
	      index_sec_cmc.bin lista_inv_cmc.bin indice_primario.bin
