//
//  lif_api.cpp
//  API to read lid files
//
//  Created by Jacob on 16/2/9.
//  Copyright © 2016年 Jacob. All rights reserved.
//

#include "lightfield.h"
#include "lidquery.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include "opencv2/opencv.hpp"
using namespace std;

LFSlab *__lf_descr_slabs;

typedef struct {
    std::string slab_names[LF_MAXSLABS];
    int nslab;
} LFSliceHeader;

static void fillSlabDefault(LFInternOp *op)
{
    int u_size = 16;
    int v_size = u_size;
    int s_size = 256;
    int t_size = s_size;
    int sample_size = 3;
    Descriptor descr = op->op_descr;
    
    if (descr.size() != 1) {
        if (descr.count(LF_READ_SAMPLES_UV) != 0) {
            int *uv_sample = reinterpret_cast<int*>(descr[LF_READ_SAMPLES_UV]);
            u_size = *uv_sample++;
            v_size = *uv_sample;
        }
        if (descr.count(LF_READ_SAMPLES_ST) != 0) {
            int *st_sample = reinterpret_cast<int*>(descr[LF_READ_SAMPLES_ST]);
            s_size = *st_sample++;
            t_size = *st_sample;
        }
        if (descr.count(LF_READ_FORMAT) != 0) {
            int *format = reinterpret_cast<int*>(descr[LF_READ_FORMAT]);
            if (*format == LF_RGB)
                sample_size = 3;
            else if (*format == LF_RGBA)
                sample_size = 4;
            else
                exit(10);
        }
    }
    
    LFShared *shared;
    for (int i = 0; i != LF_MAXSLABS; ++i) {
        shared = __lf_descr_slabs[i].shared;
        shared->sample_size = sample_size;
        shared->nu = u_size;
        shared->nv = v_size;
        shared->ns = s_size;
        shared->nt = t_size;
    }
}

bool parseLidFile(LFSlab *__lf_descr_slabs, std::string lid_fnm)
{
    int slab_count;
    int slab_id;
    std::string filepath;
    float uv[4][6];
    float st[4][6];
    int sample_uv[2];
    int sample_st[2];
    string format;
    
    bool pass = true;
    LFSlab *cur_slab;
    LFVtx *cur_vtx;
    
    pass &= lidSLAB_COUNT(slab_count, lid_fnm);
    
    for (int i = 0; i != slab_count; ++i) {
        /* read slab counts */
        pass &= lidSLAB_ID(slab_id, lid_fnm, i);
        /* read each slab basename */
        pass &= lidSLAB_BASENAME(filepath, lid_fnm, i);
        /* read vertices u, v */
        pass &= lidSLAB_UV(uv, lid_fnm, i);
        /* read vertices s, t */
        pass &= lidSLAB_ST(st, lid_fnm, i);
        /* read u, v samples */
        pass &= lidSAMPLES_UV(sample_uv, lid_fnm, i);
        /* read s, t samples */
        pass &= lidSAMPLES_ST(sample_st, lid_fnm, i);
        /* read format 3 for RGB and 4 for RGBA */
        pass &= lidFORMAT(format, lid_fnm, i);
        
        /* fill in slabs */
        cur_slab = &__lf_descr_slabs[slab_id];
        /* fill in basename */
        cur_slab->lightfield = new string(filepath);
        /* fill in u, v vertices */
        for (int i = 0; i != 4; ++i) {
            cur_vtx = &cur_slab->uv[i];
            cur_vtx->ox = uv[i][0];
            cur_vtx->oy = uv[i][1];
            cur_vtx->oz = uv[i][2];
            cur_vtx->ow = uv[i][3];
            cur_vtx->s = uv[i][4];
            cur_vtx->t = uv[i][5];
        }
        /* fill in s, t vertices */
        for (int i =0; i != 4; ++i) {
            cur_vtx = &cur_slab->st[i];
            cur_vtx->ox = st[i][0];
            cur_vtx->oy = st[i][1];
            cur_vtx->oz = st[i][2];
            cur_vtx->ow = st[i][3];
            cur_vtx->s = st[i][4];
            cur_vtx->t = st[i][5];
        }
        /* fill in u, v, s, t samples */
        cur_slab->shared->nu = sample_uv[0];
        cur_slab->shared->nv = sample_uv[1];
        cur_slab->shared->ns = sample_st[0];
        cur_slab->shared->nt = sample_st[1];
        /* fill in sample size */
        cur_slab->sample_size = (format == "RGB") ? 3 : 4;
    }
    
    return pass;
}

string DirName(string filename)
{
    size_t found;
    found = filename.find_last_of("/\\");
    return found == -1 ? "." : filename.substr(0, found);
}

/*
 * function to begin inputing all slice files in a lightfield
 */
/*
 * 暂时不使用lex & yyac机制，而是用lidquery模块
 */
