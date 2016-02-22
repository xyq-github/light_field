//
//  lightfield.h
//  rawCompress
//
//  Created by Jacob on 16/2/8.
//  Copyright © 2016年 Jacob. All rights reserved.
//

#ifndef lightfield_h
#define lightfield_h

#include <map>
#include <string>
#include "lf_consts.h"

typedef unsigned char uchar;

/* definitions related to different lightfield operations */
// const int LF_OP_DRAW 0
// const int LF_OP_CREATE 1
const int LF_OP_COMPRESS = 2;
const int LF_OP_WRITE = 3;
const int LF_OP_READ = 4;
const int LF_OP_MIN = LF_OP_COMPRESS;
const int LF_OP_MAX = LF_OP_READ;
const int LF_OP_COMPRESS_TRAIN = 5;
const int LF_OP_SHUFFLE = 7;

/* definitions related to data flow of an internal operator chain */
const int LF_SRC_SLABMEM = 0;
const int LF_SRC_FILE = 1;
const int LF_SRC_DFMEN = 3;

const int LF_DST_SLABMEM = 0;
const int LF_DST_FILE = 1;
const int LF_DST_SLABVQ = 2;
const int LF_DST_DFMEM = 4;

const int LF_DF_COMPRESS_WRITE = 0X0004;
const int LF_DF_READ_COMPRESS  = 0X0008;
const int LF_DF_READ_COMPRESS_WRITE = 0X0020;
const int LF_DF_COMPRESS = 0X0100;
const int LF_DF_WRITE = 0X0200;
const int LF_DF_READ = 0X0400;
const int LF_DF_READ_COMPRESS_TRAIN = 0X1000;
const int LF_DF_COMPRESS_TRAIN = 0X2000;

const int LF_DF_FIELD = 0;
const int LF_DF_SLAB = 1;
const int LF_DF_SLICE_ST = 2;

/* handy aliases for addressing dataflow position array */
const int LF_SLAB_POS = 0;
const int LF_U_POS = 1;
const int LF_V_POS = 2;
const int LF_S_POS = 3;
const int LF_T_POS = 4;


typedef struct {
    short u, v, s, t;
} LFPixelUVST;

typedef struct {
    unsigned char a, b, g, r;
} LFPixelRGBA8;

typedef struct {
    float ox, oy, oz, ow;
	// float x, y, z, w;
    float s, t;
} LFVtx;

typedef struct {
	int id;
	int codesize;
	int ubits, vbits, sbits, tbits;
	// int intern_format;
	float train_pct;
	void *codebook;
	// int type;
	// const int LF_VQ_LINEAR_DATA = 0X0;
	// const int LF_VQ_LINEAR_VQ = 0X1;
	// const int LF_VQ_TREE_VQ = 0X2;
	int sample_size;
} LFVQCodebook;

/*
 * the following structure contains information that might be 
 * shared by multiple slabs ( for example, all slabs in a lightfield).
 */
typedef struct {
    /* current dimensions of u, v, s, t */
    int nu, nv, ns, nt;
	// int u_frac_bits, v_frac_bits, s_frac_bits, t_frac_bits;
	// int *uindex, *vindex, *sindex, *tindex;
	
	/* how (u, v, s, t) is translated into tile indices */
	// int *vqtile_uindex, *vqtile_vindex, *vqtile_sindex, *vqtile_tindex;
	LFVQCodebook *vq;
	// int intern_format;
	int sample_size;
	// int flag;
	// const int LF_SLAB_VALID_AUX = 0X1;
} LFShared;

typedef struct {
    int id;
    LFVtx uv[4];
    LFVtx st[4];
    void *lightfield;
    int sample_size;
	LFShared *shared;
} LFSlab;

typedef struct LFField{
    LFSlab *slabs[LF_MAXSLABS];
    int nslabs;
	// unsigned enable_msk;
    LFShared *shared;
    int id;
    LFField *next;
} LFField;

typedef std::map<int, void *> Descriptor;

typedef struct {
    int op_id;
    int op_method;
    Descriptor op_descr;
    void *op_extra;
} LFOp;

const int LF_MAX_OPS = 10;
const int LF_MAX_INTERNOPS = LF_MAX_OPS;

struct LFOps;

