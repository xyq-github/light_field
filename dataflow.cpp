//
//  dataflow.cpp
//  rawCompress
//
//  Created by Jacob on 16/2/9.
//  Copyright © 2016年 Jacob. All rights reserved.
//

#include "lightfield.h"
#include <cstdlib>
#include <cassert>

const int UV_POS = 6;
const int VV_POS = 5;

#define FILLOP(id, method, descr, extra, op) { \
(op)->op_id = id;               \
(op)->op_method = method;       \
(op)->op_descr = descr;         \
(op)->op_extra = extra;}

/* post-mortem for processing without need to do anything */
void lfUpdateNoop(LFSlab **slabs, int nslabs, LFInternOp *op)
{
    // 这儿什么代码都没有
}

/* post-mortem for a block-slice processing at a time */
void lfSerialUpdateBlockSlice(LFSlab **slabs, int nslab, LFInternOp *op)
{
    LFVQCodebook *vq = slabs[op->op_pos[LF_SLAB_POS]]->shared->vq;
    int tile_v_msk = (1 << vq->vbits) - 1;
    
    if ((++op->op_pos[LF_V_POS] & tile_v_msk) == 0) {
        op->op_pos[LF_V_POS] = op->op_pos[VV_POS];
        ++op->op_pos[LF_U_POS];
    }
    op->op_output += op->op_output_size;
}

/* uv平面被分割为大小为2^(ubits+vbits)的块；
 * n预计算进入VQ train阶段的slices数目；
 * 最后返回值是2^(ubits+vbits)的倍数，因此根据n要做出向上取整之类的运算
 * 即若2^(ubits+vbits)=16,n=1...16,返回16；n=17...32,返回32
 */
static int slab_train_slices(LFSlab *slab)
{
    LFShared *sh = slab->shared;
    LFVQCodebook *vq = sh->vq;
    int msk = (1 << (vq->ubits + vq->vbits)) - 1;
    int n = sh->nu * sh->nv * vq->train_pct;
    return ((n != 0 ? n : 1) + msk) & ~msk;
}

static int slabs_train_size(LFSlab **slabs, int nslab)
{
    LFShared *sh;
    int total_size = 0;
    
    for (int i = 0; i < nslab; ++i) {
        sh = slabs[i]->shared;
        total_size += sh->ns * sh->nt * sh->sample_size * slab_train_slices(slabs[i]);
    }
    
    return total_size;
}

/*
 * function to handle all stuff related to dataflow of a slab.
 * there is no need to call this function multiple times if all
 * slabs share the same attributes (except the data)
 */
/* 
 * LF_NEW_SLAB标志位设置才会调用此函数；
 * 释放input_alloced，根据op_id设置input_size、input_alloced、output_size、op_loop
 * 分配chain_mem
 */
