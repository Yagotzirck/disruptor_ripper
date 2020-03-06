#include <stdio.h>

#include "tga_utils.h"
#include "utils.h"
#include "pic_utils.h"


typedef struct _TgaHeader
{
  BYTE IDLength;        /* 00h  Size of Image ID field */
  BYTE ColorMapType;    /* 01h  Color map type */
  BYTE ImageType;       /* 02h  Image type code */
  WORD CMapStart;       /* 03h  Color map origin */
  WORD CMapLength;      /* 05h  Color map length */
  BYTE CMapDepth;       /* 07h  Depth of color map entries */
  WORD XOffset;         /* 08h  X origin of image */
  WORD YOffset;         /* 0Ah  Y origin of image */
  WORD Width;           /* 0Ch  Width of image */
  WORD Height;          /* 0Eh  Height of image */
  BYTE PixelDepth;      /* 10h  Image pixel size */
  BYTE ImageDescriptor; /* 11h  Image descriptor byte */
} TGAHEAD;

/*
struct tga_palette_555_s{
    WORD red:  5;
    WORD green: 5;
    WORD blue:   5;
    WORD alpha: 1;
};
*/

struct tga_palette_s{
    BYTE blue, green, red, alpha;
};

/* global variables (file scope only) */
static TGAHEAD tga_header;
static struct tga_palette_s tga_palette[2][256], tga_shrunk_palette[256];


/* local functions declarations */
static WORD shrink_palette(BYTE imgBuf[], DWORD size, BYTE used_indexes[], DWORD palette);

static DWORD findMaxOf3(DWORD x, DWORD y, DWORD z);

/* functions definitions */
void psxToTgaPal(const psx_palette_t *psx_palette){
    unsigned int i;

    unsigned int maxComponent;
    double coefficient;


    tga_palette[0][0].blue = 0;
    tga_palette[0][0].green = 0;
    tga_palette[0][0].red = 0;
    tga_palette[0][0].alpha = 0;

    for(i = 1; i < 256; ++i){
            tga_palette[0][i].blue = (psx_palette[i].psxPixel.colors.blue * 255) / 63;
            tga_palette[0][i].green = (psx_palette[i].psxPixel.colors.green * 255) / 63;
            tga_palette[0][i].red = (psx_palette[i].psxPixel.colors.red * 255) / 63;

            if(psx_palette[i].psxPixel.colors.semiTransparency){
                maxComponent = findMaxOf3(tga_palette[0][i].red, tga_palette[0][i].green, tga_palette[0][i].blue);
                tga_palette[0][i].alpha = maxComponent;

                coefficient = 255.0 / maxComponent;

                tga_palette[0][i].red *= coefficient;
                tga_palette[0][i].green *= coefficient;
                tga_palette[0][i].blue *= coefficient;
            }
            else
                tga_palette[0][i].alpha = 255;
    }


    psx_palette += 256; // second psx palette

    tga_palette[1][0].blue = 0;
    tga_palette[1][0].green = 0;
    tga_palette[1][0].red = 0;
    tga_palette[1][0].alpha = 0;

    for(i = 1; i < 256; ++i){
        tga_palette[1][i].blue = (psx_palette[i].psxPixel.colors.blue * 255) / 63;
        tga_palette[1][i].green = (psx_palette[i].psxPixel.colors.green * 255) / 63;
        tga_palette[1][i].red = (psx_palette[i].psxPixel.colors.red * 255) / 63;

        if(psx_palette[i].psxPixel.colors.semiTransparency == 1 && psx_palette[i].psxPixel.pixel != 0x8000){
            maxComponent = findMaxOf3(tga_palette[1][i].red, tga_palette[1][i].green, tga_palette[1][i].blue);
            tga_palette[1][i].alpha = maxComponent;

            coefficient = 255.0 / maxComponent;

            tga_palette[1][i].red *= coefficient;
            tga_palette[1][i].green *= coefficient;
            tga_palette[1][i].blue *= coefficient;
        }
        else
            tga_palette[1][i].alpha = 255;

    }


}