typedef struct LFInternOp{
    int op_id;
    int op_method;
    Descriptor op_descr;
    void *op_extra;
    
    /* really an operator on the data flow */
    void (*op_func)(LFSlab ** /*slabs*/, int /*nslab*/, const int * /*pos*/,
                    Descriptor /*descr*/, const void * /*input*/, int /*input_size*/,
                    void * /*output*/, int /*output size*/, void * /*aux_ptr*/);
    
    /* function to call after data flow ends */
    void (*op_end)(LFOps * /*ops*/, LFInternOp * /*op*/, void * /*aux_ptr*/);
    
    /*
     * update function for next loop, this hides the internal looping and
     * I/O machanism of dataflow from the operator
     */
    void (*op_update)(LFSlab ** /*slabs*/, int /*nslabs*/, LFInternOp * /*op*/);
    int op_loop;
    int op_pos[7];
    
    /* pre_determined I/O memory and size for data flow of each operator */
    uchar *op_input_alloced;
    uchar *op_input;
    int op_input_size;
    uchar *op_output;
    int op_output_size;
    
    /* auxiliary data structure hanging off an internal operator */
    void *op_aux;
} LFInternOp;

/* stuff related to lfBegin/lfEnd */
typedef struct LFOps{
    LFSlab *op_slabs[LF_MAXSLABS];
    int op_slab_cnt;
    
    /* stuff describing an external operator chain */
    LFOp ops[LF_MAX_OPS];
    int op_cnt;
    
    /* stuff describing an internal operator chain */
    int chain_src;
	int chain_dst;
	int chain_flow;
	// int chain_type;
	LFInternOp chain_ops[LF_MAX_INTERNOPS];
	int chain_cnt;
	int chain_pos[7];
	void *chain_input;
	void *chain_output;
	void *chain_mem;
	int chain_msk;
} LFOps;

const int LF_SHARED = 0X01;
const int LF_GEN_VQ_TRAINSET = 0X02;
const int LF_GEN_VQ_CODEARRAY = 0X04;
const int LF_SET_SLAB = 0X08;
const int LF_NEW_SLAB = 0X10;
const int LF_GEN_VQ_CODEBOOK = 0X20;


typedef struct {
	LFOps ops;
	LFField *field;
	LFField *lf_list;
} LFContext;

// bool lfVerbose;
extern LFContext *__lf_context;
extern bool lfVerbose;


#include "lf_funcs.h"

/* functions defined in other files */
void lfError(std::string format, ...);
void lfOutput(std::string format, ...);
void lfDataflowAnalysis(LFOps *ops);
void *lfBeginReadLif(LFOps * /*ops*/, LFInternOp * /*op*/);


void *lfBeginReadSliceFile(LFOps * /*ops*/, LFInternOp * /*op*/);
void lfReadSliceFile(LFSlab ** /*slabs*/, int /*nslab*/, const int * /*pos*/,
					Descriptor /*descr*/, const void * /*input*/, int /*input_size*/,
					void * /*output*/, int /*output_size*/, void * /*aux_ptr*/);
void lfEndReadSliceFile(LFOps * /*ops*/, LFInternOp * /*op*/, void * /*aux_ptr*/);


void lfSerialUpdateBlockSlice(LFSlab ** /*slabs*/, int /*nslabs*/,
							  LFInternOp * /*op*/);
void lfUpdateNoop(LFSlab ** /*slabs*/, int /*nslabs*/, LFInternOp * /*op*/);

void *lfBeginMemShuffle(LFOps * /*ops*/, LFInternOp * /*op*/);
void lfMemShuffle(LFSlab ** slabs, int nslab, const int *ops,
				  Descriptor descr, const void *input, int input_size,
				  void *output, int output_size, void *aux_ptr);
void lfEndDefault(LFOps *ops, LFInternOp *op, void *aux_ptr);

void lfVQGenCodebook(LFSlab **slabs, int nslab, const int *pos,
					Descriptor descr, const void *input, int input_size,
					 void *output, int output_size, void *aux_ptr);

void lfVQCompress(LFSlab **slabs, int nslab, const int *pos,
				  Descriptor descr, const void *input, int input_size,
				  void *output, int output_size, void *aux_ptr);
void lfEndVQCompress(LFOps *ops, LFInternOp *op, void *aux_ptr);


void *lfBeginWriteLif(LFOps *ops, LFInternOp *op);
void lfWriteLif(LFSlab **slabs, int nslab, const int *pos,
				Descriptor descr, const void *input, int input_size,
				void *output, int output_size, void *aux_ptr);
void lfEndWriteLif(LFOps *ops, LFInternOp *op, void *aux_ptr);

void *lfBeginReadLif(LFOps *ops, LFInternOp *op);
void lfReadLif(LFSlab **slabs, int nslab, const int *pos,
				Descriptor descr, const void *input, int input_size,
				void *output, int output_size, void *aux_ptr);
void lfEndReadLif(LFOps *ops, LFInternOp *op, void *aux_ptr);

#endif /* lightfield_h */
