//
//  file.cpp
//  rawCompress
//
//  Created by Jacob on 16/2/18.
//  Copyright © 2016年 Jacob. All rights reserved.
//

#include <cassert>
#include "fileIO.h"


/* local helper function */
static void ClearHdr(LFFileHdr *hdr);
static bool WriteRawBlock(fstream &outfile, int offset, const char *buffer,
                          int elemCount, int elemInSize, int elemOutSize);
static int GetHdrLen(fstream &file);
static bool NextHdrToken(fstream &file, string &buffer);
static bool AssertToken(fstream &file, string token);
static bool ParseVQHdr(fstream &file, LFFileHdr *hdr, LFSlab *slabs[],
                       int nslabs);
static bool ParseSlabHdr(fstream &file, LFFileHdr *hdr, LFSlab *slabs[],
                         int nslabs);
static int Log2(unsigned long n);
static bool SkipSegment(fstream &file);
static bool ReadRawBlock(fstream &file, int offset, char *buffer,
                         int elemCount, int elemInSize, int elemOutSize);



/* ----------------------------------------------------------------------
 * ---------------          Low-level Functions           ---------------
 * ----------------------------------------------------------------------
 */

/* Open a Lightslab file */
bool lfOpenOutFile(string *fileName, LFFileHdr *hdr)
{
    hdr->file.open(*fileName, ios_base::out);
    if (!hdr->file.is_open()) {
        cerr << "Error: cannot open file " << *fileName << endl;
        return false;
    }
    
    /* it seems there is no need to call calloc */
    
    /* reserve enough hdr space to hold the header */
    hdr->hdrSize = LF_DEFAULT_HDRLEN;
    /* Initialize the header (clear it, and write terminating character) */
    ClearHdr(hdr);
    
    return true;
}

/* Write a portion (possibly all) of a slab of a lightfield to file */
bool lfWriteBlock(const void *block, int slabn, LFSlab *slab, LFFileHdr *hdr,
                  int uFirst, int uLast, int vFirst, int vLast,
                  LF_ChannelFormat format, LF_ChannelType type)
{
    LFShared *sh = slab->shared;
    LFSlabHdr *slabHdr = &(hdr->slabHdr[slabn]);
    int indexBytes = 0;         // Number of bytes necessary to index codebook
    int tilePixels = 0;         // Number of pixels contained in each tile
    LFVQCodebook *vq = sh->vq;
    int rawOffset, rawLen, inElemBytes, outElemBytes;
    int blockn, blockLen;
    
    if (vq != nullptr) {
        /* # of bytes necessary to index into the codebook array */
        /* 目前值设为常数2,即short类型 */
        assert(type == LF_INT16);
        indexBytes = 2;
        /* tilePixels is the number of pixels that are in each tile */
        tilePixels = (1 << (vq->ubits + vq->vbits + vq->sbits + vq->tbits));
    }
    
    if (format == LF_CODEBOOK_CHANNEL) {
        lfError("Error: lfWriteBlock called on a codebook!\n");
        return false;
    }
    else {
        /* Handle slab segment (not codebook) */
        /* Set hdr info, if this is the first write */
        if (slabHdr->size == 0) {
            slabHdr->format = format;
            slabHdr->type = type;
            slabHdr->slab = slab;
            slabHdr->offset = hdr->dataSize;
            if (vq != nullptr) {
                int channelSize = sh->nu * sh->nv * sh->ns * sh->nt * indexBytes / tilePixels;
                slabHdr->size = channelSize;
            }
            else {
                //                slabHdr->size = ((type == LF_INT8x4) ? 4 : 3) * (sh->nu * sh->nv * sh->ns * sh->nt);
            }
            
            lfOutput("Upping datasize by %d\n", slabHdr->size);
            hdr->dataSize += slabHdr->size;
        }
        
        /* Now write the binary section to file */
        switch (slabHdr->format) {
            case LF_RGB_CHANNEL:
            case LF_RGBA_CHANNEL:
                break;
                
            case LF_INDEX_CHANNEL:
                /* Note: this requires that the slabslices in u and v is a multiple
                 * of the tile slice. (So, for example, if you hace 2*2*2*2 blocks.
                 * then uFirst and vFirst must be even numbers, and uLast and vLast
                 * must be odd nubmers.)
                 */
                
                /* check to make sure that uFirst and vFirst are divisible by
                 * the tile size in u and v.
                 */
                if ((uFirst % (1 << sh->vq->ubits) != 0) ||
                    (vFirst % (1 << sh->vq->vbits) != 0)) {
                    lfError("Error: for writing out VQ indices, the slice must start"
                            " on an even boundary (Divisible by the tile size)\n");
                    return false;
                }
                /* Check to make sure that slice size (in u and v) is divisible
                 * by the tile size.
                 */
                if (((uLast + 1) % (1 << sh->vq->ubits) != 0) ||
                    ((vLast + 1) % (1 << sh->vq->vbits) != 0)) {
                    lfError("Error: for writing out VQ indices, the size of the "
                            " slice must be evenly divisible by the tile size\n");
                    return false;
                }
                
                /* Calculate the block offset */
                blockn = (uFirst >> sh->vq->ubits) * (sh->nv >> sh->vq->vbits) + (vFirst >> sh->vq->vbits);
                /* The length of a single tile slice in uv */
                blockLen = (1 << sh->vq->ubits) * (1 << sh->vq->vbits)
                * sh->ns * sh->nt / tilePixels;
                /* offset */
                rawOffset = hdr->hdrSize + slabHdr->offset +
                indexBytes * blockn * blockLen;
                
                /* Calculate the length of the block */
                rawLen = (1 + uLast - uFirst) * (1 + vLast - vFirst) *
                sh->ns * sh->nt / tilePixels;
                
                inElemBytes = outElemBytes = indexBytes;
                break;
                
            default:
                lfError("Error: unrecognized data format %d\n", slabHdr->format);
                break;
        }
        
        return WriteRawBlock(hdr->file, rawOffset, static_cast<const char*>(block),
                             rawLen, inElemBytes, outElemBytes);
    }
}


