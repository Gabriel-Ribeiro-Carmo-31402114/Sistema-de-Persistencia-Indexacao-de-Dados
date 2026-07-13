#ifndef LED_HPP
#define LED_HPP

#include <cstdio>
#include "../Carta/carta.hpp"
#include "../Gerenciador De Arquivo/gerenciador_de_arquivo.hpp"

// Gerenciador de espaco livre (LED - Lista de Espacos Disponiveis).
// Implementa uma pilha (LIFO) de registros logicamente removidos para reaproveitamento de espaco.

// Insere (push) um slot livre na LED no esquema LIFO (Pilha).
bool led_empilhar_slot(FILE *arquivo, int indice);

// Obtem (pop) um slot livre da LED para reuso ou retorna o final do arquivo para append.
int led_obter_slot_livre(FILE *arquivo, bool &reutilizado);

#endif // LED_HPP
