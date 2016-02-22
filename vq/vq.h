//
//  vq.hpp
//  rawCompress
//
//  Created by Jacob on 16/2/13.
//  Copyright © 2016年 Jacob. All rights reserved.
//

#ifndef vq_h
#define vq_h

#include <vector>
#include <list>
#include "tree_struct.h"
#include "vq_iface.h"

using namespace std;

typedef unsigned char uchar;

typedef struct CodeVector {
    double *code;
    vector< int > closest;     // 与code距离最小的数据向量的index
    
    CodeVector(double *ptr) { code = ptr; }
    ~CodeVector() { delete [] code; }
} CodeVector;


class TreeNode;

const double split_para = 0.01;     // 

class VQ
{
private:
    const uchar **train_set;    // 训练数据集
    const int train_bytes;      // 数据的字节大小
    const int train_size;       // 训练集大小（以向量为单位)
    const int dimension;        // 数据的维度
    const int max_code_size;    // 预期的code vector数目
    
    const double episilon = 0.001;
    
    TreeNode *root;             // tree_strcture vq
    
    void init_first_code();
    void centroid(CodeVector *code_vec);
    double dist(const CodeVector *code_vec, int idata);
    
public:
    vector< CodeVector* > codebook;

    VQ(const void *input, const int bytes,
       const int dim, const int max_code);
    VQ(const uchar *codewords, const int dimension, const int code_size);
    
    
    void train();
    int search(const uchar *input);
  
    /* friend class */
    friend class TreeNode;
};


#endif /* vq_h */