static void ClearHdr(LFFileHdr *hdr)
{
    /* go to start of file */
    hdr->file.seekp(0, ios_base::beg);
    
    /* clear hdr space */
    for (int i = 0; i != (hdr->hdrSize - 1); ++i) {
        hdr->file.put(LF_HDR_FILLER);
    }
    
    /* Terminating character */
    hdr->file.put(LF_HDR_TERMINATOR);
    
    /* go back to start for writing hdr over it */
    hdr->file.seekp(0, ios_base::beg);
}

/* Write out raw block of data. It always writes little-endian,
 * for cross-platform compatibility
 */
static bool WriteRawBlock(fstream &outfile, int offset, const char *buffer,
                          int elemCount, int elemInSize, int elemOutSize)
{
    if (elemInSize != 1 && elemInSize != 2 && elemInSize != 4 && elemInSize != 8)
    {
        lfError("Error: WriteRawBlock: What kind of number uses %d bytes?\n", elemInSize);
        return false;
    }
    
    /* go to starting offset in the file */
    outfile.seekp(offset, ios_base::beg);
    
    unsigned long long data = 0;
    /* Now write */
    for (int i = 0; i != elemCount; ++i) {
        /* Set data = element size */
        switch (elemInSize) {
            case 1:
                data = *buffer;
                break;
                
            case 2:
                data = *static_cast<const unsigned short*>
                (static_cast<const void*>(buffer));
                break;
                
            case 4:
                data = *static_cast<const unsigned int*>
                (static_cast<const void*>(buffer));
                break;
                
            case 8:
                data = *static_cast<const unsigned long long*>
                (static_cast<const void*>(buffer));
                break;
        }
        
        buffer += elemInSize;
        
        /* Write the data, little-endian */
        for (int j = 0; j != elemOutSize; ++j) {
            outfile.put(data & 0xff);
            data >>= 8;
        }
    }
    
    return true;
}

