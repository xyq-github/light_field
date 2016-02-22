/* lidquery.cpp:
 *
 * This program reads a .lid (LIght field Description) file,
 * and returns the requested attributes.
 */

#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include "lidquery.h"

using namespace std;

/* Local helper routines */
bool NextToken(ifstream &file, string &buffer);
bool GotoSlab(ifstream &file, int slabno);
bool GotoToken(ifstream &file, string token);

/* Local macros */

/* Prints error and exits if result == null */
//#define CHECKRESULTPTR(result)  if (((result) == NULL) && \
//fprintf(stderr, "Error:  *result passed NULL ptr!\n")) return false
bool CHECKRESULTPTR(void *result)
{
    if (result == nullptr) {
        cerr << "Error: *result passed NULL ptr!\n";
        return false;
    }
    return true;
}

/* Opens file, prints error & exits if file is null */
//#define SAFEOPEN(fileName, file) \
//if ((((file) = fopen((fileName.c_str()), "r")) == NULL) && \
//fprintf(stderr, "Error:  Unable to open %s\n", fileName.c_str())) return false
//bool SAFEOPEN(ifstream &file, string filename)
//{
//    file.open(filename);
//    return file.is_open();
//}

/* Count the number of slabs listed in the file */
bool lidSLAB_COUNT  (int &result, string fileName)
{
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Error: unable to open " << fileName << endl;
        exit(12);
    }
    
    string token;
    result = 0;
    /* Count slabs */
    while (NextToken(file, token)) {
        if (token == "SLAB") {
            result++;
        }
        token.clear();
    }
    file.close();
    return true;
}

/* Get the id number of the slabno'th slab in the file */
bool lidSLAB_ID (int &result,  string fileName, int slabno)
{
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Error: unable to open " << fileName << endl;
        exit(12);
    }
    
    string token;
    result = 0;
    if (GotoSlab(file, slabno) && NextToken(file, token)) {
        result = stoi(token);
        file.close();
        return true;
    }
    file.close();
    return false;
}

/* Get the file basename string of the slabno'th slab in the file */
bool lidSLAB_BASENAME(string &result, string fileName, int slabno)
{
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Error: unable to open " << fileName << endl;
        exit(12);
    }
    string token;
    
    if (GotoSlab(file, slabno) && NextToken(file, token) &&
        NextToken(file, token)) {
        result = token;
        file.close();
        return true;
    }
    file.close();
    return false;
}

/* Get the entire 4x6 (4x{x,y,z,w,s,t}) array of UV geometry */
bool lidSLAB_UV     (float result[4][6], string fileName, int slabno)
{
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Error: unable to open " << fileName << endl;
        exit(12);
    }
    string token;
    int vert, component;
    
    if (!CHECKRESULTPTR(result))
        return false;
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SLAB_UV")) {
        for (vert = 0; vert < 4; vert++) {
            for (component = 0; component < 6; component++) {
                NextToken(file, token);
                result[vert][component] = stof(token);
            }
        }
        file.close();
        return true;
    }
    file.close();
    return false;
}

/* Get the 6-components (4x{x,y,z,w,s,t}) of one vertex of UV geometry */
bool lidSLAB_UV_V   (float result[6], string fileName, int slabno,
                    int vertno)
{
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Error: unable to open " << fileName << endl;
        exit(12);
    }
    string token;
    int vert, component;
    
    CHECKRESULTPTR(result);
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SLAB_UV")) {
        for (vert = 0; vert < 4; vert++) {
            for (component = 0; component < 6; component++) {
                NextToken(file, token);
                if (vert == vertno) {
                    result[component] = stof(token);
                }
            }
        }
        file.close();
        return true;
    }
    file.close();
    return false;
}

/* Get one component {x,y,z,w,s, or t} of one vertex of UV geometry */
bool lidSLAB_UV_V_C (float &result, string fileName, int slabno, int vertno,
                    int componentno)
{
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Error: unable to open " << fileName << endl;
        exit(12);
    }
    string token;
    int vert, component;
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SLAB_UV")) {
        for (vert = 0; vert < 4; vert++) {
            for (component = 0; component < 6; component++) {
                NextToken(file, token);
                if (vert == vertno && component == componentno) {
                    result = stof(token);
                }
            }
        }
        file.close();
        return true;
    }
    file.close();
    return false;
}

/* Get the entire 4x6 (4x{x,y,z,w,s,t}) array of ST geometry */
bool lidSLAB_ST     (float result[4][6], string fileName, int slabno)
{
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Error: unable to open " << fileName << endl;
        exit(12);
    }
    string token;
    int vert, component;
    
    CHECKRESULTPTR(result);
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SLAB_ST")) {
        for (vert = 0; vert < 4; vert++) {
            for (component = 0; component < 6; component++) {
                NextToken(file, token);
                result[vert][component] = stof(token);
            }
        }
        file.close();
        return true;
    }
    file.close();
    return false;
}

/* Get the 6-components (4x{x,y,z,w,s,t}) of one vertex of UV geometry */
bool lidSLAB_ST_V   (float result[6], string fileName, int slabno,
                    int vertno)
{
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Error: unable to open " << fileName << endl;
        exit(12);
    }
    string token;
    int vert, component;
    
    CHECKRESULTPTR(result);
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SLAB_ST")) {
        for (vert = 0; vert < 4; vert++) {
            for (component = 0; component < 6; component++) {
                NextToken(file, token);
                if (vert == vertno) {
                    result[component] = stof(token);
                }
            }
        }
        file.close();
        return true;
    }
    file.close();
    return false;
}

