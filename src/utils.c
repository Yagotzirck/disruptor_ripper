#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "pic_utils.h"
#include "tga_utils.h"
#include "makedir.h"

#define NUMLEVELS 14
#define NUMSUBBLOCKS 16
#define MAXSPRITESPERBLOCK 8
#define LEVELBLOCKS_OFFSET 0x28C

#define LEVELBLOCKSIZE  0x74800
#define SPRITEBLOCKSIZE 0x7D800

#define SWAP_ENDIAN32(x) (((x)>>24) | (((x)>>8) & 0xFF00) | (((x)<<8) & 0x00FF0000) | ((x)<<24))
#define SNDBLOCK_SIZE_OFFSET    0x1B8
#define SND_DESCRIPTOR_SIZE     0x10

#define ENEMY_SPRITESHEET_OFFSET         0x110
#define ENEMY_SPRITESHEET_SIZE           0x6000
#define ENEMY_SPRITESHEET_WIDTH          128
#define ENEMY_SPRITESHEET_HEIGHT         192




#define WEAPBLOCKSOFFSET    8
#define NUMWEAPBLOCKS       80
#define REALNUMWEAPBLOCKS   10

#define WEAP_SPRITE_DESCR_STATIC_OFFSET     0x00
#define WEAP_SPRITE_DESCR_DYNAMIC_OFFSET    0x04

#define WEAP_STATIC_SPRITE_DATA_OFFSET      0x14
#define WEAP_DYNAMIC_SPRITE_DATA_OFFSET     0x18

#define WEAP_NUM_STATIC_SPRITES_OFFSET      0x24
#define WEAP_NUM_DYNAMIC_SPRITES_OFFSET     0x28


#define WEAP_PALETTE0_OFFSET            0x88
#define WEAP_PALETTE1_OFFSET            0x488

#define WEAP_SPRITESHEET_OFFSET         0x708   // size 0x9000; 384x96; could also be 0x688
#define WEAP_SPRITESHEET_SIZE           0x9000
#define WEAP_SPRITESHEET_WIDTH          384
#define WEAP_SPRITESHEET_HEIGHT         96



#define PSYONICS_DESCR_OFFSET   0x148
#define PSYONICS_NUM_BLOCKS     6
#define PSYONICS_BLOCKSIZE      0x11800

#define PSYONICS_PALETTE_OFFSET 0x64
//#define PSYONICS_DATA_OFFSET    0x864
#define PSYONICS_DATA_OFFSET    0x864

#define PSYONICS_SHEET_WIDTH    512
#define PSYONICS_SHEET_HEIGHT   64


#define HUD_SHEET_PALETTE_OFF   0x11A800
#define HUD_SHEET_NUMPALS       5
#define HUD_SHEET_OFFSET        0x11B400
#define HUD_SHEET_SIZE          0x6000
#define HUD_SHEET_WIDTH         256
#define HUD_SHEET_HEIGHT        96


#define SPEECH_DESCR_OFFSET     0x4FC
#define SPEECH_COUNT            15

#define MUSIC_DESCR_OFFSET      0x2E8
#define MUSIC_COUNT             16


#define WEAPSND_RELATIVE_OFFSET 0x9708
#define WEAPSND_BLOCK_SIZE      0x7D800


#define PSYONICSND_RELATIVE_OFFSET  0x14
#define PSYONICSND_BLOCK_SIZE       0x8000

/* global data */
FILE *in_fp;

char path[FILENAME_MAX];
char *baseDirPtr;
char *currDirPtr;


/* structures definitions */


typedef struct levelBlocks_s{
    DWORD offset;
    DWORD sndOffset, baseOffset;

    DWORD subBlocksOff[NUMSUBBLOCKS];
    DWORD spriteBlocksOffs[NUMSUBBLOCKS][MAXSPRITESPERBLOCK];
    DWORD spriteBlocksSizes[NUMSUBBLOCKS][MAXSPRITESPERBLOCK];
}levelBlocks_t;

typedef struct levelSndBlocks_s{
    DWORD offset;
    DWORD sndOffset, blockSize, numSounds;
}levelSndBlocks_t;


typedef struct spriteDescriptor_s{
        DWORD offset;	// relative to the beginning of the sprite block
        WORD  size;
        BYTE palette;   // either 0 or 1
        BYTE unk1;
        BYTE width;
        BYTE height;
        BYTE unk2;
        BYTE semiTrans_Type;
        //BYTE unk4;
        DWORD unk5;
        DWORD unk6;
}spriteDescriptor_t;

typedef struct spriteDescriptorDynamic_s{
        DWORD offset;	// relative to the beginning of the sprite block
        WORD  size;
        BYTE palette;   // either 0 or 1
        BYTE unk1;
        BYTE width;
        BYTE height;
        BYTE unk2;
        BYTE semiTrans_Type;
}spriteDescriptorDynamic_t;


/* static functions declarations */
static void init_levelBlocks(levelBlocks_t levelBlocks[]);
static void init_levelSndBlocks(levelSndBlocks_t levelSndBlocks[]);