/* Write an entire codebook to file */
bool lfWriteCodebook(const void *block, int vqn, LFSlab *slab, LFFileHdr *hdr,
                     LF_ChannelFormat format, LF_ChannelType type)
{
    LFShared *sh = slab->shared;
    LFVQHdr *vqHdr = &(hdr->vqHdr[vqn]);
    int indexBytes = 0;
    int tilePixels = 0;
    LFVQCodebook *vq = sh->vq;
    int rawOffset, rawLen, inElemBytes, outElemBytes;
    
    lfOutput("lfWriteCodebook, format %d\n", format);
    
    if (vq != nullptr) {
        /* indexBytes = # of bytes needed to index into the codebook array */
        indexBytes = 2;
        /* tilePixels is the number of pixels that are in each tile */
        tilePixels = (1 << (vq->ubits + vq->vbits + vq->sbits + vq->tbits));
    }
    
    if (format == LF_CODEBOOK_CHANNEL) {
        /* Handle codebook */
        
        /* Don't re-set the hdr info (which would allocate *another* block
         * for the codebook), if the codebook is already used by another slab.
         */
        if (vqHdr->size != 0) {
            lfOutput("skipping codebook %d -- it's shared by another slab.\n",
                     vq->id);
            return true;
        }
        
        /* Set hdr info, since we assume that we write the entire codebook out
         * at one time.
         */
        vqHdr->format = (type == LF_INT8x3 ? LF_RGB_CHANNEL : LF_RGBA_CHANNEL);
        vqHdr->type = type;
        vqHdr->codebook = vq;
        vqHdr->offset = hdr->dataSize;
        vqHdr->size = vq->codesize *
        (vq->sample_size << (vq->ubits + vq->vbits + vq->sbits + vq->tbits));
        
        lfOutput("upping datasize by VQ-codebook: %d\n", vqHdr->size);
        
        hdr->dataSize += vqHdr->size;
        rawOffset = hdr->hdrSize + vqHdr->offset;
        
        // block = vq->codebook;
        
        rawLen = vqHdr->size;
        inElemBytes = outElemBytes = 1;
        
        return WriteRawBlock(hdr->file, rawOffset,
                             static_cast<const char*>(block),
                             rawLen, inElemBytes, outElemBytes);
        
    }
    else {
        lfError("Error: lfWriteCodebook called on a non-codebook.\n");
        return false;
    }
}