static void determine_slab_dataflow(LFSlab **slabs, int nslab, LFOps *ops)
{
    /* define input storage, input/output size, loop count of pipe stages */
    LFSlab *slab = slabs[ops->chain_pos[LF_SLAB_POS]];
    LFShared *sh = slab->shared;
    LFVQCodebook *vq = sh->vq;
    LFInternOp *op = ops->chain_ops;
    bool do_block = ops->chain_msk & (LF_GEN_VQ_TRAINSET | LF_GEN_VQ_CODEARRAY);
    
    for (int i = 0; i < ops->chain_cnt; ++i, ++op) {
        if (op->op_input_alloced != nullptr) {
            delete [] static_cast<uchar*>(op->op_input_alloced) ;
            op->op_input_alloced = nullptr;
        }
        
        switch (op->op_id) {
            case LF_OP_SHUFFLE:
                op->op_input_size = (sh->ns * sh->nt * sh->sample_size)
                                    << (vq->ubits + vq->vbits);         /* ? */
                op->op_input_alloced = new uchar[op->op_input_size]();
                op->op_output_size = op->op_input_size;
                op->op_loop = 1;
                break;
                
            case LF_OP_COMPRESS:
                op->op_input_size = (sh->ns * sh->nt * sh->sample_size)
                                    << (vq->ubits + vq->vbits);
                op->op_input_alloced = new uchar[op->op_input_size]();
                op->op_output_size = (sh->ns * sh->nt * sizeof(short))
                                    >> (vq->sbits + vq->tbits);
                op->op_loop = 1;
                break;
                
            case LF_OP_WRITE:
                op->op_input_size = (vq==nullptr) ?
                    (sh->ns * sh->nt * sh->sample_size) :
                    ((sh->ns * sh->nt * sizeof(short)) >> (vq->sbits + vq->tbits));
                if (ops->chain_cnt > 1)
                    op->op_input_alloced = new uchar[op->op_input_size];
                op->op_output_size = -1;        /* DONT'T CARE */
                op->op_loop = 1;
                break;
                
            case LF_OP_READ:
                op->op_input_size = -1;         /* DON'T CARE */
                switch (op->op_method) {
                    case LF_LIGHTFIELD:
                        op->op_output_size = (vq==nullptr) ?
                            (sh->ns * sh->nt * sh->sample_size) :
                            ((sh->ns * sh->nt * sizeof(short))
                                >> (vq->sbits + vq->tbits));
                        op->op_loop =
                            do_block ? (1 << (vq->ubits + vq->vbits)) : 1;
                        break;
                        
                    case LF_SLICE_ST:
                        op->op_output_size = sh->ns * sh->nt * sh->sample_size;
                        op->op_loop = do_block ? (1 << (vq->ubits + vq->vbits)) : 1;
                        break;
                }
                break;
                
            case LF_OP_COMPRESS_TRAIN:
                op->op_input_size = slabs_train_size(slabs, nslab);
                op->op_output_size = (vq->codesize * sh->sample_size) <<             (vq->ubits + vq->vbits + vq->sbits + vq->tbits);
                op->op_loop = 1;
                break;
        }
    }
    
    switch (ops->chain_dst) {
        case LF_DST_SLABMEM:
            if (ops->chain_mem != nullptr)
                delete [] (uchar*)(ops->chain_mem);
            ops->chain_mem = new uchar[vq==nullptr ?
                                      (sh->nu * sh->nv * sh->ns * sh->nt * sh->sample_size) :
                                      ((sh->nu * sh->nv * sh->ns * sh->nt * sizeof(short)) >>
                                       (vq->ubits + vq->vbits + vq->sbits + vq->tbits))];
            break;
            
        case LF_DST_DFMEM:
            if (ops->chain_pos[LF_SLAB_POS] != 0)
                break;
            if (ops->chain_mem != nullptr)
                delete [] (uchar*)(ops->chain_mem);
            ops->chain_mem = new uchar[slabs_train_size(slabs, nslab)];
            break;
    }
}

/* 
 *  初始化op->op_aux，op->op_fun、op->op_update、op->op_end
 * 初始化op->chain_pos
 */
static void init_dataflow(LFOps *ops)
{
    LFInternOp *op = ops->chain_ops;
    
    for (int i = 0; i != ops->chain_cnt; ++i, ++op) {
        switch (op->op_id) {
            case LF_OP_READ:
                if (op->op_method == LF_LIGHTFIELD)
                    op->op_aux = lfBeginReadLif(ops, op);
                else if (op->op_method == LF_SLICE_ST)
                    op->op_aux = lfBeginReadSliceFile(ops, op);
                else {
                    lfError("Unknown method %d\n", op->op_method);
                    exit(8);
                }
                break;
                
            case LF_OP_SHUFFLE:
                op->op_aux = lfBeginMemShuffle(ops, op);
                break;
                
            case LF_OP_COMPRESS_TRAIN:
                op->op_func = lfVQGenCodebook;
                op->op_end = lfEndDefault;
                op->op_update = lfUpdateNoop;
                break;
                
            case LF_OP_COMPRESS:
                op->op_func = lfVQCompress;
                op->op_end = lfEndVQCompress;
                op->op_update = lfUpdateNoop;
                break;
                
            case LF_OP_WRITE:
                op->op_aux = lfBeginWriteLif(ops, op);
                break;
        }
    }
    
    /* initialize position */
    ops->chain_pos[LF_SLAB_POS] =
    ops->chain_pos[LF_U_POS]    =
    ops->chain_pos[LF_V_POS]    =
    ops->chain_pos[LF_S_POS]    =
    ops->chain_pos[LF_T_POS]    =
    ops->chain_pos[UV_POS]	= 0;
}

static void cleanup_dataflow(LFOps *ops)
{
    LFInternOp *op = ops->chain_ops;
    for (int i = 0; i < ops->chain_cnt; ++i) {
        if (op->op_input_alloced != nullptr) {
            delete [] static_cast<uchar*>(op->op_input_alloced);
            op->op_input_alloced = nullptr;
        }
    }
}

/*
 * functions to maintain the underlying mechanism of dataflow
 */
