//
//  compression.cpp
//  stuff to deal with compression (mainly VQ for now)
//
//  Created by Jacob on 16/2/10.
//  Copyright © 2016年 Jacob. All rights reserved.
//

#include "lightfield.h"
#include "opencv2/opencv.hpp"
#include "vq/vq.h"
#include "vq/vq_iface.h"


static void fillVQStruct(LFOps *ops, LFInternOp *op)
{
    LFSlab **slabs = ops->op_slabs;
    int nslabs = ops->op_slab_cnt;
    Descriptor descr = op->op_descr;
    int tile_u_bits = 0;
    int tile_v_bits = 0;
    int tile_s_bits = 2;
    int tile_t_bits = 2;
    float train_pct = 0.01f;
    int code_size = 1 << 16;
    
    if (descr.count(LF_VQ_TILESIZE) > 0) {
        int *tilesize = (int*)(descr[LF_VQ_TILESIZE]);
        tile_u_bits = *tilesize++;
        tile_v_bits = *tilesize++;
        tile_s_bits = *tilesize++;
        tile_t_bits = *tilesize;
    }
    if (descr.count(LF_VQ_TRAIN_SIZE) > 0) {
        float *trainsize = (float*)(descr[LF_VQ_TRAIN_SIZE]);
        train_pct = *trainsize;
    }
    if (descr.count(LF_VQ_CODESIZE) > 0) {
        int *codesize = (int*)(descr[LF_VQ_CODESIZE]);
        code_size = *codesize;
    }
    
    for (int i = 0; i < nslabs; ++i)
        if (slabs[i]->shared->vq != nullptr)
            delete slabs[i]->shared->vq;
    
    LFVQCodebook *vq = new LFVQCodebook();
    vq->ubits = tile_u_bits;
    vq->vbits = tile_v_bits;
    vq->sbits = tile_s_bits;
    vq->tbits = tile_t_bits;
    vq->codesize = code_size;
    vq->train_pct = train_pct;
    vq->codebook = nullptr;
    vq->sample_size = slabs[0]->shared->sample_size;
    
    for (int i = 0; i < nslabs; ++i) {
        slabs[i]->shared->vq = vq;
        /* lightfield data stores data now */
        slabs[i]->sample_size = 2;          /* ? */
    }
    
}

/* function to initialize memory shuffling routine */
void *lfBeginMemShuffle(LFOps *ops, LFInternOp *op)
{
    if (ops->chain_msk & LF_SET_SLAB)
        fillVQStruct(ops, op);
    
    op->op_func = lfMemShuffle;
    op->op_end = lfEndDefault;
    op->op_update = lfUpdateNoop;
    
    return nullptr;
}

/* function to reshuffle data for VQ compressing */
void lfMemShuffle(LFSlab **slabs, int nslab, const int *pos,
                  Descriptor descr, const void *input, int input_size,
                  void *output, int output_size, void *aux_ptr)
{
    LFSlab *slab = slabs[pos[LF_SLAB_POS]];
    LFShared *sh = slab->shared;
    LFVQCodebook *vq = sh->vq;
    int block_size = (sh->ns * sh->nt * sh->sample_size)
                    << (vq->ubits + vq->vbits);
    
    assert(block_size == input_size);
    assert(block_size == output_size);
    
    /*
     * repack training set so that all samples of the same VQ block
     * sit near one other
     */
    const uchar *src_ptr = static_cast<const uchar*>(input);
    uchar *dst_ptr;
    int smsk = (1 << vq->sbits) - 1;
    int tmsk = (1 << vq->tbits) - 1;
    int tblk_mul = sh->nt >> vq->tbits;
    int sblk, tblk;
    int ssmp, tsmp;
    int tile_shift = vq->ubits + vq->vbits + vq->sbits + vq->tbits;
    int idx;
    // sblk: s所在的block的位序
    // ssmp：s在其block中的位序
    for (int u = 0; u < (1<<vq->ubits); ++u)
        for (int v = 0; v < (1<<vq->vbits); ++v)
            for (int s = 0; s < sh->ns; ++s) {
                sblk = s >> vq->sbits;
                ssmp = s & smsk;
                for (int t = 0; t < sh->nt; ++t) {
                    tblk = t >> vq->tbits;
                    tsmp = t & tmsk;
                    idx  = ((sblk * tblk_mul + tblk) << tile_shift) +
                    (((((u<<vq->vbits)+v) << vq->sbits) + ssmp) << vq->tbits) + tsmp;
                    assert(idx < ((sh->ns * sh->nt) << (vq->ubits + vq->vbits)));
                    dst_ptr = static_cast<uchar*>(output) + sh->sample_size * idx;
                    for (int i = 0; i < sh->sample_size; ++i)
                        *dst_ptr++ = *src_ptr++;
                }
            }
    
}