bool lfCloseOutFile(LFFileHdr **hdr)
{
    LFSlab *slab;
    LFVQCodebook *vq;
    LFSlabHdr *slabHdr;
    LFVQHdr *vqHdr;
    string uvgeomStr[4] =
    { "geometry_uv", "           ", "           ", "           " };
    string stgeomStr[4] =
    { "geometry_st", "           ", "           ", "           " };
    
    /* Write the header */
    fstream &outfile = (*hdr)->file;
    outfile.seekp(0, ios_base::beg);
    outfile << "LIF1.0" << endl;
    
    /* Print the datasize */
    outfile << "datasize " << (*hdr)->dataSize << endl << endl;
    
    /* Print the lightfield */
    outfile << "bgnlightfield 1" << endl;
    
    /* Go through the slabs one by one */
    for (int slabno = 0; slabno != LF_MAXSLABS; ++slabno) {
        slabHdr = &((*hdr)->slabHdr[slabno]);
        slab = slabHdr->slab;
        if (slab == nullptr)
            continue;
        
        outfile << "\tbgnsegment slab " << slabno << endl;
        
        if ((vq = slab->shared->vq) != nullptr) {
            /* Output VQ index channel */
            outfile << "\t\tcompression vq " << slab->shared->vq->id << endl;
            outfile << "\t\tformat index" << endl;
            outfile << "\t\tbgnchannel index    # vq codebook indices" << endl;
            outfile << "\t\t\ttype "
            << ((slabHdr->type == LF_INT8) ? "int8" :
                (slabHdr->type == LF_INT16) ? "int16" : "int32")
            << endl;
            outfile << "\t\t\toffset " << slabHdr->offset << endl;
            outfile << "\t\t\tsize " << slabHdr->size << endl;
            outfile << "\t\tendchannel" << endl;
        }
        else {
            outfile << "\t\tcompression none" << endl;
            switch (slabHdr->format) {
                case LF_RGB_CHANNEL:
                    outfile << "\t\tformat rgb" << endl;
                    outfile << "\t\tbgnchannel rgb" << endl;
                    outfile << "\t\t\ttype int8x3" << endl;
                    outfile << "\t\t\toffset " << slabHdr->offset << endl;
                    outfile << "\t\t\tsize " << slabHdr->size << endl;
                    outfile << "\t\tendchannel" << endl;
                    break;
                    
                case LF_RGBA_CHANNEL:
                    outfile << "\t\tformat rgba" << endl;
                    outfile << "\t\tbgnchannel rgba" << endl;
                    outfile << "\t\t\ttype int8x4" << endl;
                    outfile << "\t\t\toffset " << slabHdr->offset << endl;
                    outfile << "\t\t\tsize " << slabHdr->size << endl;
                    outfile << "\t\tendchannel" << endl;
            }
        }
        
        /* Print out dimention in u, v, s, t */
        outfile << "\t\tsamples_uv " << slab->shared->nu << " " << slab->shared->nv
        << endl;
        outfile << "\t\tsamples_st " << slab->shared->ns << " " << slab->shared->nt
        << endl;
        
        /* Print out uv slab coordinates */
        for (int i = 0; i != 4; ++i) {
            outfile << "\t\t" << uvgeomStr[i] << " "
            << slab->uv[i].ox << " " << slab->uv[i].oy << " "
            << slab->uv[i].oz << " " << slab->uv[i].ow << " "
            << slab->uv[i].s << " " << slab->uv[i].t << endl;
        }
        /* Print out st slab coordinates */
        for (int i = 0; i != 4; ++i) {
            outfile << "\t\t" << stgeomStr[i] << " "
            << slab->st[i].ox << " " << slab->st[i].oy << " "
            << slab->st[i].oz << " " << slab->st[i].ow << " "
            << slab->st[i].s << " " << slab->st[i].t << endl;
        }
        
        /* End of segment */
        outfile << "\tendsegment # " << slabno << endl;    /* +1 to start at 1 */
    }
    
    /* Go through the vqs one by one */
    for (int vqno = 0; vqno != LF_MAXSLABS; ++vqno) {
        vqHdr = &((*hdr)->vqHdr[vqno]);
        vq = vqHdr->codebook;
        if (vq == nullptr) continue;
        
        outfile << "\tbgnsegment vq " << vqno << endl;
        
        switch (vqHdr->format) {
            case LF_RGB_CHANNEL:
                outfile << "\t\tformat rgb" << endl;
                outfile << "\t\tbgnchannel rgb" << endl;
                outfile << "\t\t\ttype int8x3" << endl;
                outfile << "\t\t\toffset " << vqHdr->offset << endl;
                outfile << "\t\t\tsize " << vqHdr->size << endl;
                outfile << "\t\tendchannel" << endl;
                break;
                
            case LF_RGBA_CHANNEL:
                outfile << "\t\tformat rgba" << endl;
                outfile << "\t\tbgnchannel rgba" << endl;
                outfile << "\t\t\ttype int8x4" << endl;
                outfile << "\t\t\toffset " << vqHdr->offset << endl;
                outfile << "\t\t\tsize " << vqHdr->size << endl;
                outfile << "\t\tendchannel" << endl;
                break;
        }
        
        /* print tile info */
        outfile << "\t\ttiles " << vq->codesize << endl;
        outfile << "\t\ttilesize "
        << (1 << vq->ubits) << " " << (1 << vq->vbits) << " "
        << (1 << vq->sbits) << " " << (1 << vq->tbits) << endl;
        
        /* End of segment */
        outfile << "\tendsegment # " << vqno << endl;  /* +1 to start at 1 */
    }
    
    /* End of lightfield entry */
    outfile << "endlightfield # 1" << endl;
    outfile << "endheader" << endl;
    
    /* clean up the hdr structure */
    delete (*hdr);
    *hdr = nullptr;
    
    /* close the output file */
    outfile.close();
    return true;
}



/* Open a lightfield file, alloc the LFFiled and LFFilehdr structs,
 * and initialize them from the file header info.
 */