static bool update_dataflow(LFSlab **slabs, int nslab, LFOps *ops)
{
    /* populate position */
    LFInternOp *op = ops->chain_ops;
    /* 将每个子操作op的pos设置为操作链ops的pos */
    for (int i = 0; i < ops->chain_cnt; ++i, ++op) {
        op->op_pos[LF_SLAB_POS] = ops->chain_pos[LF_SLAB_POS];
        op->op_pos[LF_U_POS] = ops->chain_pos[LF_U_POS];
        op->op_pos[LF_V_POS] = op->op_pos[VV_POS] = ops->chain_pos[LF_V_POS];
        op->op_pos[LF_S_POS] = op->op_pos[LF_T_POS] = 0;
    }
    
    /*
     * 每当新slab就确定一下数据、分配内存
     * 达到nslab就返回false退出
     */
    int pos = ops->chain_pos[LF_SLAB_POS];
    if (ops->chain_msk & LF_NEW_SLAB) {
        if (pos == 0)
            determine_slab_dataflow(slabs, nslab, ops);
        else {
            switch (ops->chain_dst) {
                case LF_DST_SLABMEM:
                    /* save lightfield data if needed */
                    slabs[pos-1]->lightfield = ops->chain_mem;
                    ops->chain_mem = nullptr;
                    if (pos >= nslab)   /* termination criterian */
                        return false;
                    determine_slab_dataflow(slabs, nslab, ops);
                    break;
                    
                case LF_DST_SLABVQ:
                    assert(ops->chain_mem != nullptr);
                    delete [] (uchar*)ops->chain_mem;    // free(ops->chain_mem);
                    ops->chain_mem = nullptr;
                    return false;
                    
                default:
                    if (pos >= nslab)
                        return false;
                    if (!(ops->chain_msk & LF_SHARED))
                        determine_slab_dataflow(slabs, nslab, ops);
                    break;
            }
        }
    }
    
    LFSlab *slab = slabs[pos];
    LFShared *sh = slab->shared;
    LFVQCodebook *vq = sh->vq;
    LFInternOp *op_src = &ops->chain_ops[0];
    LFInternOp *op_dst = &ops->chain_ops[ops->chain_cnt-1];
    
    /* trigger source */
    /* 确定第一个InternOp的输入指针 */
    switch (ops->chain_src) {
        case LF_SRC_SLABMEM:
            /* do nothing here */
            break;
            
        case LF_SRC_DFMEN:
            op_src->op_input = (uchar*)ops->chain_mem;
            break;

        case LF_SRC_FILE:
            op_src->op_input = (uchar*)ops->chain_input;
            break;
    }
    
    /* populate output stages (using input of next operators) */
    /* 确定前chain_cnt-1个InternOp的输出指针 */
    op = ops->chain_ops;
    for (int i = 0; i < ops->chain_cnt - 1; ++i, ++op) {
        op->op_output = (op+1)->op_input_alloced;
    }
    
    /* 确定后chain_cnt-1个InternOp的输入指针 */
    op = ops->chain_ops+1;
    for (int i = 1; i < ops->chain_cnt; ++i, ++op) {
        op->op_input = op->op_input_alloced;
    }
    
    /* collect data */
    /* 确定最后一个InternOp的输出指针 */
    switch (ops->chain_dst) {
        case LF_DST_SLABMEM:
            if (ops->chain_msk & LF_NEW_SLAB)
                op_dst->op_output = (uchar*)ops->chain_mem;
            else
                op_dst->op_output += op_dst->op_output_size;
            break;
            
        case LF_DST_DFMEM:
            if (pos==0 && (ops->chain_msk & LF_NEW_SLAB))
                op_dst->op_output = (uchar*)ops->chain_mem;
            else
                op_dst->op_output += op_dst->op_output_size;
            break;
            
        case LF_DST_FILE:
            op_dst->op_output = (uchar*)ops->chain_output;
            break;
            
        case LF_DST_SLABVQ:
            /* do nothing here */
            break;
    }
    
    /* update position */
    ops->chain_msk &= ~LF_NEW_SLAB;
    bool gen_vq_trainset = ops->chain_msk & LF_GEN_VQ_TRAINSET;
    bool gen_vq_codebook = ops->chain_msk & LF_GEN_VQ_CODEBOOK;
    bool gen_vq_codearray = ops->chain_msk & LF_GEN_VQ_CODEARRAY;
    
    if (gen_vq_codebook) {
        lfOutput("train VQ...\n");
        ops->chain_msk |= LF_NEW_SLAB;
        ops->chain_pos[LF_SLAB_POS]++;
    }
    else if (ops->chain_flow == LF_DF_SLAB) {
        lfOutput("dataflow 0x%x: slab %d...\n", ops->chain_msk, slabs[ops->chain_pos[LF_SLAB_POS]]->id);
        ops->chain_msk |= LF_NEW_SLAB;
        ops->chain_pos[LF_SLAB_POS]++;
    }
    else if (gen_vq_trainset) {
        int cnt = slab_train_slices(slab) >> (vq->ubits + vq->vbits);           // SLAB block的数量
        /* mod_uv应该是指uv平面内2^(ubits+vbits)大小的block的个数 */
        int mod_uv = (sh->nu * sh->nv) >> (vq->ubits + vq->vbits);
        int div_v = sh->nv >> vq->vbits;
        int rand_val = random() % mod_uv;
        
        lfOutput("dataflow 0x%x: block [%d, %d, %d]...\n", ops->chain_msk,
                 slabs[ops->chain_pos[LF_SLAB_POS]]->id,
                 ops->chain_pos[LF_U_POS], ops->chain_pos[LF_V_POS]);
        ops->chain_pos[LF_U_POS] = (rand_val / div_v) << vq->ubits;
        ops->chain_pos[LF_V_POS] = (rand_val % div_v) << vq->vbits;
        if (++ops->chain_pos[UV_POS] == cnt) {
            ops->chain_pos[UV_POS] = 0;
            ops->chain_pos[LF_SLAB_POS]++;
            ops->chain_msk |= LF_NEW_SLAB;
        }
    }
    else if (gen_vq_codearray || vq != nullptr) {
        lfOutput("dataflow 0x%x: block [%d, %d, %d]...\n", ops->chain_msk,
                 slabs[ops->chain_pos[LF_SLAB_POS]]->id,
                 ops->chain_pos[LF_U_POS], ops->chain_pos[LF_V_POS]);
        if ((ops->chain_pos[LF_V_POS]+=(1<<vq->vbits)) == sh->nv) {
            ops->chain_pos[LF_V_POS] = 0;
            if ((ops->chain_pos[LF_U_POS]+=(1<<vq->ubits)) == sh->nu) {
                ops->chain_pos[LF_U_POS] = 0;
                ops->chain_pos[LF_SLAB_POS]++;
                ops->chain_msk |= LF_NEW_SLAB;
            }
        }
    }
    else {
        lfOutput("dataflow 0x%x: slice [%d, %d, %d]...\n", ops->chain_msk,
                 slabs[ops->chain_pos[LF_SLAB_POS]]->id,
                 ops->chain_pos[LF_U_POS], ops->chain_pos[LF_V_POS]);
        if (++ops->chain_pos[LF_V_POS] == sh->nv) {
            ops->chain_pos[LF_V_POS] = 0;
            if (++ops->chain_pos[LF_U_POS] == sh->nu) {
                ops->chain_pos[LF_U_POS] = 0;
                ops->chain_pos[LF_SLAB_POS]++;
                ops->chain_msk |= LF_NEW_SLAB;
            }
        }
    }
    
    return true;
}