void *lfBeginReadSliceFile(LFOps *ops, LFInternOp *op)
{
    __lf_descr_slabs = new LFSlab[LF_MAXSLABS]();
    for (int i = 0; i < LF_MAXSLABS; ++i)
        __lf_descr_slabs[i].shared = new LFShared();
    
    /* fill in the defaults LFShared */
    fillSlabDefault(op);
    
    /* determine available slabs */
    std::string lid_fnm = *reinterpret_cast<std::string*>(op->op_extra);
    if (!parseLidFile(__lf_descr_slabs, lid_fnm))
        exit(13);
    
    /* create a file header to access slab slices */
    string dir = DirName(lid_fnm);
    LFSlab *slab;
    LFSliceHeader *hdr = new LFSliceHeader();
    ostringstream fnm;
    for (int i = 0; i != LF_MAXSLABS; ++i) {
        slab = &__lf_descr_slabs[i];
        if (slab->lightfield == nullptr)
            continue;
        string s = *((string*)slab->lightfield);
        if (s[0] == '/')
            fnm << s;
        else
            fnm << dir << '/' << s;
        hdr->slab_names[i] = fnm.str();
        // 我加的
        hdr->nslab++;
        //
        fnm.str("");
        delete (string*)slab->lightfield;
        slab->lightfield = nullptr;
    }
    
    /* determine what slabs are operable */
    int n = 0;
    if (ops->chain_msk & LF_SET_SLAB) {
        for (int i = 0; i != ops->op_slab_cnt; ++i) {
            int id = ops->op_slabs[i]->id;
            if (hdr->slab_names[i].empty())
                continue;
            if (ops->op_slabs[i]->shared) {
                if (ops->op_slabs[i]->shared->vq)
                    delete ops->op_slabs[i]->shared->vq;
                delete ops->op_slabs[i]->shared;
            }
            *ops->op_slabs[i] = __lf_descr_slabs[id];
            ops->op_slabs[i]->id = id;
            ops->op_slabs[n++] = ops->op_slabs[i];
        }
        ops->op_slab_cnt = n;
    }
    delete [] __lf_descr_slabs;
    
    /* fill in functions */
    op->op_func = lfReadSliceFile;
    op->op_end  = lfEndReadSliceFile;
    op->op_update =
        ops->chain_msk & (LF_GEN_VQ_TRAINSET | LF_GEN_VQ_CODEARRAY) ?
        lfSerialUpdateBlockSlice : lfUpdateNoop;
    
    return hdr;
}

/*
 * functions to input a slice file
 * 由opencv协助完成
 */
void lfReadSliceFile(LFSlab **slabs, int nslabs, const int *pos,
                     Descriptor descr, const void *input, int input_size,
                     void *output, int output_size, void *aux_ptr)
{
    using namespace cv;
    Mat image;
    LFSliceHeader *hdr = reinterpret_cast<LFSliceHeader*>(aux_ptr);
    LFSlab *slab = slabs[pos[LF_SLAB_POS]];
    LFShared *sh = slab->shared;
    string slice_name;
    
    ostringstream ss;
    ss << hdr->slab_names[slab->id] << "." << pos[LF_U_POS] << "." << pos[LF_V_POS]
        << ".tif";
    slice_name = ss.str();
    ss.str("");
    
    image = imread(slice_name);
    if (image.data == nullptr) {
        lfError("lfReadSliceFile: cannot read %s\n", slice_name.c_str());
        exit(14);
    }
   
    /* 检查图像实际大小与内存数据是否匹配 */
    int zsize = image.channels();
    int xsize = image.rows;
    int ysize = image.cols;
    assert(zsize == 3 || zsize == 4);
    assert(xsize == sh->ns);
    assert(ysize == sh->nt);
    
    /* 往内存中读入像素信息 */
    /* 图像一行一行在内存中存放 */
    uchar *pp = reinterpret_cast<uchar*>(output);
    if (sh->sample_size == 3) {
        for (int i = 0; i < xsize; ++i) {
            for (int j = 0; j < ysize; ++j) {
                Vec3b pixel = image.at<Vec3b>(i, j);
                *pp++ = pixel[0];
                *pp++ = pixel[1];
                *pp++ = pixel[2];
            }
        }
    }
    else {
        for (int i = 0; i < xsize; ++i) {
            for (int j = 0; j < ysize; ++j) {
                Vec4b pixel = image.at<Vec4b>(i, j);
                *pp++ = pixel[0];
                *pp++ = pixel[1];
                *pp++ = pixel[2];
                *pp++ = pixel[3];
            }
        }
    }
    
//     测试代码
//    pp = reinterpret_cast<uchar*>(output);
//    Mat test_im(xsize, ysize, CV_8UC3);
//    for (int i = 0; i < xsize; ++i)
//        for (int j = 0; j < ysize; ++j) {
//            test_im.at<Vec3b>(i, j)[0] = *pp++;
//            test_im.at<Vec3b>(i, j)[1] = *pp++;
//            test_im.at<Vec3b>(i, j)[2] = *pp++;
//        }
//    namedWindow("Image");
//    imshow("Image", test_im);
//    waitKey(0);
}

/*
 * function to end inputing all slice files in a lightfield
 */
void lfEndReadSliceFile(LFOps *ops, LFInternOp *op, void *aux_ptr)
{
    LFSliceHeader *hdr = static_cast<LFSliceHeader*>(aux_ptr);
    
    delete hdr;
    hdr = nullptr;
    
}