bool lfOpenInFile(string *fileName, LFFileHdr *hdr, LFSlab *slabs[],
                  int nslabs)
{
    /* alloc the slabHdr array */
    //    *hdr = new LFFileHdr();
    
    fstream &inFile = hdr->file;
    inFile.open(*fileName, ios_base::in);
    if (!inFile.is_open()) {
        cerr << "Error: cannot open file " << fileName << endl;
        exit(20);
    }
    
    /* Figure out hdr length */
    hdr->hdrSize = GetHdrLen(inFile);
    
    /* allocate shared structure */
    for (int slabn = 0; slabn != LF_MAXSLABS; ++slabn) {
        slabs[slabn]->shared = new LFShared();
    }
    
    /* make sure we're at beginning of the file */
    inFile.seekg(0, ios_base::beg);
    
    string buffer;
    /* Read the hdr, and use it to fill in the data for enabled slabs */
    AssertToken(inFile, "LIF1.0");
    AssertToken(inFile, "datasize");
    NextHdrToken(inFile, buffer);
    hdr->dataSize = stoi(buffer);
    AssertToken(inFile, "bgnlightfield");
    NextHdrToken(inFile, buffer);
    /* Note: ignore lightfield number for now */
    
    /* First, read all the codebook headers */
    fstream::pos_type currPos = inFile.tellg();
    while (NextHdrToken(inFile, buffer)) {
        if (buffer == "endlightfield")
            break;
        else if (buffer == "bgnsegment") {
            NextHdrToken(inFile, buffer);
            /* if it's a vq segment, parse the header */
            if (buffer == "vq")
                ParseVQHdr(inFile, hdr, slabs, nslabs);
            /* If it's not a vq segment, skip it for now... */
            else
                SkipSegment(inFile);
        }
        else {
            cerr << "Error: unexpected token: \"" << buffer << "\"\n";
            return false;
        }
    }
    
    /* Restore file position to start of the lightfield */
    inFile.seekg(currPos);
    
    /* Now we can parse all the slabs */
    while (true) {
        /* Read next token */
        if (!NextHdrToken(inFile, buffer)) {
            lfError("Error: Unexpected end of file header.\n");
            return false;
        }
        
        /* Check if endlightfield */
        if (buffer == "endlightfield")
            break;
        
        /* Check if bgnsegment */
        else if (buffer == "bgnsegment") {
            NextHdrToken(inFile, buffer);
            if (buffer == "slab")
                ParseSlabHdr(inFile, hdr, slabs, nslabs);
            else
                SkipSegment(inFile);
        }
        else  {
            cerr << "Error: unexpected token: \"" << buffer << "\"\n";
            return false;
        }
    }
    /* return */
    return true;
}

/* this routine reads the rest of the segment */
static bool SkipSegment(fstream &file)
{
    string buffer;
    while (NextHdrToken(file, buffer)) {
        if (buffer == "endsegment")
            return true;
    }
    /* if we get here, we ran out of the header without endsegment */
    return false;
}

/* Parse a VQ codebook header. It assumes that "bgnsegment vq" has already
 * been parsed.
 */
static bool ParseVQHdr(fstream &file, LFFileHdr *hdr, LFSlab *slabs[],
                       int nslabs)
{
    string buffer;
    int vqn;
    
    /* get codebook number */
    NextHdrToken(file, buffer);
    vqn = stoi(buffer);
    lfOutput("ParseVQHdr %d\n", vqn);
    /* Note: does not check to see if slab is enabled */
    lfOutput("reading vq hdr info for VQ codebook %d\n", vqn);
    
    /* Allocate the codebook structure */
    hdr->vqHdr[vqn].codebook = new LFVQCodebook();
    
    /* Parse the rest of the segment */
    while (NextHdrToken(file, buffer)) {
        if (buffer == "endsegment") return true;
        
        else if (buffer == "format") {
            NextHdrToken(file, buffer);
            if (buffer == "rgb") {
                hdr->vqHdr[vqn].format = LF_RGB_CHANNEL;
                hdr->vqHdr[vqn].codebook->sample_size = 3;
            }
            else if (buffer == "rgba") {
                hdr->vqHdr[vqn].format = LF_RGBA_CHANNEL;
                hdr->vqHdr[vqn].codebook->sample_size = 4;
            }
            else {
                cerr << "Error: Unknown codebook format " << buffer << endl;
                return false;
            }
        }
        
        else if (buffer == "bgnchannel") {
            NextHdrToken(file, buffer);
            /* VQ codebook shouldn't be vq encoded */
            AssertToken(file, "type");
            NextHdrToken(file, buffer);
            if (buffer == "int8x3") {
                hdr->vqHdr[vqn].type = LF_INT8x3;
            }
            else if (buffer == "int8x4") {
                hdr->vqHdr[vqn].type = LF_INT8x4;
            }
            /* offset */
            AssertToken(file, "offset");
            NextHdrToken(file, buffer);
            hdr->vqHdr[vqn].offset = stoi(buffer);
            /* size */
            AssertToken(file, "size");
            NextHdrToken(file, buffer);
            hdr->vqHdr[vqn].size = stoi(buffer);
            lfOutput("vq %d size %d\n", vqn, hdr->vqHdr[vqn].size);
            /* end channel */
            AssertToken(file, "endchannel");
        }
        
        /* number of tiles in codebook */
        else if (buffer == "tiles") {
            NextHdrToken(file, buffer);
            hdr->vqHdr[vqn].codebook->codesize = stoi(buffer);
        }
        
        /* size of tiles in codebook */
        else if (buffer == "tilesize") {
            NextHdrToken(file, buffer);
            hdr->vqHdr[vqn].codebook->ubits = Log2(stoi(buffer));
            NextHdrToken(file, buffer);
            hdr->vqHdr[vqn].codebook->vbits = Log2(stoi(buffer));
            NextHdrToken(file, buffer);
            hdr->vqHdr[vqn].codebook->sbits = Log2(stoi(buffer));
            NextHdrToken(file, buffer);
            hdr->vqHdr[vqn].codebook->tbits = Log2(stoi(buffer));
        }
    }
    /* if we get here, then we ran out of header without endsegment */
    return false;
}