void set_tga_hdr(enum tgaColorMap isCMapped, enum tgaImageType imgType, WORD CMapLen, BYTE CMapDepth, WORD width, WORD height, BYTE PixelDepth, enum tgaImageDescriptor ImageDesc){
    tga_header.IDLength =          0;           /* No image ID field used, size 0 */
    tga_header.ColorMapType =      isCMapped;
    tga_header.ImageType =         imgType;
    tga_header.CMapStart =         0;           /* Color map origin */
    tga_header.CMapLength =        CMapLen;     /* Number of palette entries */
    tga_header.CMapDepth =         CMapDepth;   /* Depth of color map entries */
    tga_header.XOffset =           0;           /* X origin of image */
    tga_header.YOffset =           0;           /* Y origin of image */
    tga_header.Width =             width;       /* Width of image */
    tga_header.Height =            height;      /* Height of image */
    tga_header.PixelDepth =        PixelDepth;  /* Image pixel size */
    tga_header.ImageDescriptor =   ImageDesc;
}

void write_tga_hdr(FILE *stream){
    fwrite(&tga_header.IDLength, sizeof(tga_header.IDLength), 1, stream);
    fwrite(&tga_header.ColorMapType, sizeof(tga_header.ColorMapType), 1, stream);
    fwrite(&tga_header.ImageType, sizeof(tga_header.ImageType), 1, stream);
    fwrite(&tga_header.CMapStart, sizeof(tga_header.CMapStart), 1, stream);
    fwrite(&tga_header.CMapLength, sizeof(tga_header.CMapLength), 1, stream);
    fwrite(&tga_header.CMapDepth, sizeof(tga_header.CMapDepth), 1, stream);
    fwrite(&tga_header.XOffset, sizeof(tga_header.XOffset), 1, stream);
    fwrite(&tga_header.YOffset, sizeof(tga_header.YOffset), 1, stream);
    fwrite(&tga_header.Width , sizeof(tga_header.Width), 1, stream);
    fwrite(&tga_header.Height, sizeof(tga_header.Height), 1, stream);
    fwrite(&tga_header.PixelDepth, sizeof(tga_header.PixelDepth), 1, stream);
    fwrite(&tga_header.ImageDescriptor, sizeof(tga_header.ImageDescriptor), 1, stream);
}

void write_tga_pal(FILE *stream, DWORD palette){
    fwrite(tga_palette[palette], sizeof(struct tga_palette_s), tga_header.CMapLength, stream);
}

void write_shrunk_tga_pal(FILE *stream){
    fwrite(tga_shrunk_palette, sizeof(struct tga_palette_s), tga_header.CMapLength, stream);
}

