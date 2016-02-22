/* lidquery.c:
 *
 * This program reads a .lid (LIght field Description) file,
 * and returns the requested attributes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lidquery.h"

/* Local helper routines */
int NextToken(FILE *file, char *buffer, int maxTokenSize);
int GotoSlab(FILE *file, int slabno);
int GotoToken(FILE *file, char *token);

/* Local macros */

/* Prints error and exits if result == null */
#define CHECKRESULTPTR(result)  if (((result) == NULL) && \
        fprintf(stderr, "Error:  *result passed NULL ptr!\n")) return 0

/* Opens file, prints error & exits if file is null */
#define SAFEOPEN(fileName, file) \
        if ((((file) = fopen((fileName), "r")) == NULL) && \
	fprintf(stderr, "Error:  Unable to open %s\n", fileName)) return 0

/* Count the number of slabs listed in the file */
int lidSLAB_COUNT  (int *result,  char *fileName)
{
    FILE *file;
    char token[400];
    
    CHECKRESULTPTR(result);
    SAFEOPEN(fileName, file);
    
    /* Count slabs */
    *result = 0;
    while (NextToken(file, token, 400)) {
        if (!strcmp(token, "SLAB")) {
            (*result)++;
        }
    }
    fclose(file);
    return(1);
}

/* Get the id number of the slabno'th slab in the file */
int lidSLAB_ID     (int *result,  char *fileName, int slabno)
{
    FILE *file;
    char token[400];
    
    CHECKRESULTPTR(result);
    SAFEOPEN(fileName, file);
    
    *result = 0;
    if (GotoSlab(file, slabno) && NextToken(file, token, 400)) {
        *result = atoi(token);
        fclose(file);
        return(1);
    }
    fclose(file);
    return(0);
}

/* Get the file basename string of the slabno'th slab in the file */
int lidSLAB_BASENAME(char *result, char *fileName, int slabno)
{
    FILE *file;
    char token[400];
    
    CHECKRESULTPTR(result);
    SAFEOPEN(fileName, file);
    
    *result = '\0';
    if (GotoSlab(file, slabno) && NextToken(file, token, 400) &&
        NextToken(file, token, 400)) {
        strcpy(result, token);
        fclose(file);
        return(1);
    }
    fclose(file);
    return(0);
}

/* Get the entire 4x6 (4x{x,y,z,w,s,t}) array of UV geometry */
int lidSLAB_UV     (float result[4][6], char *fileName, int slabno)
{
    FILE *file;
    char token[400];
    int vert, component;
    
    CHECKRESULTPTR(result);
    SAFEOPEN(fileName, file);
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SLAB_UV")) {
        for (vert = 0; vert < 4; vert++) {
            for (component = 0; component < 6; component++) {
                NextToken(file, token, 400);
                result[vert][component] = atof(token);
            }
        }
        fclose(file);
        return (1);
    }
    fclose(file);
    return(0);
}

/* Get the 6-components (4x{x,y,z,w,s,t}) of one vertex of UV geometry */
int lidSLAB_UV_V   (float result[6], char *fileName, int slabno,
                    int vertno)
{
    FILE *file;
    char token[400];
    int vert, component;
    
    CHECKRESULTPTR(result);
    SAFEOPEN(fileName, file);
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SLAB_UV")) {
        for (vert = 0; vert < 4; vert++) {
            for (component = 0; component < 6; component++) {
                NextToken(file, token, 400);
                if (vert == vertno) {
                    result[component] = atof(token);
                }
            }
        }
        fclose(file);
        return (1);
    }
    fclose(file);
    return(0);
}

/* Get one component {x,y,z,w,s, or t} of one vertex of UV geometry */
int lidSLAB_UV_V_C (float *result, char *fileName, int slabno, int vertno,
                    int componentno)
{
    FILE *file;
    char token[400];
    int vert, component;
    
    CHECKRESULTPTR(result);
    SAFEOPEN(fileName, file);
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SLAB_UV")) {
        for (vert = 0; vert < 4; vert++) {
            for (component = 0; component < 6; component++) {
                NextToken(file, token, 400);
                if (vert == vertno && component == componentno) {
                    *result = atof(token);
                }
            }
        }
        fclose(file);
        return (1);
    }
    fclose(file);
    return(0);
}

