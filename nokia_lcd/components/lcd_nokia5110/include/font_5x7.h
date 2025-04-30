#ifndef FONT_5X7_H
#define FONT_5X7_H

#include <stdint.h>

#define FONT_5X7_WIDTH   5
#define FONT_5X7_HEIGHT  8  // Incluye 1px de espacio debajo

// Tabla de caracteres ASCII (32-127)
extern const uint8_t font_5x7[96][5];

#endif // FONT_5X7_H