/* functions definitions */
void ripEnemySprites(void){
    unsigned int i, j, k, l;
    unsigned int numSprites;

    //BYTE block[SPRITEBLOCKSIZE];
    BYTE *block; //debug; remove later
    //BYTE spriteData[128 * 256];
    BYTE *spriteData;  //debug; remove later
    BYTE *enemySpriteSheet;

    levelBlocks_t levelBlocks[NUMLEVELS];

    psx_palette_t *psxPalette;

    spriteDescriptor_t *spriteDescriptor;

    spriteData = malloc(128 * 256);

    init_levelBlocks(levelBlocks);

    block = malloc(0xFFFFFF);

    char *oldBaseDirPtr = baseDirPtr;
    strcpy(baseDirPtr, "Enemies/");
    makeDir(path);
    baseDirPtr = baseDirPtr + strlen(baseDirPtr);




    for(i = 0; i < NUMLEVELS; ++i){
        if(i == 11)
            continue;
        printf("Ripping level %u enemies...", i+1);
        for(j = 0; j < NUMSUBBLOCKS; ++j)
            if(levelBlocks[i].subBlocksOff[j] != levelBlocks[i].spriteBlocksOffs[j-1])
                for(k = 0; k < MAXSPRITESPERBLOCK; ++k)
                    if(levelBlocks[i].spriteBlocksOffs[j][k]){
                        //block = malloc(levelBlocks[i].spriteBlocksSizes[j][k]);

                        if(levelBlocks[i].spriteBlocksSizes[j][k] > 0xFFFFFF){
                            //printf("bad palette pointer = %08X\ni = %u, j = %u, k = %u\n", *(DWORD *)(block + 0x100), i, j, k);
                            continue;
                        }

                        fseek(in_fp, levelBlocks[i].spriteBlocksOffs[j][k], SEEK_SET);
                        //fread(block, 1, levelBlocks[i].spriteBlocksSizes[j][k], in_fp);
                        fread(block, 1, 0xFFFFFF, in_fp);

                        spriteDescriptor = block + *(DWORD *)block;

                        psxPalette = block + *(DWORD *)(block + 0x100);

                        numSprites = block[0x325];


                        if(*(DWORD *)(block + 0x100) > 0xFFFF00){
                            //printf("bad palette pointer = %08X\ni = %u, j = %u, k = %u\n", *(DWORD *)(block + 0x100), i, j, k);
                            continue;
                        }


                        psxToTgaPal(psxPalette);

                        sprintf(baseDirPtr, "Level %.2u/", i+1);
                        makeDir(path);
                        currDirPtr = baseDirPtr + strlen(baseDirPtr);






                        for(l = 0; l < numSprites; ++l){
                            if(spriteDescriptor[l].offset > 0xFFFF00)
                                break;
                            spriteDecode(block + spriteDescriptor[l].offset, spriteData, spriteDescriptor[l].width * spriteDescriptor[l].height);

                            sprintf(currDirPtr, "Block%.2u_Sprite%u_Frame%.3u.tga", j+1, k+1, l+1);
                            pic_handler(spriteData, spriteDescriptor[l].width, spriteDescriptor[l].height, spriteDescriptor[l].palette);
                        }



                        enemySpriteSheet = block + *(DWORD *)(block + ENEMY_SPRITESHEET_OFFSET);


                        for(l = 0; l < 2; ++l){
                            sprintf(currDirPtr, "_Block%.2u_Spritesheet_palette %u.tga", j+1, l+1);
                            pic_handler(enemySpriteSheet, ENEMY_SPRITESHEET_WIDTH, ENEMY_SPRITESHEET_HEIGHT, l);
                        }



                        if( /*(i == 6 && j == 1) || */ (i == 12 && j == 6) ){
                            //psxPalette = (enemySpriteSheet + (180 * 128));  // embedded palette offset
                            psxPalette = (enemySpriteSheet + 0x5A00);  // embedded palette offset
                            psxToTgaPal(psxPalette);
                            sprintf(currDirPtr, "_Block%.2u_Spritesheet_palette embedded.tga", j+1);

                            {
                                FILE *out2_fp = fopen(path, "wb");
                                fwrite(enemySpriteSheet, 1, ENEMY_SPRITESHEET_SIZE, out2_fp);
                                //exit(0);
                            }

                            pic_handler(enemySpriteSheet, ENEMY_SPRITESHEET_WIDTH, ENEMY_SPRITESHEET_HEIGHT, 0);
                        }


                }

        puts("done");
    }


    {
        psx_palette_t sheetPalette1[2][256];
        BYTE enemySpriteSheet1[ENEMY_SPRITESHEET_SIZE];

        /* rip a krueger sheet apparently unreachable in the normal loop */
        fseek(in_fp, 0x987CE10, SEEK_SET);
        fread(&sheetPalette1[0][0], 1, 0x400, in_fp);
        fread(enemySpriteSheet1, 1, ENEMY_SPRITESHEET_SIZE, in_fp);

        psxPalette = &sheetPalette1[0][0];
        psxToTgaPal(psxPalette);


        for(l = 0; l < 2; ++l){
            sprintf(currDirPtr, "_Block%.2u_Spritesheet1_palette %u.tga", j+1, l+1);
            pic_handler(enemySpriteSheet1, ENEMY_SPRITESHEET_WIDTH, ENEMY_SPRITESHEET_HEIGHT, l);
        }

        /* rip a sheet with embedded palette */
        fseek(in_fp, 0x4AD8870, SEEK_SET);
        fread(enemySpriteSheet1, 1, ENEMY_SPRITESHEET_SIZE, in_fp);

        psxPalette = (enemySpriteSheet1 + 0x5A00);  // embedded palette offset
        psxToTgaPal(psxPalette);



        strcpy(baseDirPtr, "Level 07/_Block02_Spritesheet_palette embedded.tga");
        pic_handler_embedded16Colors(enemySpriteSheet1, ENEMY_SPRITESHEET_WIDTH, ENEMY_SPRITESHEET_HEIGHT, 0);
    }

    baseDirPtr = oldBaseDirPtr;

    free(spriteData);
    free(block);
}