/* Get the entire 4x6 (4x{x,y,z,w,s,t}) array of ST geometry */
int lidSLAB_ST     (float result[4][6], char *fileName, int slabno)
{
    FILE *file;
    char token[400];
    int vert, component;
    
    CHECKRESULTPTR(result);
    SAFEOPEN(fileName, file);
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SLAB_ST")) {
        for (vert = 0; vert < 4; vert++) {
            for (component = 0; component < 6; component++) {
                NextToken(file, token, 400);
                result[vert][component] = atof(token);
            }
        }
        fclose(file);
        return (1);
    }
    fclose(file);
    return(0);
}

/* Get the 6-components (4x{x,y,z,w,s,t}) of one vertex of UV geometry */
int lidSLAB_ST_V   (float result[6], char *fileName, int slabno,
                    int vertno)
{
    FILE *file;
    char token[400];
    int vert, component;
    
    CHECKRESULTPTR(result);
    SAFEOPEN(fileName, file);
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SLAB_ST")) {
        for (vert = 0; vert < 4; vert++) {
            for (component = 0; component < 6; component++) {
                NextToken(file, token, 400);
                if (vert == vertno) {
                    result[component] = atof(token);
                }
            }
        }
        fclose(file);
        return (1);
    }
    fclose(file);
    return(0);
}

/* Get one component {x,y,z,w,s, or t} of one vertex of ST geometry */
int lidSLAB_ST_V_C (float *result, char *fileName, int slabno, int vertno,
                    int componentno)
{
    FILE *file;
    char token[400];
    int vert, component;
    
    CHECKRESULTPTR(result);
    SAFEOPEN(fileName, file);
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SLAB_ST")) {
        for (vert = 0; vert < 4; vert++) {
            for (component = 0; component < 6; component++) {
                NextToken(file, token, 400);
                if (vert == vertno && component == componentno) {
                    *result = atof(token);
                }
            }
        }
        fclose(file);
        return (1);
    }
    fclose(file);
    return(0);
}

/* Get the number of samples of U and V */
int lidSAMPLES_UV  (int result[2], char *fileName, int slabno)
{
    FILE *file;
    char token[400];
    int component;
    
    CHECKRESULTPTR(result);
    SAFEOPEN(fileName, file);
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SAMPLES_UV")) {
        for (component = 0; component < 2; component++) {
            NextToken(file, token, 400);
            result[component] = atoi(token);
        }
        fclose(file);
        return (1);
    }
    fclose(file);
    return(0);
}

/* Get the number of samples of U */
int lidSAMPLES_U   (int *result, char *fileName, int slabno)
{
    FILE *file;
    char token[400];
    
    CHECKRESULTPTR(result);
    SAFEOPEN(fileName, file);
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SAMPLES_UV")) {
        if (NextToken(file, token, 400)) {
            *result = atoi(token);
            fclose(file);
            return (1);
        }
    }
    fclose(file);
    return(0);
}

/* Get the number of samples of V */
int lidSAMPLES_V   (int *result, char *fileName, int slabno)
{
    FILE *file;
    char token[400];
    
    CHECKRESULTPTR(result);
    SAFEOPEN(fileName, file);
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SAMPLES_UV")) {
        if (NextToken(file, token, 400) && NextToken(file, token, 400)) {
            *result = atoi(token);
            fclose(file);
            return (1);
        }
    }
    fclose(file);
    return(0);
}

/* Get the number of samples of S and T */
int lidSAMPLES_ST  (int result[2], char *fileName, int slabno)
{
    FILE *file;
    char token[400];
    int component;
    
    CHECKRESULTPTR(result);
    SAFEOPEN(fileName, file);
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SAMPLES_ST")) {
        for (component = 0; component < 2; component++) {
            NextToken(file, token, 400);
            result[component] = atoi(token);
        }
        fclose(file);
        return (1);
    }
    fclose(file);
    return(0);
}

/* Get the number of samples of S */
int lidSAMPLES_S   (int *result, char *fileName, int slabno)
{
    FILE *file;
    char token[400];
    
    CHECKRESULTPTR(result);
    SAFEOPEN(fileName, file);
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SAMPLES_ST")) {
        if (NextToken(file, token, 400)) {
            *result = atoi(token);
            fclose(file);
            return (1);
        }
    }
    fclose(file);
    return(0);
}

