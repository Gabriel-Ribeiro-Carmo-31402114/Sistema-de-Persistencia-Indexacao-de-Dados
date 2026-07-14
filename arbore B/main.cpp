#include "Menu/menu.hpp"

int main() {
    MenuMTG menu("cartas.bin");
    if (!menu.inicializar()) {
        return 1;
    }
    menu.executar();
    return 0;
}