void ripWeapons(void){
    unsigned int i, j, k, l;
    unsigned int numSprites;

    char *baseDirPtrOld = baseDirPtr;

    //BYTE block[SPRITEBLOCKSIZE];
    BYTE *block; //debug; remove later
    //BYTE spriteData[128 * 256];
    BYTE *spriteData;  //debug; remove later

    DWORD weapBlocks[NUMWEAPBLOCKS];

    BYTE *spritesEncodedData;

    psx_palette_t *psxPalette;

    spriteDescriptor_t *spriteDescriptor;
    spriteDescriptorDynamic_t *spriteDescriptorDynamic;
    BYTE *spriteWeapSheet;

    spriteData = malloc(128 * 256);


    block = malloc(0xFFFFFF);

    fseek(in_fp, WEAPBLOCKSOFFSET, SEEK_SET);
    fread(weapBlocks, sizeof(weapBlocks[0]), NUMWEAPBLOCKS, in_fp);

    strcpy(baseDirPtr, "Weapons/");
    makeDir(path);
    baseDirPtr += strlen(baseDirPtr);

    spriteWeapSheet = block + WEAP_SPRITESHEET_OFFSET;

    //for(i = 0; i < NUMWEAPBLOCKS; ++i){
    for(i = 0; i < 10; ++i){    // all the blocks after the 10th one appear to be duplicates
        fseek(in_fp, weapBlocks[i], SEEK_SET);

        fread(block, 1, 0xFFFFFF, in_fp);

        numSprites = *(DWORD *)(block + WEAP_NUM_STATIC_SPRITES_OFFSET);

        spriteDescriptor = block + *(DWORD *)(block + WEAP_SPRITE_DESCR_STATIC_OFFSET) + 0x11000;

        spritesEncodedData = block + *(DWORD *)(block + WEAP_STATIC_SPRITE_DATA_OFFSET) + 0x11000;

        sprintf(baseDirPtr, "Weapon block %.2u/", i+1);
        makeDir(path);
        currDirPtr = baseDirPtr + strlen(baseDirPtr);

        psxPalette = block + WEAP_PALETTE1_OFFSET;
        psxToTgaPal(psxPalette);

        strcpy(currDirPtr, "_Weapon sheet palette 3.tga");
        pic_handler(spriteWeapSheet, WEAP_SPRITESHEET_WIDTH, WEAP_SPRITESHEET_HEIGHT, 0);

        psxPalette = block + WEAP_PALETTE0_OFFSET;

        //psxToTgaPal(&psxPalette[0][0]);
        psxToTgaPal(psxPalette);

        strcpy(currDirPtr, "_Weapon sheet palette 1.tga");
        pic_handler(spriteWeapSheet, WEAP_SPRITESHEET_WIDTH, WEAP_SPRITESHEET_HEIGHT, 0);

        strcpy(currDirPtr, "_Weapon sheet palette 2.tga");
        pic_handler(spriteWeapSheet, WEAP_SPRITESHEET_WIDTH, WEAP_SPRITESHEET_HEIGHT, 1);


        for(j = 0; j < numSprites; ++j){
            if(spriteDescriptor[j].offset > 0xFFFF00)
                break;
            spriteDecode(spritesEncodedData + spriteDescriptor[j].offset, spriteData, spriteDescriptor[j].unk1 * spriteDescriptor[j].width);

            sprintf(currDirPtr, "Static frame%.2u.tga", j+1);
            pic_handler(spriteData, spriteDescriptor[j].unk1, spriteDescriptor[j].width, spriteDescriptor[j].palette);

            // handling weapons with red variant
            if(i == 3 || i == 4){

                sprintf(currDirPtr, "Static frame red %.2u.tga", j+1);
                pic_handler(spriteData, spriteDescriptor[j].unk1, spriteDescriptor[j].width, 1);
            }
        }

        spriteDescriptorDynamic = block + *(DWORD *)(block + WEAP_SPRITE_DESCR_DYNAMIC_OFFSET) + 0x11000;

        spritesEncodedData = block + *(DWORD *)(block + WEAP_DYNAMIC_SPRITE_DATA_OFFSET) + 0x11000;

        numSprites = *(DWORD *)(block + WEAP_NUM_DYNAMIC_SPRITES_OFFSET);

        for(j = 0; j < numSprites; ++j){
            if(spriteDescriptorDynamic[j].offset > 0xFFFF00)
                break;
            spriteDecode(spritesEncodedData + spriteDescriptorDynamic[j].offset, spriteData, spriteDescriptorDynamic[j].unk1 * spriteDescriptorDynamic[j].width);

            sprintf(currDirPtr, "Dynamic frame%.2u.tga", j+1);
            pic_handler(spriteData, spriteDescriptorDynamic[j].unk1, spriteDescriptorDynamic[j].width, spriteDescriptorDynamic[j].palette);

            // handling weapons with red variant
            if(i == 3 || i == 4){
                sprintf(currDirPtr, "Dynamic frame red %.2u.tga", j+1);
                pic_handler(spriteData, spriteDescriptorDynamic[j].unk1, spriteDescriptorDynamic[j].width, 1);
            }
        }
    }



    /* handling sheets with embedded palettes */

    // zodiac
    fseek(in_fp, weapBlocks[8], SEEK_SET);
    fread(block, 1, 0xFFFFFF, in_fp);

    psxPalette = spriteWeapSheet + 384 * 84;
    psxToTgaPal(psxPalette);

    strcpy(baseDirPtr, "Weapon block 09/_Weapon sheet palette embedded.tga");
    pic_handler_embedded16Colors(spriteWeapSheet, WEAP_SPRITESHEET_WIDTH, WEAP_SPRITESHEET_HEIGHT, 0);

    // phase rifle
    {
        psx_palette_t psxPalette[4][256];   // actually they're 16 entries, but the palette conversion function is designed for 256 so whatevs
        fseek(in_fp, weapBlocks[3], SEEK_SET);
        fread(block, 1, 0xFFFFFF, in_fp);

        // acquire the 4 sub-palettes; the phase repeater has the same palettes so we can acquire them only once for both weapons
        memcpy(&psxPalette[0][0], spriteWeapSheet + 384 * 48 + 256, 32);
        memcpy(&psxPalette[1][0], spriteWeapSheet + 384 * 60 + 256, 32);
        memcpy(&psxPalette[2][0], spriteWeapSheet + 384 * 72 + 256, 32);
        memcpy(&psxPalette[3][0], spriteWeapSheet + 384 * 84 + 256, 32);

        for(i = 0; i < 4; i += 2){
            psxToTgaPal(&psxPalette[i][0]);
            sprintf(baseDirPtr, "Weapon block 04/_Weapon sheet palette embedded %u.tga", i + 1);
            pic_handler_embedded16Colors(spriteWeapSheet, WEAP_SPRITESHEET_WIDTH, WEAP_SPRITESHEET_HEIGHT, 0);

            sprintf(baseDirPtr, "Weapon block 04/_Weapon sheet palette embedded %u.tga", i + 2);
            pic_handler_embedded16Colors(spriteWeapSheet, WEAP_SPRITESHEET_WIDTH, WEAP_SPRITESHEET_HEIGHT, 1);
    }

        // phase repeater
        fseek(in_fp, weapBlocks[4], SEEK_SET);
        fread(block, 1, 0xFFFFFF, in_fp);

        /*
        memcpy(&psxPalette[0][0], spriteWeapSheet + 384 * 48 + 256, 32);
        memcpy(&psxPalette[1][0], spriteWeapSheet + 384 * 60 + 256, 32);
        memcpy(&psxPalette[2][0], spriteWeapSheet + 384 * 72 + 256, 32);
        memcpy(&psxPalette[3][0], spriteWeapSheet + 384 * 84 + 256, 32);
        */

        for(i = 0; i < 4; i += 2){
            psxToTgaPal(&psxPalette[i][0]);
            sprintf(baseDirPtr, "Weapon block 05/_Weapon sheet palette embedded %u.tga", i + 1);
            pic_handler_embedded16Colors(spriteWeapSheet, WEAP_SPRITESHEET_WIDTH, WEAP_SPRITESHEET_HEIGHT, 0);

            sprintf(baseDirPtr, "Weapon block 05/_Weapon sheet palette embedded %u.tga", i + 2);
            pic_handler_embedded16Colors(spriteWeapSheet, WEAP_SPRITESHEET_WIDTH, WEAP_SPRITESHEET_HEIGHT, 1);
        }
    }


    baseDirPtr = baseDirPtrOld; // restore baseDirPtr

    free(block);
    free(spriteData);
}

