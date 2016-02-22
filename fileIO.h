//
//  file.hpp
//  rawCompress
//
//  Created by Jacob on 16/2/18.
//  Copyright © 2016年 Jacob. All rights reserved.
//

#ifndef fileIO_h
#define fileIO_h

#include <fstream>
#include <iostream>
#include "lightfield.h"

using namespace std;

/* Currently, we set aside this much room for the header when we
 * write a file.  For reading, it will accept any sized header.
 * This header should be big enough to hold information about
 * the maximum number of slabs, and a separate vq codebook for
 * every slab.
 */

const int LF_DEFAULT_HDRLEN = 2000 * LF_MAXSLABS;

/* This character marks the end of the header.  When it appears in
 * the file, the next character should be the first byte of binary
 * data.
 */
const char LF_HDR_TERMINATOR = '\0';
const char LF_HDR_FILLER = ' ';

/* These data structures record information about the file that is
 * not necessarily stored (or not easily computed) from the
 * regular lightfield / lightslab structres.
 */
typedef enum {
    LF_RGB_CHANNEL,
    LF_RGBA_CHANNEL,
    LF_INDEX_CHANNEL,
    LF_CODEBOOK_CHANNEL,
} LF_ChannelFormat;

typedef enum {
    LF_INT8,
    LF_INT8x3,
    LF_INT8x4,
    LF_INT16,
    LF_INT32,
} LF_ChannelType;

typedef struct {
    LF_ChannelFormat    format;
    LF_ChannelType      type;
    int                 offset;
    int                 size;
    LFSlab              *slab;
} LFSlabHdr;

typedef struct {
    LF_ChannelFormat    format;
    LF_ChannelType      type;
    int                 offset;
    int                 size;
    LFVQCodebook        *codebook;
} LFVQHdr;

typedef struct {
    int                 hdrSize;
    int                 dataSize;
    LFSlabHdr           slabHdr[LF_MAXSLABS];
    LFVQHdr             vqHdr[LF_MAXSLABS];
    fstream             file;
} LFFileHdr;


#endif /* fileIO_h */


/* Open a Lightslab file, malloc the LFFileHdr struct,
 * and fill it in with data from the LFField structure.
 */
bool lfOpenOutFile(string *fileName, LFFileHdr *hdr);

/* Write a portion (possibly all) of a slab of a lightfield to file */
bool lfWriteBlock(const void *block, int slabn, LFSlab *slab, LFFileHdr *,
                 int uFirst, int uLast, int vFirst, int vLast,
                 LF_ChannelFormat format, LF_ChannelType type);

/* Write an entire codebook to file */
bool lfWriteCodebook(const void *block, int vqn, LFSlab *slab, LFFileHdr *hdr,
                     LF_ChannelFormat format, LF_ChannelType type);


/* Write the header (if necessary) and close the file.
 * NOTE -- This frees the LFFileHdr Structure!
 */
bool lfCloseOutFile(LFFileHdr **hdr);


/* Open a Lightfield file, malloc the LFField and LFFileHdr structs,
 * and initialize them from the file header info
 */
bool lfOpenInFile(string *filename, LFFileHdr *hdr, LFSlab *slabs[],
                  int nslabs);

/* Read an entire codebook from file */
bool lfReadCodebook(LFVQCodebook *vq, LFShared *sh, LFFileHdr *hdr);

/* Read a portion (possibly all) of a slab of a lightfield from file */
bool lfReadBlock(void *block, int slabn, LFSlab *slab, LFFileHdr *hdr,
                 int uFirst, int uLast, int vFirst, int vLast,
                 LF_ChannelFormat format, LF_ChannelType type);

/* close the input file */
/* free the file header structure */
bool lfCloseInFile(LFFileHdr **hdr);