/* Parse a slab header. It assumes that "bgnsegment slab" has already
 * been parsed.
 */
static bool ParseSlabHdr(fstream &file, LFFileHdr *hdr, LFSlab *slabs[],
                         int nslabs)
{
    string buffer;
    int slabn, vqn;
    
    /* get slab number */
    NextHdrToken(file, buffer);
    slabn = stoi(buffer);
    /* Note: does not check to see if slab is enabled... */
    lfOutput("reading slab hdr info for slab %d\n", slabn);
    slabs[slabn]->id = slabn;
    
    /* Parse the rest of the segment */
    while (NextHdrToken(file, buffer)) {
        if (buffer == "endsegment") return true;
        
        if (buffer == "compression") {
            NextHdrToken(file, buffer);
            if (buffer == "vq") {
                NextHdrToken(file, buffer);
                vqn = stoi(buffer);
                slabs[slabn]->shared->vq = hdr->vqHdr[vqn].codebook;
            }
            else {
                slabs[slabn]->shared->vq = nullptr;
            }
        }
        
        else if (buffer == "format") {
            NextHdrToken(file, buffer);
            if (buffer == "index") {
                hdr->slabHdr[slabn].format = LF_INDEX_CHANNEL;
                slabs[slabn]->shared->sample_size = 2;
            }
            else if (buffer == "rgb" || buffer == "rgba")
                ;
        }
        
        else if (buffer == "bgnchannel") {
            NextHdrToken(file, buffer);
            if (buffer == "index") {
                AssertToken(file, "type");
                NextHdrToken(file, buffer);
                if (buffer == "int8")
                    hdr->slabHdr[slabn].type = LF_INT8;
                else if (buffer == "int16")
                    hdr->slabHdr[slabn].type = LF_INT16;
                else if (buffer == "int32")
                    hdr->slabHdr[slabn].type = LF_INT32;
            }
            else /* not vq encoded */;
            
            /* offset */
            AssertToken(file, "offset");
            NextHdrToken(file, buffer);
            hdr->slabHdr[slabn].offset = stoi(buffer);
            /* size */
            AssertToken(file, "size");
            NextHdrToken(file, buffer);
            hdr->slabHdr[slabn].size = stoi(buffer);
            lfOutput("slab %d size %d\n", slabn, hdr->slabHdr[slabn].size);
            /* end channel */
            AssertToken(file, "endchannel");
        }
        
        /* sample size */
        else if (buffer == "samples_uv") {
            NextHdrToken(file, buffer);
            slabs[slabn]->shared->nu = stoi(buffer);
            NextHdrToken(file, buffer);
            slabs[slabn]->shared->nv = stoi(buffer);
        }
        else if (buffer == "samples_st") {
            NextHdrToken(file, buffer);
            slabs[slabn]->shared->ns = stoi(buffer);
            NextHdrToken(file, buffer);
            slabs[slabn]->shared->nt = stoi(buffer);
        }
        
        /* geometry */
        else if (buffer == "geometry_uv") {
            for (int vtx = 0; vtx != 4; ++vtx) {
                NextHdrToken(file, buffer);
                slabs[slabn]->uv[vtx].ox = stof(buffer);
                NextHdrToken(file, buffer);
                slabs[slabn]->uv[vtx].oy = stof(buffer);
                NextHdrToken(file, buffer);
                slabs[slabn]->uv[vtx].oz = stof(buffer);
                NextHdrToken(file, buffer);
                slabs[slabn]->uv[vtx].ow = stof(buffer);
                NextHdrToken(file, buffer);
                slabs[slabn]->uv[vtx].s = stof(buffer);
                NextHdrToken(file, buffer);
                slabs[slabn]->uv[vtx].t = stof(buffer);
            }
        }
        else if (buffer == "geometry_st") {
            for (int vtx = 0; vtx != 4; ++vtx) {
                NextHdrToken(file, buffer);
                slabs[slabn]->st[vtx].ox = stof(buffer);
                NextHdrToken(file, buffer);
                slabs[slabn]->st[vtx].oy = stof(buffer);
                NextHdrToken(file, buffer);
                slabs[slabn]->st[vtx].oz = stof(buffer);
                NextHdrToken(file, buffer);
                slabs[slabn]->st[vtx].ow = stof(buffer);
                NextHdrToken(file, buffer);
                slabs[slabn]->st[vtx].s = stof(buffer);
                NextHdrToken(file, buffer);
                slabs[slabn]->st[vtx].t = stof(buffer);
            }
        }
    }
    /* if we get here, then we ran out of a header without endsegment */
    return false;
}