void ripPsyonics(void){
    DWORD blocksOffsets[PSYONICS_NUM_BLOCKS];

    BYTE block[PSYONICS_BLOCKSIZE];

    BYTE *imgData = block + PSYONICS_DATA_OFFSET;

    psx_palette_t *psxPalette = block + PSYONICS_PALETTE_OFFSET;


    unsigned int i;

    fseek(in_fp, PSYONICS_DESCR_OFFSET, SEEK_SET);
    fread(blocksOffsets, sizeof(blocksOffsets[0]), PSYONICS_NUM_BLOCKS, in_fp);

    strcpy(baseDirPtr, "Psyonics/");
    makeDir(path);
    currDirPtr = baseDirPtr + strlen(baseDirPtr);

    for(i = 0; i < PSYONICS_NUM_BLOCKS; ++i){
        fseek(in_fp, blocksOffsets[i], SEEK_SET);
        fread(block, 1, PSYONICS_BLOCKSIZE, in_fp);

        psxToTgaPal(psxPalette);

        sprintf(currDirPtr, "Psyonic sheet %u - palette 1.tga", i+1);
        pic_handler(imgData, PSYONICS_SHEET_WIDTH, PSYONICS_SHEET_HEIGHT, 0);

        sprintf(currDirPtr, "Psyonic sheet %u - palette 2.tga", i+1);
        pic_handler(imgData, PSYONICS_SHEET_WIDTH, PSYONICS_SHEET_HEIGHT, 1);
    }
}



