#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "pic_utils.h"
#include "tga_utils.h"
#include "makedir.h"

extern FILE *in_fp;
extern char path[];
extern char *baseDirPtr;

/* local functions declarations */
static void init_path(const char *wadPath);




int main(int argc, char **argv){
    unsigned int choice = 10;

     puts("\t\t\tDisruptor ripper by Yagotzirck");

    if(argc != 2){
        fputs("Usage: disruptor_ripper.exe WAD.IN\n", stderr);
        return 1;
    }

    /* create main directory */
    init_path(argv[1]);

    if((in_fp = fopen(argv[1], "rb")) == NULL){
        fprintf(stderr, "Couldn't open %s\n", argv[1]);
        return 1;
    }

    do{
        puts("Select what you want to extract:\n"
             "0:\tHUD, weapons, psyonics\n"
             "1:\tEnemies' sprites(takes a while)\n"
             "2:\tSounds\n");

    scanf(" %d", &choice);
    while(getchar() != '\n')    /* clear stdin buffer */
        ;
    if(choice > 2)
        puts("Invalid choice");
    } while(choice > 2);


    switch(choice){
        case 0:
            ripPsyonics();
            ripWeapons();
            ripHUD();
            break;
        case 1:
            ripEnemySprites();
            break;
        case 2:
            ripSounds();
            break;
    }


    fclose(in_fp);

    return 0;
}


/* local functions definitions */
static void init_path(const char *wadPath){
    const char wadInString[] = "WAD.IN";
    strcpy(path, wadPath);

    /* Create a directory named "Disruptor resources" on same path as WAD.IN */

    baseDirPtr = path + strlen(path) - strlen(wadInString);

    if(strcmp(baseDirPtr, wadInString) != 0){
        fprintf(stderr, "%s isn't the expected Disruptor's data archive (%s)\n", wadPath, wadInString);
        exit(EXIT_FAILURE);
    }

    strcpy(baseDirPtr, "Disruptor resources/");
    makeDir(path);

    /* make baseDirPtr point to the end of "Disruptor resources/" string */
    baseDirPtr += strlen(baseDirPtr);
}
