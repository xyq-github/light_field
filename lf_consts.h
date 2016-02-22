//
//  lf_consts.h
//  rawCompress
//
//  Created by Jacob on 16/2/9.
//  Copyright © 2016年 Jacob. All rights reserved.
//

#ifndef lf_consts_h
#define lf_consts_h

/* BEGIN: terminator of all descriptions */
const int LF_NULL = 0xffffffff;
/* END: terminator of all descriptions */

/*     BEGIN: methods */
/*         BEGIN: compression methods */
const int LF_VQ = 0X011;
/*         END: compression methods */

/*         BEGIN: read methods */
const int LF_LIGHTFIELD = 0x021;
const int LF_SLICE_ST   = 0x002;
/*         END: read methods */

/*         BEGIN: write methods */
/*	LF_LIGHTFIELD */
/*         END: write methods */

/*         BEGIN: attribute names */
const int LF_SLAB_UV = 0x051;
const int LF_SLAB_ST = 0x052;
/*         END: attribute names */
/*     END: methods */

/*     BEGIN: operator descriptions */
/*         BEGIN: read parameters */
const int LF_READ_SAMPLES_UV = 0x111;
const int LF_READ_SAMPLES_ST = 0x112;
const int LF_READ_FORMAT     = 0x113;
/*         END: read parameters */

/*         BEGIN: VQ compression parameters */
const int LF_VQ_TRAIN_SIZE   = 0x121;
const int LF_VQ_TILESIZE     = 0x122;
const int LF_VQ_CODESIZE     = 0x123;
/*         END: VQ compression parameters */
/*     END: operator descriptions */

/*         BEGIN: data format */
const int LF_RGB   =   0x201;
const int LF_RGBA  =   0x202;
/*         END: data format */

/* BEGINMASK: lightfield indices */
const int LF_SLAB_0   =  0x00000001;
const int LF_SLAB_1   =  0x00000002;
const int LF_SLAB_2   =  0x00000004;
const int LF_SLAB_3   =  0x00000008;
const int LF_SLAB_4   =  0x00000010;
const int LF_SLAB_5   =  0x00000020;
const int LF_SLAB_6   =  0x00000040;
const int LF_SLAB_7   =  0x00000080;
const int LF_SLAB_8   =  0x00000100;
const int LF_SLAB_9   =  0x00000200;
const int LF_SLAB_10  =  0x00000400;
const int LF_SLAB_11  =  0x00000800;
const int LF_SLAB_12  =  0x00001000;
const int LF_SLAB_13  =  0x00002000;
const int LF_SLAB_14  =  0x00004000;
const int LF_SLAB_15  =  0x00008000;
const int LF_SLAB_16  =  0x00010000;
const int LF_SLAB_17  =  0x00020000;
const int LF_SLAB_18  =  0x00040000;
const int LF_SLAB_19  =  0x00080000;
const int LF_SLAB_20  =  0x00100000;
const int LF_SLAB_21  =  0x00200000;
const int LF_SLAB_22  =  0x00400000;
const int LF_SLAB_23  =  0x00800000;
const int LF_SLAB_24  =  0x01000000;
const int LF_SLAB_25  =  0x02000000;
const int LF_SLAB_26  =  0x04000000;
const int LF_SLAB_27  =  0x08000000;
const int LF_SLAB_28  =  0x10000000;
const int LF_SLAB_29  =  0x20000000;
const int LF_MAXSLABS =  0x0000001e;
const int LF_FIELD    =  0x3fffffff;
/* ENDMASK: lightfield indices */


#endif /* lf_consts_h */