void ripHUD(void){
    BYTE hudData[HUD_SHEET_SIZE];
    psx_palette_t hudPalette[6][256];
    psx_palette_t *currPair;

    unsigned int i;

    fseek(in_fp, HUD_SHEET_PALETTE_OFF, SEEK_SET);
    fread(hudPalette, 1, sizeof(hudPalette), in_fp);

    fread(hudData, 1, HUD_SHEET_SIZE, in_fp);

    strcpy(baseDirPtr, "HUD/");
    makeDir(path);
    currDirPtr = baseDirPtr + strlen(baseDirPtr);

    for(i = 0; i < HUD_SHEET_NUMPALS -1; i+= 2){
        currPair = &hudPalette[i][0];
        psxToTgaPal(currPair);

        sprintf(currDirPtr, "HUD - palette %u.tga", i + 1);
        pic_handler(hudData, HUD_SHEET_WIDTH, HUD_SHEET_HEIGHT, 0);

        sprintf(currDirPtr, "HUD - palette %u.tga", i + 2);
        pic_handler(hudData, HUD_SHEET_WIDTH, HUD_SHEET_HEIGHT, 1);
    }

        //extract the 5th and last variant
        currPair = &hudPalette[i][0];
        psxToTgaPal(currPair);

        sprintf(currDirPtr, "HUD - palette %u.tga", i + 1);
        pic_handler(hudData, HUD_SHEET_WIDTH, HUD_SHEET_HEIGHT, 0);

}


