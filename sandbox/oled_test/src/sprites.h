#ifndef SPRITES_H
#define SPRITES_H

#include <stdlib.h>                   
#include <stdint.h>

typedef struct Sprite{                
  uint8_t width;
  uint8_t height;
  const uint8_t *content;
} sprite;

extern sprite sprite0;

#endif // SPRITES_H