static void dataflow(LFOps *ops)
{
    /* initialize data flow */
    /* 初始化操作需要的函数指针 */
    /* 初始化op->aux，对于READ操作便是文件信息，SHUFFLE设空指针 */
    /* 初始化ops的pos成员为0 */
    init_dataflow(ops);
    
    /* trigger source to spit out data for the first time */
    update_dataflow(ops->op_slabs, ops->op_slab_cnt, ops);
    
    LFInternOp *op = ops->chain_ops;
    for (int k = 0; ; ) {
        /* loop thougth an operation */
        for (int i = 0; i < op->op_loop; ++i) {
            (*op->op_func)(ops->op_slabs, ops->op_slab_cnt, op->op_pos,
                           op->op_descr, op->op_input, op->op_input_size,
                           op->op_output, op->op_output_size, op->op_aux);
            (*op->op_update)(ops->op_slabs, ops->op_slab_cnt, op);
        }
        
        ++k;    ++op;
        if (k == ops->chain_cnt) {
            /* 换到下一块 slab或者退出 */
            k = 0;    op = ops->chain_ops;
            
            /*
             * keep pumping source data into the data flow stream until 
             * it's time to quit
             */
            if (!update_dataflow(ops->op_slabs, ops->op_slab_cnt, ops))
                break;
        }
    }
    
    /* terminate all ops */
    op = ops->chain_ops;
    for (int k = 0; k < ops->chain_cnt; ++k, ++op)
        (*op->op_end)(ops, op, op->op_aux);
    
    /* clean up leftover of dataflow */
    cleanup_dataflow(ops);
}

