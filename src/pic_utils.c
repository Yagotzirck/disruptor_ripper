#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pic_utils.h"
#include "tga_utils.h"

extern char path[];

/* functions definitions */
void pic_handler(const BYTE *spriteData, DWORD width, DWORD height, DWORD palette){
    FILE *tga_fp;

    BYTE *bufToWrite;

    int shrunkSize;
    WORD CMapLen;

    enum tgaImageType imgType;

    DWORD rawBytesToWrite;

    BYTE shrunkBuf[256 * 256];
    BYTE spriteData2[256 * 256];


    if((tga_fp = fopen(path, "wb")) == NULL){
        fprintf(stderr, "Couldn't create %s\n", path);
        perror("Reason");
        exit(EXIT_FAILURE);
    }

    memcpy(spriteData2, spriteData, width * height);

    shrunkSize = shrink_tga(shrunkBuf, spriteData2, width * height, &CMapLen, palette);

    /* RLE compression resulted in increased data size */
    if(shrunkSize == -1){
        bufToWrite = spriteData2;
        imgType = IMGTYPE_COLORMAPPED;
        rawBytesToWrite = width * height;
    }
    /* RLE compression worked fine; save the compressed data */
    else{
        bufToWrite = shrunkBuf;
        imgType = IMGTYPE_COLORMAPPED_RLE;
        rawBytesToWrite = shrunkSize;
    }


    /* set and write the tga header */
    set_tga_hdr(PALETTED, imgType, CMapLen, 32, width, height, 8, ATTRIB_BITS | TOP_LEFT);
    write_tga_hdr(tga_fp);


    /* write the tga palette */
    write_shrunk_tga_pal(tga_fp);

    /* write the image data */
    fwrite(bufToWrite, 1, rawBytesToWrite, tga_fp);

    fclose(tga_fp);
}

void pic_handler_embedded16Colors(BYTE *spriteData, DWORD width, DWORD height, DWORD palette){
    FILE *tga_fp;

    BYTE *bufToWrite;

    int shrunkSize;
    const WORD CMapLen = 16;

    enum tgaImageType imgType;

    DWORD rawBytesToWrite;

    BYTE shrunkBuf[384 * 96 * 2];
    BYTE spriteData16Cols[384 * 96];

    if((tga_fp = fopen(path, "wb")) == NULL){
        fprintf(stderr, "Couldn't create %s\n", path);
        perror("Reason");
        exit(EXIT_FAILURE);
    }

    {
        unsigned i;
        for(i = 0; i < width * height; ++i)
            spriteData16Cols[i] = spriteData[i] & 0xF;
    }


    // convert 4 bpp sprite data to 8 bpp
    /*
    {
        unsigned i;
        for(i = 0; i < width / 2 * height; ++i){
            spriteData16Cols[i*2] = spriteData[i] >> 4;
            spriteData16Cols[i*2 + 1] = spriteData[i] & 0xF;
        }
    }
    */


    //shrunkSize = shrink_tga(shrunkBuf, spriteData2, width * height, &CMapLen, palette);
    shrunkSize = tga_compressData(shrunkBuf, spriteData16Cols, width * height);

    /* RLE compression resulted in increased data size */
    if(shrunkSize == -1){
        bufToWrite = spriteData16Cols;
        imgType = IMGTYPE_COLORMAPPED;
        rawBytesToWrite = width * height;
    }
    /* RLE compression worked fine; save the compressed data */
    else{
        bufToWrite = shrunkBuf;
        imgType = IMGTYPE_COLORMAPPED_RLE;
        rawBytesToWrite = shrunkSize;
    }


    /* set and write the tga header */
    set_tga_hdr(PALETTED, imgType, CMapLen, 32, width, height, 8, ATTRIB_BITS | TOP_LEFT);
    write_tga_hdr(tga_fp);


    /* write the tga palette */
    write_tga_pal(tga_fp, palette);

    /* write the image data */
    fwrite(bufToWrite, 1, rawBytesToWrite, tga_fp);

    fclose(tga_fp);
}

void spriteDecode(BYTE *src, BYTE *dest, int destSize){

    BYTE *a2, *a1, *a3;

    BYTE *t0;

    int t2;
    int t4;

    memset(dest, 0, destSize);

    {
        unsigned int temp_a1, temp_a2, temp_a3;

        temp_a2 = *(DWORD *)src;
        temp_a3 = *(DWORD *)(src + 4);
        temp_a1 = *(DWORD *)(src + 8);

        src += 12;

        a2 = src + temp_a2;
        t0 = a2 + 3;
        a3 = a2 + temp_a3;
        a1 = a3 + temp_a1;
    }

    while( (t2 = *src++) != 0xFF){

        if(t2 & 0x80){
            dest += t2 & 0x7F;
            continue;
        }

        dest += t2;

        switch(t2 & 3){
            case 0:
                break;

            case 1:
                t2 = *(DWORD *)a2;
                a2 += 4;

                t2 <<= 8;
                *(DWORD *)(dest - 1) = t2;
                dest += 3;

                break;

            case 2:
                *(WORD *)dest = *(WORD *)a3;
                a3 += 2;
                dest += 2;

                break;

            case 3:
                *dest++ = *t0;
                t0 += 4;

                break;
        }

        t2 = *src++;

        t4 = (t2 & 0x3F) * 4;

        memcpy(dest, a1, t4);
        dest += t4;
        a1 += t4;

        switch(t2 >> 6){
            case 0:
                break;

            case 1:
                *dest = *t0;

                dest += 4;
                t0 += 4;
                break;

            case 2:
                *(WORD *)dest = *(WORD *)a3;

                dest += 4;
                a3 += 2;
                break;

            case 3:
                *(DWORD *)dest = (*(DWORD *)a2) & 0x00FFFFFF;

                dest += 4;
                a2 += 4;
                break;
        }
    }
}
