/* lidquery.c:
 *
 * This program reads a .lid (LIght field Description) file,
 * and returns the requested attributes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lidquery.h"

/* Local functions */
void Usage(char *programName);

/* Helpful macros */
#define CHECKARGC(n) (argc == (n)) ? 0 : (Usage(argv[0]),1)

int main (int argc, char *argv[])
{
    char *fileName;
    int slabno;
    char *attrib;
    
    if (argc < 3) {
        Usage(argv[0]);
    }
    
    /* SLAB_COUNT */
    if (!strcmp(argv[2], "SLAB_COUNT")) {
        int slabct;
        if (lidSLAB_COUNT(&slabct, argv[1])) {
            printf("%d\n", slabct);
            exit(0);
        } else {
            exit(-1);
        }
    }
    
    /* All the rest of the attributes require a slabno, thus at least
     * 4 arguments.
     */
    if (argc < 4) {
        Usage(argv[0]);
    }
    fileName = argv[1];
    slabno = atoi(argv[2]);
    attrib =   argv[3];
    
    /* SLAB_ID */
    if (!strcmp(attrib, "SLAB_ID")) {
        int slabid = 0;
        CHECKARGC(4);
        if (lidSLAB_ID(&slabid, fileName, slabno)) {
            printf("%d\n", slabid);
            exit(0);
        } else {
            exit(-1);
        }
    }
    
    /* SLAB_BASENAME */
    if (!strcmp(attrib, "SLAB_BASENAME")) {
        char name[400];
        CHECKARGC(4);
        if (lidSLAB_BASENAME(name, fileName, slabno)) {
            printf("%s\n", name);
            exit(0);
        } else {
            exit(-1);
        }
    }
    
    /* SLAB_UV */
    if (!strcmp(attrib, "SLAB_UV")) {
        float uvgeom[4][6];
        int vert, comp;
        CHECKARGC(4);
        if (lidSLAB_UV(uvgeom, fileName, slabno)) {
            for (vert = 0; vert < 4; vert++) {
                for (comp = 0; comp < 6; comp++) {
                    printf("%f ", uvgeom[vert][comp]);
                }
                printf("\n");
            }
            exit(0);
        } else {
            exit(-1);
        }
    }
    
    /* SLAB_UV_V */
    if (!strcmp(attrib, "SLAB_UV_V")) {
        float uvgeom[6];
        int comp;
        CHECKARGC(5);
        if (lidSLAB_UV_V(uvgeom, fileName, slabno, atoi(argv[4]))) {
            for (comp = 0; comp < 6; comp++) {
                printf("%f ", uvgeom[comp]);
            }
            printf("\n");
            exit(0);
        } else {
            exit(-1);
        }
    }
    
    
    /* SLAB_UV_V_C */
    if (!strcmp(attrib, "SLAB_UV_V_C")) {
        float uvgeom;
        CHECKARGC(6);
        if (lidSLAB_UV_V_C(&uvgeom, fileName, slabno, atoi(argv[4]),
                           atoi(argv[5]))) {
            printf("%f\n", uvgeom);
            exit(0);
        } else {
            exit(-1);
        }
    }
    
    
    /* SLAB_ST */
    if (!strcmp(attrib, "SLAB_ST")) {
        float stgeom[4][6];
        int vert, comp;
        CHECKARGC(4);
        if (lidSLAB_ST(stgeom, fileName, slabno)) {
            for (vert = 0; vert < 4; vert++) {
                for (comp = 0; comp < 6; comp++) {
                    printf("%f ", stgeom[vert][comp]);
                }
                printf("\n");
            }
            exit(0);
        } else {
            exit(-1);
        }
    }
    
    /* SLAB_ST_V */
    if (!strcmp(attrib, "SLAB_ST_V")) {
        float stgeom[6];
        int comp;
        CHECKARGC(5);
        if (lidSLAB_ST_V(stgeom, fileName, slabno, atoi(argv[4]))) {
            for (comp = 0; comp < 6; comp++) {
                printf("%f ", stgeom[comp]);
            }
            printf("\n");
            exit(0);
        } else {
            exit(-1);
        }
    }
    
    
    /* SLAB_ST_V_C */
    if (!strcmp(attrib, "SLAB_ST_V_C")) {
        float stgeom;
        CHECKARGC(6);
        if (lidSLAB_ST_V_C(&stgeom, fileName, slabno, atoi(argv[4]),
                           atoi(argv[5]))) {
            printf("%f\n", stgeom);
            exit(0);
        } else {
            exit(-1);
        }
    }
    
    
    /* SAMPLES_UV */
    if (!strcmp(attrib, "SAMPLES_UV")) {
        int samples[2];
        int comp;
        CHECKARGC(4);
        if (lidSAMPLES_UV(samples, fileName, slabno)) {
            for (comp = 0; comp < 2; comp++) {
                printf("%d ", samples[comp]);
            }
            printf("\n");
            exit(0);
        } else {
            exit(-1);
        }
    }
    
    /* SAMPLES_U */
    if (!strcmp(attrib, "SAMPLES_U")) {
        int samples;
        CHECKARGC(4);
        if (lidSAMPLES_U(&samples, fileName, slabno)) {
            printf("%d\n", samples);
            exit(0);
        } else {
            exit(-1);
        }
    }
    
    /* SAMPLES_V */
    if (!strcmp(attrib, "SAMPLES_V")) {
        int samples;
        CHECKARGC(4);
        if (lidSAMPLES_V(&samples, fileName, slabno)) {
            printf("%d\n", samples);
            exit(0);
        } else {
            exit(-1);
        }
    }
    
    /* SAMPLES_ST */
    if (!strcmp(attrib, "SAMPLES_ST")) {
        int samples[2];
        int comp;
        CHECKARGC(4);
        if (lidSAMPLES_ST(samples, fileName, slabno)) {
            for (comp = 0; comp < 2; comp++) {
                printf("%d ", samples[comp]);
            }
            printf("\n");
            exit(0);
        } else {
            exit(-1);
        }
    }
    
    /* SAMPLES_S */
    if (!strcmp(attrib, "SAMPLES_S")) {
        int samples;
        CHECKARGC(4);
        if (lidSAMPLES_S(&samples, fileName, slabno)) {
            printf("%d\n", samples);
            exit(0);
        } else {
            exit(-1);
        }
    }
    
    /* SAMPLES_T */
    if (!strcmp(attrib, "SAMPLES_T")) {
        int samples;
        CHECKARGC(4);
        if (lidSAMPLES_T(&samples, fileName, slabno)) {
            printf("%d\n", samples);
            exit(0);
        } else {
            exit(-1);
        }
    }
    
    /* FORMAT */
    if (!strcmp(attrib, "FORMAT")) {
        char format[400];
        CHECKARGC(4);
        if (lidFORMAT(format, fileName, slabno)) {
            printf("%s\n", format);
            exit(0);
        } else {
            exit(-1);
        }
    }
    
    
    /* Default case */
    fprintf(stderr, "Error: Unrecognized attribute: %s\n", attrib);
    Usage(argv[0]);
    exit(-1);
}



void Usage(char *programName)
{
    fprintf(stderr, "\n"
            "Usage: %s <filename> <attribute>\n", programName);
    fprintf(stderr, 
            "where <attribute> is one of the following:\n"
            "	SLAB_COUNT\n"
            "	<slabno> SLAB_ID\n"
            "	<slabno> SLAB_BASENAME\n"
            "	<slabno> SLAB_UV\n"
            "	<slabno> SLAB_UV_V   <vertexno>\n"
            "	<slabno> SLAB_UV_V_C <vertexno> <componentno>\n"
            "	<slabno> SLAB_ST\n"
            "	<slabno> SLAB_ST_V   <vertexno>\n"
            "	<slabno> SLAB_ST_V_C <vertexno> <componentno>\n"
            "	<slabno> SAMPLES_UV\n"
            "	<slabno> SAMPLES_U\n"
            "	<slabno> SAMPLES_V\n"
            "	<slabno> SAMPLES_ST\n"
            "	<slabno> SAMPLES_S\n"
            "	<slabno> SAMPLES_T\n"
            "     <slabno> FORMAT\n");
    exit(-1);
}