void ripSounds(void){
    typedef struct sndDescr_s{
        DWORD offset;
        DWORD unk1;
        DWORD sampleRate;   // appears to be 0x400 when the frequency is 11025 Hz, 0x800 when it's 22050 Hz
        DWORD unk2;
    }sndDescr_t;

    typedef struct speechDescr_s{
        DWORD offset;
        DWORD size;
    }speechDescr_t;

    typedef struct musicDescr_s{
        DWORD offset;
        DWORD unk[7];
    }musicDescr_t;


    typedef struct VAGhdr_s{        // All the values in this header must be big endian
        char id[4];                   // VAGp
        DWORD version;              // I guess it doesn't matter, I'll use a random number(0)
        DWORD reserved;             // I guess it doesn't matter either
        DWORD dataSize;
        DWORD samplingFrequency;
        char  reserved2[12];
        char  name[16];
    }VAGhdr_t;

    sndDescr_t *sndDescr;
    BYTE *dataBlock;

    VAGhdr_t VAGhdr = {
        {'V', 'A', 'G', 'p'},
        0,
        0,
        0,              // will be set for each entry later on
        0,              // as above
        "Yagotzirck",   // Couldn't resist :)
        ""              // will be set for each entry later on
    };

    levelSndBlocks_t levelSndBlocks[NUMLEVELS];

    speechDescr_t speechDescr[SPEECH_COUNT];

    musicDescr_t musicDescr[MUSIC_COUNT];

    char *baseDirPtrOld = baseDirPtr;
    char *currSubDir;

    unsigned int i, j, sndCount;
    int sndSize;
    FILE *out_fp;



    strcpy(baseDirPtr, "Sounds/");
    makeDir(path);
    baseDirPtr += strlen(baseDirPtr);


    /************* rip ingame sounds ******************/
    strcpy(baseDirPtr, "Ingame sounds/");
    makeDir(path);
    currSubDir = baseDirPtr + strlen(baseDirPtr);

    init_levelSndBlocks(levelSndBlocks);

    for(i = 0; i < NUMLEVELS; ++i){
        if(i == 11)
            continue;

        if((sndDescr = malloc(levelSndBlocks[i].numSounds * SND_DESCRIPTOR_SIZE)) == NULL){
            fprintf(stderr, "Couldn't allocate %u bytes for level %u's sound descriptor array\n", levelSndBlocks[i].numSounds * SND_DESCRIPTOR_SIZE, i);
            exit(EXIT_FAILURE);
        }

        if((dataBlock = malloc(levelSndBlocks[i].blockSize)) == NULL){
            fprintf(stderr, "Couldn't allocate %u bytes for level %u's data block\n", levelSndBlocks[i].blockSize, i);
            exit(EXIT_FAILURE);
        }

        sprintf(currSubDir, "Level %.2u/", i + 1);
        makeDir(path);
        currDirPtr = currSubDir + strlen(currSubDir);




        fseek(in_fp, levelSndBlocks[i].sndOffset - (levelSndBlocks[i].numSounds * SND_DESCRIPTOR_SIZE), SEEK_SET);

        fread(sndDescr, SND_DESCRIPTOR_SIZE, levelSndBlocks[i].numSounds, in_fp);   // read the sound descriptor
        fread(dataBlock, 1, levelSndBlocks[i].blockSize, in_fp);                    // read the sound data block

        for(j = 0, sndCount = 1; j < levelSndBlocks[i].numSounds - 1; ++j){
            sndSize = sndDescr[j+1].offset - sndDescr[j].offset;

            if(sndSize == 0)
                continue;

            sprintf(currDirPtr, "%.2u.vag", sndCount);
            if((out_fp = fopen(path, "wb")) == NULL){
                fprintf(stderr, "Couldn't create %s\n", path);
                exit(EXIT_FAILURE);
            }

            VAGhdr.samplingFrequency = (sndDescr[j].sampleRate / 0x400) * 11025;
            VAGhdr.samplingFrequency = SWAP_ENDIAN32(VAGhdr.samplingFrequency);

            VAGhdr.dataSize = SWAP_ENDIAN32(sndSize);
            sprintf(VAGhdr.name, "Level%.2u snd%.2u", i+1, sndCount);

            // write VAG header to file
            fwrite(&VAGhdr, sizeof(VAGhdr), 1, out_fp);

            // write sound data to file
            fwrite(&dataBlock[sndDescr[j].offset], 1, sndSize, out_fp);

            fclose(out_fp);
            ++sndCount;
        }

        // handle the last sound
        sndSize = levelSndBlocks[i].blockSize - sndDescr[j].offset;

        sprintf(currDirPtr, "%.2u.vag", sndCount);
            if((out_fp = fopen(path, "wb")) == NULL){
                fprintf(stderr, "Couldn't create %s\n", path);
                exit(EXIT_FAILURE);
            }

            VAGhdr.samplingFrequency = (sndDescr[j].sampleRate / 0x400) * 11025;
            VAGhdr.samplingFrequency = SWAP_ENDIAN32(VAGhdr.samplingFrequency);

            VAGhdr.dataSize = SWAP_ENDIAN32(sndSize);
            sprintf(VAGhdr.name, "Level%.2u snd%.2u", i+1, sndCount);

            // write VAG header to file
            fwrite(&VAGhdr, sizeof(VAGhdr), 1, out_fp);

            // write sound data to file
            fwrite(&dataBlock[sndDescr[j].offset], 1, sndSize, out_fp);

            fclose(out_fp);

            // free the current level buffers
            free(sndDescr);
            free(dataBlock);
    }


    /************* rip loading screen speeches ******************/
    strcpy(baseDirPtr, "Loading screen speeches/");
    makeDir(path);
    currDirPtr = baseDirPtr + strlen(baseDirPtr);

    fseek(in_fp, SPEECH_DESCR_OFFSET, SEEK_SET);
    fread(speechDescr, sizeof(speechDescr[0]), SPEECH_COUNT, in_fp);

    VAGhdr.samplingFrequency = 22050;
    VAGhdr.samplingFrequency = SWAP_ENDIAN32(VAGhdr.samplingFrequency);

    for(i = 0, sndCount = 1; i < SPEECH_COUNT; ++i){
        if(speechDescr[i].offset == 0)
            continue;

        if((dataBlock = malloc(speechDescr[i].size)) == NULL){
            fprintf(stderr, "Couldn't allocate %u bytes speech %u's data block\n", speechDescr[i].size, i);
            exit(EXIT_FAILURE);
        }

        sprintf(currDirPtr, "Speech %.2u.vag", sndCount);
            if((out_fp = fopen(path, "wb")) == NULL){
                fprintf(stderr, "Couldn't create %s\n", path);
                exit(EXIT_FAILURE);
            }



        fseek(in_fp, speechDescr[i].offset, SEEK_SET);
        fread(dataBlock, 1, speechDescr[i].size, in_fp);

        VAGhdr.dataSize = SWAP_ENDIAN32(speechDescr[i].size);
        sprintf(VAGhdr.name, "Speech %.2u", sndCount);

        // write VAG header to file
        fwrite(&VAGhdr, sizeof(VAGhdr), 1, out_fp);

        // write sound data to file
        fwrite(dataBlock, 1, speechDescr[i].size, out_fp);

        fclose(out_fp);
        free(dataBlock);
        ++sndCount;
    }

    /************* rip weapon sounds ******************/
    {
    DWORD weapBlocks[REALNUMWEAPBLOCKS];
    BYTE *sndBegin, *sndEnd;

    strcpy(baseDirPtr, "Weapon sounds/");
    makeDir(path);
    currDirPtr = baseDirPtr + strlen(baseDirPtr);



    dataBlock = malloc(WEAPSND_BLOCK_SIZE);

    fseek(in_fp, WEAPBLOCKSOFFSET, SEEK_SET);
    fread(weapBlocks, sizeof(weapBlocks[0]), REALNUMWEAPBLOCKS, in_fp);



    for(i = 0; i < REALNUMWEAPBLOCKS; ++i){
        fseek(in_fp, weapBlocks[i] + WEAPSND_RELATIVE_OFFSET, SEEK_SET);
        fread(dataBlock, 1, WEAPSND_BLOCK_SIZE, in_fp);

        sndCount = 1;

        sndBegin = sndEnd = dataBlock;
        //while(sndEnd < &dataBlock[WEAPSND_BLOCK_SIZE]){
        while(sndEnd[16] != 0){
            while(*(DWORD *)sndEnd != 0x77770700)
                sndEnd += 4;

            sndEnd += 16;

            sndSize = sndEnd - sndBegin;

            sprintf(currDirPtr, "Weap%.2u snd%u.vag", i + 1, sndCount);
            if((out_fp = fopen(path, "wb")) == NULL){
                fprintf(stderr, "Couldn't create %s\n", path);
                exit(EXIT_FAILURE);
            }

            VAGhdr.dataSize = SWAP_ENDIAN32(sndSize);
            sprintf(VAGhdr.name, "Weap%.2u snd%u", i + 1, sndCount);

            if((i == 6 && sndCount >= 3) || (i == 5 && sndCount == 2) || i == 9)
                VAGhdr.samplingFrequency = 11025;
            else
                VAGhdr.samplingFrequency = 22050;

            VAGhdr.samplingFrequency = SWAP_ENDIAN32(VAGhdr.samplingFrequency);

            // write VAG header to file
            fwrite(&VAGhdr, sizeof(VAGhdr), 1, out_fp);

            // write sound data to file
            fwrite(sndBegin, 1, sndSize, out_fp);

            fclose(out_fp);
            ++sndCount;
            sndBegin = sndEnd;
        }
    }

    free(dataBlock);
    }

        /************* rip psyonics sounds ******************/

    VAGhdr.samplingFrequency = 22050;
    VAGhdr.samplingFrequency = SWAP_ENDIAN32(VAGhdr.samplingFrequency);

    {
    DWORD psyonicsBlocks[PSYONICS_NUM_BLOCKS];
    BYTE *sndBegin, *sndEnd;
    DWORD sndRelativeOffset;

    strcpy(baseDirPtr, "Psyonics sounds/");
    makeDir(path);
    currDirPtr = baseDirPtr + strlen(baseDirPtr);



    dataBlock = malloc(PSYONICSND_BLOCK_SIZE);

    fseek(in_fp, PSYONICS_DESCR_OFFSET, SEEK_SET);
    fread(psyonicsBlocks, sizeof(psyonicsBlocks[0]), PSYONICS_NUM_BLOCKS, in_fp);


    for(i = 0; i < PSYONICS_NUM_BLOCKS; ++i){
        // re-arrange the offsets to point to sounds
        fseek(in_fp, psyonicsBlocks[i] + PSYONICSND_RELATIVE_OFFSET, SEEK_SET);
        fread(&sndRelativeOffset, sizeof(sndRelativeOffset), 1, in_fp);

        psyonicsBlocks[i] += sndRelativeOffset + 0x1000;

        fseek(in_fp, psyonicsBlocks[i], SEEK_SET);
        fread(dataBlock, 1, PSYONICSND_BLOCK_SIZE, in_fp);

        sndCount = 1;

        sndBegin = sndEnd = dataBlock;
        //while(sndEnd < &dataBlock[PSYONICSND_BLOCK_SIZE]){
        while(sndEnd[16] != 0){
            while(*(DWORD *)sndEnd != 0x77770700)
                sndEnd += 4;

            sndEnd += 16;

            sndSize = sndEnd - sndBegin;

            sprintf(currDirPtr, "Psyonic%u snd%u.vag", i + 1, sndCount);
            if((out_fp = fopen(path, "wb")) == NULL){
                fprintf(stderr, "Couldn't create %s\n", path);
                exit(EXIT_FAILURE);
            }

            VAGhdr.dataSize = SWAP_ENDIAN32(sndSize);
            sprintf(VAGhdr.name, "Psyonic%u snd%u", i + 1, sndCount);

            /*
            if((i == 6 && sndCount >= 3) || (i == 5 && sndCount == 2))
                VAGhdr.samplingFrequency = 11025;
            else
                VAGhdr.samplingFrequency = 22050;

            VAGhdr.samplingFrequency = SWAP_ENDIAN32(VAGhdr.samplingFrequency);
            */



            // write VAG header to file
            fwrite(&VAGhdr, sizeof(VAGhdr), 1, out_fp);

            // write sound data to file
            fwrite(sndBegin, 1, sndSize, out_fp);

            fclose(out_fp);
            ++sndCount;
            sndBegin = sndEnd;
        }
    }

    free(dataBlock);
    }



#if 0   // extracted music is buggy as fuck; might look into it sometime in the future
    /************* rip music ******************/
    strcpy(baseDirPtr, "Music/");
    makeDir(path);
    currDirPtr = baseDirPtr + strlen(baseDirPtr);

    fseek(in_fp, MUSIC_DESCR_OFFSET, SEEK_SET);
    fread(musicDescr, sizeof(speechDescr[0]), MUSIC_COUNT, in_fp);

    for(i = 0, sndCount = 1; i < MUSIC_COUNT; ++i){
        if(speechDescr[i].offset == 0)
            continue;

        if(i < MUSIC_COUNT - 1)
            sndSize = musicDescr[i+1].offset - musicDescr[i].offset;
        else
            sndSize = 0x4EB000;

        if(sndSize < 0){
            j = i + 2;
            do
                sndSize = musicDescr[j++].offset - musicDescr[i].offset;
            while(sndSize < 0);
        }



        if((dataBlock = malloc(sndSize)) == NULL){
            fprintf(stderr, "Couldn't allocate %u bytes speech %u's data block\n", speechDescr[i].size, i);
            exit(EXIT_FAILURE);
        }

        sprintf(currDirPtr, "Music %.2u.vag", sndCount);
            if((out_fp = fopen(path, "wb")) == NULL){
                fprintf(stderr, "Couldn't create %s\n", path);
                exit(EXIT_FAILURE);
            }



        fseek(in_fp, musicDescr[i].offset, SEEK_SET);
        fread(dataBlock, 1, sndSize, in_fp);

        VAGhdr.dataSize = SWAP_ENDIAN32(sndSize);
        sprintf(VAGhdr.name, "Music %.2u", sndCount);

        // write VAG header to file
        fwrite(&VAGhdr, sizeof(VAGhdr), 1, out_fp);

        // write sound data to file
        fwrite(dataBlock, 1, sndSize, out_fp);

        fclose(out_fp);
        free(dataBlock);
        ++sndCount;
    }

#endif

    // restore the base directory pointer
    baseDirPtr = baseDirPtrOld;
}