static int GetHdrLen(fstream &file)
{
    int i = 0;
    char c;
    
    file.seekg(0, ios_base::beg);
    /* Count up characters in header until we hit terminator character */
    while ((c = file.get()) != EOF) {
        ++i;
        if (c == LF_HDR_TERMINATOR)
            break;
    }
    file.seekg(0, ios_base::beg);
    
    return i;
}

/* This routine returns true on success, false on failure (end-of-hdr)
 */
static bool NextHdrToken(fstream &file, string &buffer)
{
    char c;
    buffer.clear();
    
    /* state machine:
     * 0 = haven't seen start of token yet
     * 1 = reading token
     */
    int state = 0;
    
    while (true) {
        c = file.get();
        
        /* Handle EOF */
        if (c == EOF || c == LF_HDR_TERMINATOR) {
            if (state == 1) return true;
            else            return false;
        }
        
        /* Handle comments */
        if (c == '#') {
            /* Finish reading the comment, so we start on next line next time */
            while (c != '\n' && c != EOF)
                c = file.get();
            if (state == 1) return true;
            else            continue;
        }
        
        /* Handle end-of-comment */
        if (c == '\n') {
            if (state == 1) return true;
            else {
                state = 0;
                continue;
            }
        }
        
        /* Handle whitespace */
        if (isspace(c)) {
            if (state == 1) return true;
            else            continue;
        }
        
        /* Default case */
        {
            if (state == 0)
                state = 1;
            if (state == 1)
                buffer += c;
        }
    }
}

/* Read a token from input file, check to make sure it matched what
 * we were expecting
 */
static bool AssertToken(fstream &file, string token)
{
    string buf;
    /* Read next token */
    if (!NextHdrToken(file, buf)) {
        cerr << "Error: expected token \"" << token << "\", got EOF." << endl;
        return 0;
    }
    
    /* Check to make sure it matches what we expect */
    if (buf == token)
        return true;
    else {
        cerr << "Error: expected token \"" << token << "\", got \""
        << buf << "\"." << endl;
        return false;
    }
}

/* this function takes the log (base2) of n, assuming n is a power of 2 */
static int Log2(unsigned long n)
{
    for (int log = 0; log < 32; ++log)
        if (1 & (n >> log))
            return log;
    
    return -1;
}


/* Read an entire codebook from file */
/* (this will allocate the memory needed for the codebook array) */
bool lfReadCodebook(LFVQCodebook *vq, LFShared *sh, LFFileHdr *hdr)
{
    lfOutput("lfReadCodebook %d\n", vq->id);
    
    /* tileBytes = # of pixels that are in each tile */
    int tilePixels = (1 << (vq->ubits + vq->vbits + vq->sbits + vq->tbits));
    
    /* allocate space for the codebook array */
    if (vq->codebook != nullptr)
        lfError("Warning: lfReadCodebook: codebook already contains data!\n");
    vq->codebook = static_cast<void *>
                (new char[vq->codesize * tilePixels * vq->sample_size]);
    
    LFVQHdr *vqHdr = &(hdr->vqHdr[vq->id]);
    
    int rawOffset = hdr->hdrSize + vqHdr->offset;
    int rawLen = vqHdr->size;
    int inElemBytes = 1;
    int outElemBytes = 1;
    
    if (ReadRawBlock(hdr->file, rawOffset, static_cast<char *>(vq->codebook),
                 rawLen, inElemBytes, outElemBytes))
    {
        lfOutput("Done reading %d bytes of codebook.\n", rawLen);
        return true;
    }
    else
        return false;
}

/* Read in raw block of data. It reads writes little-endian,
 * for cross-platform compatibility.
 */