/* Get the number of samples of T */
int lidSAMPLES_T   (int *result, char *fileName, int slabno)
{
    FILE *file;
    char token[400];
    
    CHECKRESULTPTR(result);
    SAFEOPEN(fileName, file);
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SAMPLES_ST")) {
        if (NextToken(file, token, 400) && NextToken(file, token, 400)) {
            *result = atoi(token);
            fclose(file);
            return (1);
        }
    }
    fclose(file);
    return(0);
}

int lidFORMAT      (char  *result, char *fileName, int slabno)
{
    FILE *file;
    char token[400];
    
    CHECKRESULTPTR(result);
    SAFEOPEN(fileName, file);
    
    if (GotoSlab(file, slabno) && GotoToken(file, "FORMAT")) {
        if (NextToken(file, token, 400)) {
            strcpy(result, token);
            fclose(file);
            return (1);
        }
    }
    fclose(file);
    return(0);
}


/* This routine skips to the slabno'th slab listing in the file.
 * The first slab is 0, second is 1, etc...
 * It returns 1 on success, and 0 on failure. 
 */
int GotoSlab(FILE *file, int slabno) {
    int currSlab = 0;
    char token[400];
    /* skip past slabno slabs */
    while (NextToken(file, token, 400)) {
        if (!strcmp(token, "SLAB")) {
            currSlab++;
            if (currSlab > slabno) return 1;
        }
    }
    
    /* If we get here, we hit EOF. */
    fprintf(stderr, "Error:  file only has %d slabs.\n", currSlab);
    return 0;
}
  
/* This routine reads until the first occurrence of the token.
 * It returns 1 on success, and 0 on failure. 
 */
int GotoToken(FILE *file, char *token) {
    char tok[400];
    
    /* Search for token */
    while (NextToken(file, tok, 400)) {
        if (!strcmp(token, tok)) return(1);
        /* If we hit start of next slab, stop looking */
        if (!strcmp(tok, "SLAB")) break;
    }
    /* if we get here, we didn't find the token */
    fprintf(stderr, "Error:  didn't find token %s in the slab.\n", token);
    return 0;
}
  
/* This routine returns 1 on success, 0 on failure (end-of-hdr).
 * It assumes that buffer has been allocated and is large enough
 * to hold a token of maxTokenSize-1 + null-terminator.
 */
int NextToken(FILE *file, char *buffer, int maxTokenSize)
{
    int index=0;		/* Index into the buffer */
    int c;
    /* State machine:
     * 0 = haven't seen start of token yet
     * 1 = reading token
     * 2 = token finished
     * 3 = inside a comment
     */
    int state = 0;
    
    while (state != 2) {
        c = fgetc(file);
        
        /* Handle EOF */
        if (c == EOF) {
            buffer[index] = '\0';
            /* Return 1 iff we've seen part of a token, 0 otherwise */
            if (state == 1) {
                return 1;
            } else {
                return 0;
            }
        }
        
        /* Handle comments */
        if (c == '#') {
            if (state == 1) {
                buffer[index] = '\0';
                /* Finish reading the comment, so we start on next line next time */
                while ((c != '\n') && (c != EOF)) {
                    c = fgetc(file);
                }
                /* now return token that we read. */
                return 1;
            } else {
                /* We're inside a comment now */
                state=3;
                continue;
            }
        }
        
        /* Handle end-of-comment? */
        if (c == '\n') {
            if (state == 1) {
                buffer[index] = '\0';
                return 1;
            } else {
                /* End of comment, back into whitespace */
                state=0;
                continue;
            }
        }
        
        /* Handle whitespace */
        if (isspace(c)) {
            if (state == 1) {
                buffer[index] = '\0';
                return 1;
            } else {
                /* do nothing -- go on to next character */
                continue;
            }
        }
        
        /* Default case */
        {
            if (state == 0) {
                state = 1;
            }
            if (state == 1 && index < maxTokenSize-1) {
                buffer[index++] = c;
            }
        }
    }
    return(0);
}