/* tga_compressData(): compress data(RLE encoding) but keep the palette intact */
int tga_compressData(BYTE imgDest[], const BYTE imgBuf[], DWORD size){
BYTE used_indexes[256] = {0};
    unsigned int i = 0, j = 0;

    BYTE pixelCount = 0;    // this is TGA's RLE-packet counter field; the real count is pixelCount + 1

    do{
        while(i + 1 < size && imgBuf[i+1] == imgBuf[i]){
            ++pixelCount;
            ++i;

            if(pixelCount == 127)
                break;
        }

        /* we got a gain (or uninflated size if 2 consecutive pixels only:
        ** pixelCount byte + pixel byte VS. pixel byte repeated 2 times)
        */
        if(pixelCount){
            imgDest[j++] =  pixelCount | 0x80;    // this is a RLE packet, so the MSB must be set
            imgDest[j++] =  used_indexes[imgBuf[i++]];
            pixelCount = 0;
        }
        else{ // check how many subsequent pixels are uncompressible by RLE
            unsigned int identicalPixelsCount = 0;

            BYTE next_pixelCount = 0;   // the starting pixelCount value for the next packet

            /* save the index in which we put pixelCount later on, once we know the pixel count value
            ** for the current non-RLE packet
            */
            unsigned int packet_pixelCountIdx = j++;

            // put the 1st uncompressible pixel in the destination buffer
            imgDest[j++] = used_indexes[imgBuf[i++]];


            /* in order to avoid inflating the image data when breaking the
            ** run of unidentical pixels, we need at least 3 consecutive identical pixels;
            ** breaking at 2 would inflate data by 1 byte due to an extra pixelCount byte.
            **
            ** As an example consider the following string, where the letters represent pixel bytes
            ** and numbers represent pixelCount bytes, e.g. "abcddefghhh" (11 bytes):
            **
            ** (For clarity's sake, pixelCount's values in this dummy example indicate the real packet's
            ** length, and not (packet length) - 1 with MSB set for RLE packets, as happens in TGA encoding)
            **
            ** - breaking at the double d's gives "3abc2d3efg3h", resulting in 12 bytes;
            ** - breaking at the triple h's gives "8abcddefg3h" , resulting in 11 bytes.
            **
            ** By breaking at the triple h's, we got the same size as the original data,
            ** thus we avoided inflating it.
            */
            do{
                if(i + 1 >= size){
                    /* if pixelCount is 0, image's last pixel has already been saved
                    ** immediately before entering this loop;
                    ** otherwise, the last pixel belongs to a pixel run and must be saved
                    */
                    if(pixelCount){
                        // put the uncompressible pixel in the destination buffer
                        imgDest[j++] = used_indexes[imgBuf[i++]];
                        ++pixelCount;
                    }
                    break;
                }

                if(imgBuf[i+1] == imgBuf[i])
                    ++identicalPixelsCount;
                else
                    identicalPixelsCount = 0;

                /* 3 subsequent identical pixels counted(possibly more after);
                ** we can break the run of uncompressible pixels without inflating the data
                */
                if(identicalPixelsCount == 2){
                    /* the last pixel which has been put in the current packet must be discarded, since it belongs
                    ** to the next RLE packet
                    */
                    --pixelCount;
                    --j;

                    /* we already counted 3 identical pixels; no need to count them again in the next
                    ** RLE counting loop
                    */
                    next_pixelCount = 2;
                    ++i;
                    break;
                }

                // put the uncompressible pixel in the destination buffer
                imgDest[j++] = used_indexes[imgBuf[i++]];
                ++pixelCount;

            }while(pixelCount < 127);

            // put pixelCount in the destination buffer for the current non-RLE packet
            imgDest[packet_pixelCountIdx] = pixelCount;

            // set the number of identical pixels counted for the next packet
            pixelCount = next_pixelCount;
        }
    }while(i < size);

    /* if the RLE compression resulted in increased size keep the data in its raw form */
    if(j >= size)
        return -1;

    return j;
}

