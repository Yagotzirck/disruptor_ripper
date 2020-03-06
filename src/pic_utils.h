#ifndef PIC_UTILS_H
#define PIC_UTILS_H

#include "utils.h"

typedef struct psx_palette_s{
    union{
        struct colors_s{
            WORD red:  5;
            WORD green: 5;
            WORD blue:   5;
            WORD semiTransparency: 1;
        }colors;
        WORD pixel;
    }psxPixel;
}psx_palette_t;

/* functions declarations */
void pic_handler(const BYTE *spriteData, DWORD width, DWORD height, DWORD palette);
void pic_handler_embedded16Colors(BYTE *spriteData, DWORD width, DWORD height, DWORD palette);
void spriteDecode(BYTE *src, BYTE *dest, int destSize);


#endif /* PIC_UTILS_H */