static bool ReadRawBlock(fstream &file, int offset, char *buffer,
                         int elemCount, int elemInSize, int elemOutSize)
{
    /* sanity check to avoid bus errors */
    if (elemOutSize != 1 && elemOutSize != 2 && elemOutSize != 4
        && elemOutSize != 8) {
        lfError("Error: ReadRawBlock: what kind of number uses %d bytes??\n",
                elemOutSize);
        return false;
    }
    
    /* Go to starting offset in the file */
    file.seekg(offset, ios_base::beg);
    
    unsigned long long data;
    unsigned int tempData;
    /* Now read */
    for (int i = 0; i != elemCount; ++i) {
        /* Read the data, little-endian */
        data = tempData = 0;
        for (int j = 0; j != elemInSize; ++j) {
            tempData = file.get();
            /* put the character in the jth byte */
            data += tempData << (j << 3);
        }
        
        switch (elemOutSize) {
            case 1:
                *buffer = data;
                break;
                
            case 2:
                *static_cast<short*>(static_cast<void*>(buffer)) = data;
                break;
                
            case 4:
                *static_cast<int*>(static_cast<void*>(buffer)) = data;
                break;
                
            case 8:
                *static_cast<long long *>(static_cast<void*>(buffer)) = data;
                break;
        }
        buffer += elemOutSize;
    }
    
    return true;
}

bool lfReadBlock(void *block, int slabn, LFSlab *slab, LFFileHdr *hdr,
                 int uFirst, int uLast, int vFirst, int vLast,
                 LF_ChannelFormat format, LF_ChannelType type)
{
    LFShared *sh = slab->shared;
    LFVQCodebook *vq = sh->vq;
    LFSlabHdr *slabHdr = &(hdr->slabHdr[slabn]);
    int indexBytes = 0;
    int tilePixels = 0;
    int blockn, blockLen;
    int rawOffset, rawLen, inElemBytes, outElemBytes;
    
    if (vq != nullptr) {
        indexBytes = 2;
        tilePixels = 1 << (vq->ubits + vq->vbits + vq->sbits + vq->tbits);
    }
    
    if (format == LF_CODEBOOK_CHANNEL) {
        lfError("Error: lfReadBlock called on a codebook.\n");
        return false;
    }
    
    switch (slabHdr->format) {
        case LF_RGB_CHANNEL:
        case LF_RGBA_CHANNEL:
            /* do nothing */
            break;
            
        case LF_INDEX_CHANNEL:
            /* Note: this requires that the slabslice in u and v is a multiple
             * of the tile size.  (So, for example, if you have 2x2x2x2 blocks,
             * then uFirst and vFirst must be even numbers, and uLast and vLast
             * must be odd numbers.)
             */
            
            /* Check to make sure that uFirst and vFirst are divisible by
             * the tile size in u and v.
             */
            if (((uFirst % (1 << vq->ubits)) != 0) ||
                ((vFirst % (1 << vq->vbits)) != 0)) {
                lfError("Error: for reading in VQ indices, the slice must start"
                        " on an even boundary (Divisible by the tile size)\n");
                return false;
            }
            /* Check to make sure that slice size (in u and v) is divisible
             * by the tile size.
             */
            if (((uLast+1) % (1 << vq->ubits)) != 0 ||
                ((vLast + 1) % (1 << vq->vbits)) !=0) {
                lfError("Error: for reading in VQ indices, the size of the slice"
                        " must be evenly divisible by the tile size\n");
                return false;
            }
            
            /* calculate the block offset */
            blockn = (uFirst >> vq->ubits) * (sh->nv >> vq->vbits) +
                (vFirst >> vq->vbits);
            /* the length of a single tile slice in uv */
            blockLen = (sh->ns >> vq->sbits) * (sh->nt >> vq->tbits);
            /* offset */
            rawOffset = hdr->hdrSize + slabHdr->offset +
                blockn * blockLen * indexBytes;
            
            rawLen = (1 + uLast - uFirst) * (1 + vLast - vFirst) *
                sh->ns * sh->nt / tilePixels;
            
            inElemBytes = outElemBytes = indexBytes;
            
            break;
    }
    
    return ReadRawBlock(hdr->file, rawOffset, static_cast<char*>(block),
                        rawLen, inElemBytes, outElemBytes);
}

/* close the input file and free the file header structure */
bool lfCloseInFile(LFFileHdr **hdr)
{
    (*hdr)->file.close();
    
    delete *hdr;
    *hdr = nullptr;
    return true;
}