/* shrink_tga(): compress both palette (by deleting unused entries) and data(RLE encoding)*/
int shrink_tga(BYTE imgDest[], BYTE imgBuf[], DWORD size, WORD *CMapLen, DWORD palette){
    BYTE used_indexes[256] = {0};
    unsigned int i = 0, j = 0;

    *CMapLen = shrink_palette(imgBuf, size, used_indexes, palette);
    BYTE pixelCount = 0;    // this is TGA's RLE-packet counter field; the real count is pixelCount + 1

    do{
        while(i + 1 < size && imgBuf[i+1] == imgBuf[i]){
            ++pixelCount;
            ++i;

            if(pixelCount == 127)
                break;
        }

        /* we got a gain (or uninflated size if 2 consecutive pixels only:
        ** pixelCount byte + pixel byte VS. pixel byte repeated 2 times)
        */
        if(pixelCount){
            imgDest[j++] =  pixelCount | 0x80;    // this is a RLE packet, so the MSB must be set
            imgDest[j++] =  used_indexes[imgBuf[i++]];
            pixelCount = 0;
        }
        else{ // check how many subsequent pixels are uncompressible by RLE
            unsigned int identicalPixelsCount = 0;

            BYTE next_pixelCount = 0;   // the starting pixelCount value for the next packet

            /* save the index in which we put pixelCount later on, once we know the pixel count value
            ** for the current non-RLE packet
            */
            unsigned int packet_pixelCountIdx = j++;

            // put the 1st uncompressible pixel in the destination buffer
            imgDest[j++] = used_indexes[imgBuf[i++]];


            /* in order to avoid inflating the image data when breaking the
            ** run of unidentical pixels, we need at least 3 consecutive identical pixels;
            ** breaking at 2 would inflate data by 1 byte due to an extra pixelCount byte.
            **
            ** As an example consider the following string, where the letters represent pixel bytes
            ** and numbers represent pixelCount bytes, e.g. "abcddefghhh" (11 bytes):
            **
            ** (For clarity's sake, pixelCount's values in this dummy example indicate the real packet's
            ** length, and not (packet length) - 1 with MSB set for RLE packets, as happens in TGA encoding)
            **
            ** - breaking at the double d's gives "3abc2d3efg3h", resulting in 12 bytes;
            ** - breaking at the triple h's gives "8abcddefg3h" , resulting in 11 bytes.
            **
            ** By breaking at the triple h's, we got the same size as the original data,
            ** thus we avoided inflating it.
            */
            do{
                if(i + 1 >= size){
                    /* if pixelCount is 0, image's last pixel has already been saved
                    ** immediately before entering this loop;
                    ** otherwise, the last pixel belongs to a pixel run and must be saved
                    */
                    if(pixelCount){
                        // put the uncompressible pixel in the destination buffer
                        imgDest[j++] = used_indexes[imgBuf[i++]];
                        ++pixelCount;
                    }
                    break;
                }

                if(imgBuf[i+1] == imgBuf[i])
                    ++identicalPixelsCount;
                else
                    identicalPixelsCount = 0;

                /* 3 subsequent identical pixels counted(possibly more after);
                ** we can break the run of uncompressible pixels without inflating the data
                */
                if(identicalPixelsCount == 2){
                    /* the last pixel which has been put in the current packet must be discarded, since it belongs
                    ** to the next RLE packet
                    */
                    --pixelCount;
                    --j;

                    /* we already counted 3 identical pixels; no need to count them again in the next
                    ** RLE counting loop
                    */
                    next_pixelCount = 2;
                    ++i;
                    break;
                }

                // put the uncompressible pixel in the destination buffer
                imgDest[j++] = used_indexes[imgBuf[i++]];
                ++pixelCount;

            }while(pixelCount < 127);

            // put pixelCount in the destination buffer for the current non-RLE packet
            imgDest[packet_pixelCountIdx] = pixelCount;

            // set the number of identical pixels counted for the next packet
            pixelCount = next_pixelCount;
        }
    }while(i < size);

    /* if the RLE compression resulted in increased size keep the data in its raw form
    ** (update the pixel indexes to the new shrunk palette first) */
    if(j >= size){
        for(i = 0; i < size; ++i)
            imgBuf[i] = used_indexes[imgBuf[i]];

        return -1;
    }

    return j;
}

/* local functions definitions */
static WORD shrink_palette(BYTE imgBuf[], DWORD size, BYTE used_indexes[], DWORD palette){
    unsigned int i, j;

    /* scan the whole image to track the palette colors actually used */
    for(i = 0; i < size; ++i)
        used_indexes[imgBuf[i]] = 1;

    /* remap the palette with the used palette colors placed sequentially */
    for(i = 0, j = 0; i < 256; ++i)
        if(used_indexes[i]){
            tga_shrunk_palette[j] = tga_palette[palette][i];
            used_indexes[i] = j++;
        }

    return j;
}

static DWORD findMaxOf3(DWORD x, DWORD y, DWORD z){
    DWORD max;

    if(x >= y)
        max = x;
    else
    max = y;

    if(z >= max)
        max = z;

    return max;
}




