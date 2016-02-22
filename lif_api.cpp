//
//  lif_api.cpp
//  rawCompress
//
//  Created by Jacob on 16/2/17.
//  Copyright © 2016年 Jacob. All rights reserved.
//

#include "lightfield.h"
#include "fileIO.h"

void *lfBeginWriteLif(LFOps *ops, LFInternOp *op)
{
    LFFileHdr *hdr = new LFFileHdr();
    
    if (!lfOpenOutFile(static_cast<string*>(op->op_extra), hdr)) {
        cerr << "Cannot read " << *static_cast<string*>(op->op_extra) << endl;
        exit(15);
    }
    
    op->op_func = lfWriteLif;
    op->op_end = lfEndWriteLif;
    op->op_update = ops->chain_msk & (LF_GEN_VQ_CODEARRAY | LF_GEN_VQ_TRAINSET)
        ? lfSerialUpdateBlockSlice : lfUpdateNoop;

    return hdr;
}

void lfWriteLif(LFSlab **slabs, int nslab, const int *pos,
                Descriptor descr, const void *input, int input_size,
                void *output, int output_size, void *aux_ptr)
{
    LFSlab *slab = slabs[pos[LF_SLAB_POS]];
    int u = pos[LF_U_POS];
    int v = pos[LF_V_POS];
    int id = slab->id;
    LFFileHdr *hdr = static_cast<LFFileHdr*>(aux_ptr);
    
    if (slab->shared->vq != nullptr) {
        LFVQCodebook *vq = slab->shared->vq;
        bool done = lfWriteBlock(input, id, slab, hdr,
                        u, u + (1<<vq->ubits) - 1, v, v + (1<<vq->vbits) - 1,
                        LF_INDEX_CHANNEL, LF_INT16);

        if (!done)
            exit(16);
    }
    else {
        /* write raw file. Don't use now. */
    }
}

void lfEndWriteLif(LFOps *ops, LFInternOp *op, void *aux_ptr)
{
    LFFileHdr *hdr = static_cast<LFFileHdr*>(aux_ptr);
    LFSlab *slab;
    LFVQCodebook *vq;
    
    /* output VQ codebook if necessary */
    for (int i = 0; i != ops->op_slab_cnt; ++i) {
        slab = ops->op_slabs[i];
        
        if ((vq = slab->shared->vq) == nullptr)
            continue;
        
        bool done = lfWriteCodebook(vq->codebook, vq->id, slab, hdr,
                                    LF_CODEBOOK_CHANNEL,
                                    vq->sample_size == 3 ? LF_INT8x3 : LF_INT8x4);
    
        if (!done)
            exit(17);
    }
    
    lfCloseOutFile(&hdr);
}


static void readLifFillSlab(LFOps *ops, LFFileHdr *hdr, LFSlab **slabs)
{
    /* determine slabs to be operated on */
    int id;
    int n = 0;
    for (int i = 0; i != ops->op_slab_cnt; ++i) {
        id = ops->op_slabs[i]->id;
        if (hdr->slabHdr[id].size != 0)
            ops->op_slabs[n++] = ops->op_slabs[i];
    }
    ops->op_slab_cnt = n;
    
    LFShared *sh;
    LFVQCodebook *vq;
    for (int i = 0; i != ops->op_slab_cnt; ++i) {
        id = ops->op_slabs[i]->id;
        
        /* copy slab attributes information over */
        *ops->op_slabs[i] = *slabs[id];
        
        /* read codebook information if needed */
        sh = ops->op_slabs[i]->shared;
        vq = sh->vq;
        if (hdr->slabHdr[id].format == LF_INDEX_CHANNEL
            && vq->codebook == nullptr)
        {
            lfReadCodebook(vq, sh, hdr);
        }
    }
}

void *lfBeginReadLif(LFOps *ops, LFInternOp *op)
{
    LFFileHdr *hdr = new LFFileHdr();
    LFSlab *slabs[LF_MAXSLABS];
    
    /* allocate temporary slabs */
    for (int i = 0; i != LF_MAXSLABS; ++i)
        slabs[i] = new LFSlab();
    
    /* Open the file and read the header */
    if (!lfOpenInFile(static_cast<string*>(op->op_extra), hdr,
                      slabs, LF_MAXSLABS)) {
        cerr << "Cannot read " << *static_cast<string*>(op->op_extra);
        exit(21);
    }
    
    /* fill in slab data structure */
    if (ops->chain_msk & LF_SET_SLAB)
        readLifFillSlab(ops, hdr, slabs);
    
    /* determine dataflow per this operation */
    op->op_func = lfReadLif;
    op->op_end  = lfEndReadLif;
    op->op_update = ops->chain_msk & (LF_GEN_VQ_TRAINSET | LF_GEN_VQ_CODEARRAY) ?
        lfSerialUpdateBlockSlice : lfUpdateNoop;
    
    /* free temporary slabs */
    for (int i = 0; i != LF_MAXSLABS; ++i)
        delete slabs[i];
    
    return hdr;
}

void lfReadLif(LFSlab **slabs, int nslab, const int *pos,
               Descriptor descr, const void *input, int input_size,
               void *output, int output_size, void *aux_ptr)
{
    LFSlab *slab = slabs[pos[LF_SLAB_POS]];
    int u = pos[LF_U_POS];
    int v = pos[LF_V_POS];
    int id = slab->id;
    LFFileHdr *hdr = static_cast<LFFileHdr*>(aux_ptr);
    
    if (hdr->slabHdr[id].format == LF_INDEX_CHANNEL) {
        LFVQCodebook *vq = slab->shared->vq;
        lfReadBlock(output, id, slab, hdr,
                    u, u+(1<<vq->ubits)-1, v, v+(1<<vq->vbits)-1,
                     LF_INDEX_CHANNEL, LF_INT16);
    }
    else {
        /* do nothing */
    }
}

void lfEndReadLif(LFOps *ops, LFInternOp *op, void *aux_ptr)
{
    LFFileHdr *hdr = static_cast<LFFileHdr*>(aux_ptr);
    lfCloseInFile(&hdr);
}