/* function to generate a codebook */
void lfVQGenCodebook(LFSlab **slabs, int nslab, const int *pos,
                     Descriptor descr, const void *input, int input_size,
                     void *output, int output_size, void *aux_ptr)
{
    LFVQCodebook *vq = slabs[0]->shared->vq;
    
    
    CodeBook *book = vqGenCodebook(input, input_size, vq->sample_size << (vq->ubits+vq->vbits+vq->sbits+vq->tbits), vq->codesize);
    
    vq->codebook = book->codewords;
    vq->codesize = book->code_size;
}

/* function to generate VQ code array */
void lfVQCompress(LFSlab **slabs, int nslab, const int *pos,
                  Descriptor descr, const void *input, int input_size,
                  void *output, int output_size, void *aux_ptr)
{
    
    LFSlab *slab = slabs[pos[LF_SLAB_POS]];
    LFShared *sh = slab->shared;
    LFVQCodebook *vq = sh->vq;
    
    int block_size =
        (sh->sample_size * sh->ns * sh->nt) << (vq->ubits + vq->vbits);
    assert(input_size == block_size);
    
    int last_byte_offset;
    vqEncode(static_cast<const uchar*>(input), input_size,
             vq->sample_size << (vq->ubits + vq->vbits + vq->sbits + vq->tbits),
             static_cast<const uchar*>(vq->codebook), vq->codesize,
             output, last_byte_offset);

    assert(last_byte_offset == output_size);
    
    
    /* 测试图像压缩效果 */
//    using namespace cv;
//    unsigned short *array = static_cast<unsigned short*>(output);
//    uchar *code;
//    short idx;
//    int tblk_mul = (sh->nt >> vq->tbits);
//    Mat image(128, 128, CV_8UC3);
//    for (int s = 0; s != 128; ++s) {
//        int sblk = (s >> vq->sbits);
//        int ssmp = s & ((1<<vq->sbits) - 1);
//        for (int t = 0; t != 128; ++t) {
//            int tblk = (t >> vq->tbits);
//            int tsmp = t & ((1 << vq->tbits) - 1);
//            
//            idx = sblk * tblk_mul + tblk;
//            idx = array[idx];
//            code = (static_cast<uchar*>(vq->codebook) + 48*idx);
//
//            image.at<Vec3b>(s, t)[0] = code[(ssmp << vq->tbits) + tsmp];
//            image.at<Vec3b>(s, t)[1] = code[(ssmp << vq->tbits) + tsmp + 1];
//            image.at<Vec3b>(s, t)[2] = code[(ssmp << vq->tbits) + tsmp + 2];
//   
//        }
//    }
//    
//    namedWindow("image");
//    imshow("image", image);
//    waitKey(0);
}


void lfEndVQCompress(LFOps *ops, LFInternOp *op, void *aux_ptr)
{
    LFShared *shared;
    
    for (int i = 0; i != ops->op_slab_cnt; ++i) {
        shared = ops->op_slabs[i]->shared;
        if (shared->vq)
            shared->sample_size = 2;
    }
}

