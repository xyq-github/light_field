//
//  tree_struct.cpp
//  rawCompress
//
//  Created by Jacob on 16/2/14.
//  Copyright © 2016年 Jacob. All rights reserved.
//

#include "tree_struct.h"
#include <cmath>
#include <cassert>
#include <limits>
#include <iostream>

TreeNode::TreeNode(const vector<CodeVector*> &codebook,
                   const int dimension,
                   const vector<int> codes,
                   TreeNode *parent)
        : codebook(codebook), dimension(dimension),
        code_index(codes), parent(parent)
{
    left = right = nullptr;
    
    assert(codes.size() >= 1);
    
    if (codes.size() == 1) {
        centroid_1 = nullptr;
        centroid_2 = nullptr;
        hyperplane = nullptr;
        return ;
    }
    
    centroid_1 = new double[dimension]();
    centroid_2 = new double[dimension]();
    hyperplane = new double[dimension + 1]();
    
    /* compute centroid_1 and centroid_2 */
    centroid();
    
    /* compute hyper plane */
    cal_hyperplane();
    
    /* split this node */
    split();
}

TreeNode::~TreeNode()
{
    if (hyperplane != nullptr) {
        delete [] hyperplane;
        delete [] centroid_2;
        delete [] centroid_1;
    }
    
    if (left != nullptr && right != nullptr) {
        delete left;
        delete right;
    }
}

/*
 * function that yields two centroids which best represent code vectors 
 * related to this node. 
 */
void TreeNode::centroid()
{
    assert(centroid_1 != nullptr);
    assert(centroid_2 != nullptr);
    
    if (code_index.size() == 2) {
        int fst = code_index[0];
        int snd = code_index[1];
        for (int d = 0; d != dimension; ++d) {
            centroid_1[d] = codebook[fst]->code[d];
            centroid_2[d] = codebook[snd]->code[d];
        }
        return ;
    }
    
    double *centroid = new double[dimension]();
    
    for (vector<int>::iterator it = code_index.begin();
         it != code_index.end(); ++it)
        for (int d = 0; d != dimension; ++d)
            centroid[d] += codebook[*it]->code[d];
//    for (int d = 0; d != dimension; ++d)
//        for (vector<int>::iterator it = code_index.begin();
//             it != code_index.end(); ++it)
//            centroid[d] += codebook[*it]->code[d];
    
    for (int d = 0; d != dimension; ++d) {
        centroid_1[d] = centroid[d] * (1 + split_para) / code_index.size();
        centroid_2[d] = centroid[d] * (1 - split_para) / code_index.size();
    }
    
    delete [] centroid;
}

/* 
 * this function calculate the hyperplane
 */
void TreeNode::cal_hyperplane()
{
    assert(hyperplane != nullptr);
    
    double *p = centroid_1;
    double *q = centroid_2;
//    double dist = distance(p, q);
    double argment = 0;
    
    for (int d = 0; d != dimension; ++d)
        argment += p[d] * p[d] - q[d] * q[d];
//    argment = argment / dist / 2;
    argment /= 2;
    hyperplane[0] = argment;
    
    for (int d = 0; d != dimension; ++d)
        hyperplane[d + 1] = q[d] - p[d];
}

/*
 * this function compute the inner product between two vectors p and hyperplane q
 */
double TreeNode::inner_product(double *p, double *q)
{
    assert(p != nullptr && q != nullptr);
    double result = q[0];
    for (int d = 0; d != dimension; ++d)
        result += p[d] * q[d+1];
    return result;
}

/*
 * this function splits codes with opposite signs
 */
void TreeNode::split()
{
    vector<int> l_part;
    vector<int> r_part;
    
    for (vector<int>::iterator it = code_index.begin();
         it != code_index.end(); ++it) {
        double *code = codebook[*it]->code;
        double dist = inner_product(code, hyperplane);
        if (dist < 0)
            l_part.push_back(*it);
        else
            r_part.push_back(*it);
        code_dist.push_back(dist);
    }
    
    // 这种情况应该只在元素个数相当少的时候出现
    if (l_part.size() == 0 || r_part.size() == 0) {
        assert(code_index.size() < 8);
        vector<double>::iterator it;
        if (l_part.size() == 0) {
            it = min_element(code_dist.begin(), code_dist.end());
            int idx = (int)(it - code_dist.begin());
            l_part.push_back(idx);
            r_part.erase(r_part.begin() + idx);
        }
        else {
            it = max_element(code_dist.begin(), code_dist.end());
            int idx = (int)(it - code_dist.begin());
            r_part.push_back(idx);
            l_part.erase(l_part.begin() + idx);
        }
    }
    
    assert(l_part.size() > 0 && r_part.size() > 0);
    
    left = new TreeNode(codebook, dimension, l_part, this);
    right = new TreeNode(codebook, dimension, r_part, this);
}


/*
 * this funtions determin if the distance between p and q is greater than threshhold.
 * if so, return -1; else return the distance.
 */
double TreeNode::part_distance(double *p, double *q, double threshhold)
{
    double dist = 0;
    for (int d = 0; d != dimension; ++d) {
        dist += (p[d] - q[d]) * (p[d] - q[d]);
        if (dist > threshhold)
            return -1;
    }
    return dist;
}


/*
 * this fucntion find the approximately optimal nearest neighbor of input vector
 */
int TreeNode::search(const uchar *data)
{
    TreeNode *node = this;
    double *input = new double[dimension]();
    
    // translate data from uchar to double
    for (int d = 0; d != dimension; ++d)
        input[d] = data[d];
    
    // depth-only search
    while (node->left != nullptr && node->right != nullptr) {
        dist_from_input = inner_product(input, node->hyperplane);
        if (dist_from_input < 0)
            node = node->left;
        else
            node = node->right;
    }
    
    // search backward
    assert(node->code_index.size() == 1);
    node->min = node->code_index[0];
    node->min_dist =
        part_distance(input, codebook[node->min]->code, numeric_limits<double>::max());
    
    while (node->parent != nullptr) {
        node->parent->min = node->min;
        node->parent->min_dist = node->min_dist;
        node = node->parent;
        
        /* 
         * search all codewords associated with the node that have a signed distance from the hyperplane with an opposite sign to that of dist_from_input
         */
        /* 这段代码消耗了最主要的时间资源，不清楚注释后对压缩质量的影响 */
//        for (int i = 0; i != node->code_dist.size(); ++i) {
//            if (dist_from_input * code_dist[i] < 0
//                && abs(dist_from_input - code_dist[i]) < node->min_dist) {
//                double dist = part_distance(input, codebook[code_index[i]]->code, node->min_dist);
//                if (dist > 0) {
//                    node->min_dist = dist;
//                    node->min = code_index[i];
//                }
//            }
//        }
    }
    
    delete [] input;
    return node->min;
}
