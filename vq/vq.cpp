//
//  vq.cpp
//  rawCompress
//
//  Created by Jacob on 16/2/13.
//  Copyright © 2016年 Jacob. All rights reserved.
//

#include "vq.h"
#include <cassert>
#include <numeric>
#include <limits>
#include <iostream>

VQ::VQ(const void *input, const int bytes, const int dim, const int max_code)
      : train_bytes(bytes), dimension(dim),
        max_code_size(max_code), train_size(bytes / dim),
        train_set(new const uchar*[bytes / dim])
{
    // initialize train_set from input
    // train_set[i]分别指向input中第i个vector
    const uchar *in = static_cast<const uchar*>(input);
    for (int i = 0; i < train_size; ++i)
        train_set[i] = in + i * dimension;
    
    codebook.reserve(max_code);
    
    init_first_code();
}

/*
 * This function computes the centroid of some vectors from beg to end
 * whose indices are stored in closest,
 * and stores it into code_vec
 */
void VQ::centroid(CodeVector *code_vec)
{
    // icode refer to the index of the code vector in codebook
    // beg and end are iterators from icode.closest
    
    vector<int>::iterator beg = code_vec->closest.begin();
    vector<int>::iterator end = code_vec->closest.end();

    assert(code_vec->code != nullptr);
    
    // initialize
    for (int i = 0; i != dimension; ++i)
        code_vec->code[i] = 0;
    
    for (vector<int>::iterator it = beg; it != end; ++it)
        for (int i = 0; i != dimension; ++i)
            code_vec->code[i] += train_set[*it][i];
    
    for (int i = 0; i != dimension; ++i)
        code_vec->code[i] /= code_vec->closest.size();
}



/* 
 * This function computes the distance from code_vector to
 * input vector indicated by idata.
 * The distance is returned if run successfully.
 */
double VQ::dist(const CodeVector *code_vec, int idata)
{
    assert(code_vec->code != nullptr);
    assert(idata < train_size);
    
    double dist = 0;
    for (int i = 0; i != dimension; ++i)
        dist += (code_vec->code[i] - train_set[idata][i]) *
                (code_vec->code[i] - train_set[idata][i]);
    
//    dist /= dimension;
    return dist;
}

void VQ::init_first_code()
{
    CodeVector *init_code = new CodeVector(new double[dimension]);
    
    // 将code的closest设置为全部training vector
    init_code->closest.reserve(train_size);
    
    for (int i = 0; i < train_size; ++i)
        init_code->closest.push_back(i);
    
    // compute the centroid of all input vector and store it into init_code
    centroid(init_code);

    codebook.push_back(init_code);
}

/*
 * this function generate a linear code book stored into codebook.
 * the method used here is known as LBG algorithm with initial guess 
 * of code book by spitting.
 */
void VQ::train()
{
    int N = 1;
    double avg_dist_prv = 0;
    double avg_dist;
    double error;
    int old_N;
    
    // initialize avg_dist_prv
    for (vector<int>::iterator it = codebook[0]->closest.begin();
         it != codebook[0]->closest.end(); ++it)
        avg_dist_prv += dist(codebook[0], *it);
    
    // main loop
    while (N <= max_code_size) {
        cout << "N = " << N << " ... ";

        assert(N == codebook.size());
  
        /* first stage: splitting current code vectors with parameter = 0.1 */
        for (int i = 0; i != N; ++i)
            codebook.push_back(new CodeVector(new double[dimension]));
        for (int i = 1; i <= N; ++i) {
            double *src = codebook[i - 1]->code;
            double *dst = codebook[N + i - 1]->code;
            for (int j = 0; j != dimension; ++j) {
                dst[j] = src[j] * (1 - split_para);
                src[j] *= (1 + split_para);
            }
        }
        
        /* double code vector numbers */
        old_N = N;
        N *= 2;
        
        /* iteration to achieve optimal results */
        error = 2 * episilon;
        while (error > episilon)
        {
            /* second stage: find the neatest code vector for all data vectors */
            for (int i = 0; i != N; ++i) {
                codebook[i]->closest.clear();
            }
            
             /* 构造树状codebook加速查找nearest neighbor速度 */
            vector<int> codes;
            for (int i = 0; i != codebook.size(); ++i)
                codes.push_back(i);
            root = new TreeNode(codebook, dimension, codes, nullptr);
            
            /* 寻找nearest neighbor 占用了大多数的时间 */
            for (int i = 0; i < train_size; ++i) {
//                double min_dist = dist(codebook[0], i);
//                int min = 0;
//                for (int j = 1; j < N; ++j)
//                    if (dist(codebook[j], i) < min_dist) {
//                        min_dist = dist(codebook[j], i);
//                        min = j;
//                    }
                int min = root->search(train_set[i]);
                codebook[min]->closest.push_back(i);
            }
            
            delete root;
            
            // 删除空的code vector
            // 注意删除元素导致迭代器失效
            vector<CodeVector*>::iterator it = codebook.begin();
            while (it != codebook.end()) {
                if ((*it)->closest.size() == 0) {
                    (*it)->~CodeVector();
                    it = codebook.erase(it);
                    N--;
                }
                else
                    ++it;
            }
            
            if (N < old_N)
                return;
            
            /* thrid stage: compute centroid for N partitions and udpate all code vectors */
            for (int i = 0; i < N; ++i) {
                centroid(codebook[i]);
            }
            
            /* forth stage: compute average distortion */
            avg_dist = 0.0;
            for (int i = 0; i != N; ++i) {
                for (vector<int>::iterator it = codebook[i]->closest.begin();
                     it != codebook[i]->closest.end(); ++it) {
                    avg_dist += dist(codebook[i], *it);
                }
            }
            
            error = (avg_dist_prv - avg_dist) / avg_dist_prv;
            avg_dist_prv = avg_dist;
        }
    }
}


VQ::VQ(const uchar *codewords, const int dimension, const int code_size)
        : train_set(nullptr), train_bytes(0), train_size(0),
            dimension(dimension),
            max_code_size(code_size)
{
    codebook.reserve(max_code_size);
    for (int i = 0; i != max_code_size; ++i) {
        CodeVector *code_vec = new CodeVector(new double[dimension]);
        codebook.push_back(code_vec);
        for (int d = 0; d != dimension; ++d)
            code_vec->code[d] = codewords[i*dimension + d];
    }

    
    vector<int> codes;
    for (int i = 0; i != codebook.size(); ++i)
        codes.push_back(i);
    root = new TreeNode(codebook, dimension, codes, nullptr);
}

int VQ::search(const uchar *input)
{
    return root->search(input);
}
