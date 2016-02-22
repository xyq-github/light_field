//
//  api.cpp
//  the external interface
//
//  Created by Jacob on 16/2/9.
//  Copyright © 2016年 Jacob. All rights reserved.
//

#include <cassert>
#include "lightfield.h"

static LFSlab *allocSlab(int id)
{
    LFSlab *slab = new LFSlab();
    
    slab->id = id;
    slab->lightfield = nullptr;
    slab->shared = nullptr;
    
    return slab;
}

void lfBegin(int id)
{
    LFContext *ctx = __lf_context;
    LFField *field = ctx->field;
    LFOps *ops = &ctx->ops;
    
    assert(ops->op_cnt == 0);
    ops->op_slab_cnt = 0;
    if (id >= (1 << LF_MAXSLABS)) {
        lfError("Illegal slab mask 0x%08x\n", id);
        exit(3);
    }
    
    // 为 slabs 分配内存
    for (int i = 0; i < LF_MAXSLABS; ++i) {
        if (id & (LF_SLAB_0 << i)) {
            if (!field->slabs[i])
                field->slabs[i] = allocSlab(i);
            ops->op_slabs[ops->op_slab_cnt++] = field->slabs[i];
        }
    }
}

static void addOp(int op_id, int method, Descriptor descr, void *extra)
{
    LFOps *ops = &__lf_context->ops;
    LFOp *op;
    
    assert(ops->op_cnt < LF_MAX_OPS);
    
    op = &ops->ops[ops->op_cnt++];
    op->op_id = op_id;
    op->op_method = method;
    op->op_descr = descr;
    op->op_extra = extra;
}

void lfRead(int method, Descriptor descr, std::string filename)
{
    switch (method) {
        case LF_LIGHTFIELD:
            break;
            
        case LF_SLICE_ST:
            break;
            
        default:
            lfError("Unknown read method %d\n", method);
            exit(4);
    }
    
    addOp(LF_OP_READ, method, descr, &filename);
}

void lfCompress(int method, Descriptor descr)
{
    switch (method) {
        case LF_VQ:
            break;
            
        default:
            lfError("Unknown compression method %d\n", method);
            exit(5);
            break;
    }
    
    addOp(LF_OP_COMPRESS, method, descr, nullptr);
}

void lfWrite(int method, Descriptor descr, std::string filename)
{
    switch (method) {
        case LF_LIGHTFIELD:
            break;
            
        default:
            lfError("Unknown write method %d\n", method);
            exit(6);
            break;
    }
    
    addOp(LF_OP_WRITE, method, descr, &filename);
}

void lfEnd(void)
{
    LFOps *ops = &__lf_context->ops;
    
    if (ops->op_cnt == 0)
        return;
    
    /* do parsing */
    lfDataflowAnalysis(ops);
    
    ops->op_cnt = 0;
}