/* Get one component {x,y,z,w,s, or t} of one vertex of ST geometry */
bool lidSLAB_ST_V_C (float &result, string fileName, int slabno, int vertno,
                    int componentno)
{
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Error: unable to open " << fileName << endl;
        exit(12);
    }
    string token;
    int vert, component;
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SLAB_ST")) {
        for (vert = 0; vert < 4; vert++) {
            for (component = 0; component < 6; component++) {
                NextToken(file, token);
                if (vert == vertno && component == componentno) {
                    result = stof(token);
                }
            }
        }
        file.close();
        return true;
    }
    file.close();
    return false;
}

/* Get the number of samples of U and V */
bool lidSAMPLES_UV  (int result[2], string fileName, int slabno)
{
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Error: unable to open " << fileName << endl;
        exit(12);
    }
    string token;
    int component;
    
    CHECKRESULTPTR(result);
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SAMPLES_UV")) {
        for (component = 0; component < 2; component++) {
            NextToken(file, token);
            result[component] = stoi(token);
        }
        file.close();
        return true;
    }
    file.close();
    return false;
}

/* Get the number of samples of U */
bool lidSAMPLES_U   (int &result, string fileName, int slabno)
{
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Error: unable to open " << fileName << endl;
        exit(12);
    }
    string token;
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SAMPLES_UV")) {
        if (NextToken(file, token)) {
            result = stoi(token);
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

/* Get the number of samples of V */
bool lidSAMPLES_V   (int &result, string fileName, int slabno)
{
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Error: unable to open " << fileName << endl;
        exit(12);
    }
    string token;
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SAMPLES_UV")) {
        if (NextToken(file, token) && NextToken(file, token)) {
            result = stoi(token);
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

/* Get the number of samples of S and T */
bool lidSAMPLES_ST  (int result[2], string fileName, int slabno)
{
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Error: unable to open " << fileName << endl;
        exit(12);
    }
    string token;
    int component;
    
    CHECKRESULTPTR(result);
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SAMPLES_ST")) {
        for (component = 0; component < 2; component++) {
            NextToken(file, token);
            result[component] = stoi(token);
        }
        file.close();
        return true;
    }
    file.close();
    return false;
}

/* Get the number of samples of S */
bool lidSAMPLES_S   (int &result, string fileName, int slabno)
{
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Error: unable to open " << fileName << endl;
        exit(12);
    }
    string token;
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SAMPLES_ST")) {
        if (NextToken(file, token)) {
            result = stoi(token);
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

/* Get the number of samples of T */
bool lidSAMPLES_T   (int &result, string fileName, int slabno)
{
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Error: unable to open " << fileName << endl;
        exit(12);
    }
    string token;
    
    if (GotoSlab(file, slabno) && GotoToken(file, "SAMPLES_ST")) {
        if (NextToken(file, token) && NextToken(file, token)) {
            result = stoi(token);
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

bool lidFORMAT      (string &result, string fileName, int slabno)
{
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Error: unable to open " << fileName << endl;
        exit(12);
    }
    
    if (GotoSlab(file, slabno) && GotoToken(file, "FORMAT")) {
        if (NextToken(file, result)) {
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}


/* This routine skips to the slabno'th slab listing in the file.
 * The first slab is 0, second is 1, etc...
 * It returns 1 on success, and 0 on failure.
 */
bool GotoSlab(ifstream &file, int slabno) {
    int currSlab = 0;
    string token;
    /* skip past slabno slabs */
    while (NextToken(file, token)) {
        if (token == "SLAB") {
            currSlab++;
            if (currSlab > slabno)
                return true;
        }
    }
    
    /* If we get here, we hit EOF. */
    fprintf(stderr, "Error:  file only has %d slabs.\n", currSlab);
    return false;
}

/* This routine reads until the first occurrence of the token.
 * It returns 1 on success, and 0 on failure.
 */
bool GotoToken(ifstream &file, string token) {
    string tok;
    
    /* Search for token */
    while (NextToken(file, tok)) {
        if (token == tok) return true;
        /* If we hit start of next slab, stop looking */
        if (tok == "SLAB") break;
    }
    /* if we get here, we didn't find the token */
    cerr << "Error:  didn't find token " << token << " in the slab.\n";
    return false;
}

/* This routine returns 1 on success, 0 on failure (end-of-hdr).
 * It assumes that buffer has been allocated and is large enough
 * to hold a token of maxTokenSize-1 + null-terminator.
 */
bool NextToken(ifstream &file, string &buffer)
{
    char c;
    /* State machine:
     * 0 = haven't seen start of token yet
     * 1 = reading token
     * 2 = token finished
     * 3 = inside a comment
     */
    int state = 0;
    
    buffer.clear();
    
    while (state != 2) {
        c = file.get();
        
        /* Handle EOF */
        if (c == EOF) {
            /* return true iff we've seen part of a token, 0 otherwise */
            if (state == 1) {
                return true;
            } else {
                return false;
            }
        }
        
        /* Handle comments */
        if (c == '#') {
            if (state == 1) {
//                buffer[index] = '\0';
                /* Finish reading the comment, so we start on next line next time */
                while ((c != '\n') && (c != EOF)) {
                    c = file.get();
                }
                /* now return token that we read. */
                return true;
            } else {
                /* We're inside a comment now */
                state=3;
                continue;
            }
        }
        
        /* Handle end-of-comment? */
        if (c == '\n') {
            if (state == 1) {
//                buffer[index] = '\0';
                return true;
            } else {
                /* End of comment, back into whitespace */
                state=0;
                continue;
            }
        }
        
        /* Handle whitespace */
        if (isspace(c)) {
            if (state == 1) {
//                buffer[index] = '\0';
                return true;
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
            if (state == 1) {
                buffer += c;
            }
        }
    }
    return false;
}