void lfEndDefault(LFOps *ops, LFInternOp *op, void *aux_ptr)
{
}

static void set_op_chain(LFOps *ops, int src, int dst, int dataflow,
                         int set_slab, int cnt, void *input, void *output)
{
    LFInternOp *op = ops->chain_ops;
    ops->chain_msk &= ~(LF_GEN_VQ_TRAINSET |
                        LF_GEN_VQ_CODEBOOK |
                        LF_GEN_VQ_CODEARRAY   |
                        LF_SET_SLAB);
    
    /* 设置压缩标志位 */
    int n_true = 0;
    for (int i = 0; i < cnt; ++i, ++op) {
        if (op->op_id == LF_OP_SHUFFLE) {
            ops->chain_msk |= LF_GEN_VQ_TRAINSET;
            n_true++;
        }
        else if (op->op_id == LF_OP_COMPRESS_TRAIN) {
            ops->chain_msk |= LF_GEN_VQ_CODEBOOK;
            n_true++;
        }
        else if (op->op_id == LF_OP_COMPRESS) {
            ops->chain_msk |= LF_GEN_VQ_CODEARRAY;
            if (ops->chain_msk & LF_GEN_VQ_TRAINSET) {
                ops->chain_msk &= ~LF_GEN_VQ_TRAINSET;
                n_true--;
            }
        }
    }
    
    /*
     * cannot do more than one of LF_GEN_VQ_TRAINSET, LF_GEN_VQ_CODEBOOK,
     * LF_GEN_VQ_CODEARRAY in a single stream.
     */
    assert(n_true <= 1);
    
    ops->chain_msk |= set_slab | LF_NEW_SLAB;
    
    ops->chain_src = src;
    ops->chain_dst = dst;
    ops->chain_flow = dataflow;
    ops->chain_cnt = cnt;
    ops->chain_input = input;
    ops->chain_output = output;
}

void lfDataflowAnalysis(LFOps *ops)
{
    const LFOp *op = &ops->ops[0];
    LFInternOp *iop = ops->chain_ops;
    
    if (op[0].op_id == LF_OP_READ
        && op[1].op_id == LF_OP_COMPRESS
        && op[2].op_id == LF_OP_WRITE) {
        /* gather training set from file */
        FILLOP(LF_OP_READ,
               op[0].op_method, op[0].op_descr, op[0].op_extra, &iop[0]);
        FILLOP(LF_OP_SHUFFLE,
               op[1].op_method, op[1].op_descr, op[1].op_extra, &iop[1]);
        /* 数据从文件读到内存 */
        set_op_chain(ops, LF_SRC_FILE, LF_DST_DFMEM,
                     LF_DF_SLICE_ST, LF_SET_SLAB, 2, op[0].op_extra, nullptr);
        dataflow(ops);
        
        /* do training */
        FILLOP(LF_OP_COMPRESS_TRAIN,
               op[1].op_method, op[1].op_descr, op[1].op_extra, &iop[0]);
        set_op_chain(ops, LF_SRC_DFMEN, LF_DST_SLABVQ,
                     LF_DF_SLAB, 0, 1, nullptr, nullptr);
        dataflow(ops);
        
        
        /* do read/compress/write */
        FILLOP(op[0].op_id,
               op[0].op_method, op[0].op_descr, op[0].op_extra, &iop[0]);
        FILLOP(LF_OP_SHUFFLE,
               op[1].op_method, op[1].op_descr, op[1].op_extra, &iop[1]);
        FILLOP(op[1].op_id,
               op[1].op_method, op[1].op_descr, op[1].op_extra, &iop[2]);
        FILLOP(op[2].op_id,
               op[2].op_method, op[2].op_descr, op[2].op_extra, &iop[3]);
        set_op_chain(ops, LF_SRC_FILE, LF_DST_FILE,
                     LF_DF_SLICE_ST, 0, 4, op[0].op_extra, op[2].op_extra);
    }
    
    else if (op->op_id == LF_OP_READ) {
        FILLOP(op[0].op_id,
               op[0].op_method, op[0].op_descr, op[0].op_extra, &iop[0]);
        set_op_chain(ops, LF_SRC_FILE, LF_DST_SLABMEM,
                     LF_DF_SLICE_ST, LF_SET_SLAB, 1, op[0].op_extra, nullptr);
    }
    
    dataflow(ops);
    
}