/* static functions definitions */
static void init_levelBlocks(levelBlocks_t levelBlocks[]){
    unsigned int i, j, k;

    fseek(in_fp, LEVELBLOCKS_OFFSET, SEEK_SET);

    for(i = 0; i < 9; ++i)
        fread(&levelBlocks[i].offset, sizeof(levelBlocks[0].offset), 1, in_fp);

    //skip a zero-entry
    fseek(in_fp, 4, SEEK_CUR);

    for(; i < NUMLEVELS; ++i)
        fread(&levelBlocks[i].offset, sizeof(levelBlocks[0].offset), 1, in_fp);

    //initialize sub-blocks
    for(i = 0; i < NUMLEVELS; ++i){
        fseek(in_fp, levelBlocks[i].offset, SEEK_SET);

        fread(&levelBlocks[i].sndOffset, sizeof(levelBlocks[0].sndOffset), 1, in_fp);
        fread(&levelBlocks[i].baseOffset, sizeof(levelBlocks[0].baseOffset), 1, in_fp);
        fread(levelBlocks[i].subBlocksOff, sizeof(levelBlocks[0].subBlocksOff[0]), NUMSUBBLOCKS, in_fp);

        for(j = 0; j < NUMSUBBLOCKS; ++j){
            levelBlocks[i].subBlocksOff[j] += levelBlocks[i].offset + levelBlocks[i].baseOffset + LEVELBLOCKSIZE;

            fseek(in_fp, levelBlocks[i].subBlocksOff[j], SEEK_SET);
            fread(&levelBlocks[i].spriteBlocksOffs[j][0], sizeof(levelBlocks[0].spriteBlocksOffs[0][0]), MAXSPRITESPERBLOCK, in_fp);
            fread(&levelBlocks[i].spriteBlocksSizes[j][0], sizeof(levelBlocks[0].spriteBlocksSizes[0][0]), MAXSPRITESPERBLOCK, in_fp);


            for(k = 0; k < MAXSPRITESPERBLOCK; ++k)
                if(levelBlocks[i].spriteBlocksOffs[j][k])
                    levelBlocks[i].spriteBlocksOffs[j][k] = levelBlocks[i].spriteBlocksOffs[j][k] * 2048 + levelBlocks[i].subBlocksOff[j];
        }


    }
}


static void init_levelSndBlocks(levelSndBlocks_t levelSndBlocks[]){
    unsigned int i;
    DWORD sndOffsetCheck;

    fseek(in_fp, LEVELBLOCKS_OFFSET, SEEK_SET);

    for(i = 0; i < 9; ++i)
        fread(&levelSndBlocks[i].offset, sizeof(levelSndBlocks[0].offset), 1, in_fp);

    //skip a zero-entry
    fseek(in_fp, 4, SEEK_CUR);

    for(; i < NUMLEVELS; ++i)
        fread(&levelSndBlocks[i].offset, sizeof(levelSndBlocks[0].offset), 1, in_fp);

    //initialize sub-blocks
    for(i = 0; i < NUMLEVELS; ++i){
        fseek(in_fp, levelSndBlocks[i].offset + SNDBLOCK_SIZE_OFFSET, SEEK_SET);
        fread(&levelSndBlocks[i].blockSize, 4, 1, in_fp);

        fseek(in_fp, levelSndBlocks[i].offset, SEEK_SET);
        fread(&levelSndBlocks[i].sndOffset, sizeof(levelSndBlocks[0].sndOffset), 1, in_fp);
        levelSndBlocks[i].sndOffset += levelSndBlocks[i].offset + LEVELBLOCKSIZE;

        fseek(in_fp, levelSndBlocks[i].sndOffset - SND_DESCRIPTOR_SIZE, SEEK_SET);
        levelSndBlocks[i].numSounds = 1;

        while(fread(&sndOffsetCheck, 4, 1, in_fp), sndOffsetCheck != 0){
            ++levelSndBlocks[i].numSounds;
            fseek(in_fp, -(SND_DESCRIPTOR_SIZE + 4), SEEK_CUR);
        }
    }
}



