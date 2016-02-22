//
//  tree_struct.hpp
//  rawCompress
//
//  Created by Jacob on 16/2/14.
//  Copyright © 2016年 Jacob. All rights reserved.
//

#ifndef tree_struct_h
#define tree_struct_h


#include "vq.h"
#include <vector>

using namespace std;

typedef unsigned char uchar;

class VQ;
struct CodeVector;

class TreeNode
{
private:    
    TreeNode *left;
    TreeNode *right;
    TreeNode *parent;
    
    vector< int > code_index;           // 此节点包含的code的为序（codebook中）
    vector< double > code_dist;         // code到hyperplane的有向距离
    double *centroid_1;                 /* centroid_1 与 centroid_2 */
    double *centroid_2;                 /* 是此节点code们生成的两个中心 */
    double *hyperplane;                 //
    
    int min;                            // 加密时向parent传递的最短code
    double min_dist;                    // 加密时向parent传递的最小距离
    
    const vector<CodeVector*> &codebook;
    const int dimension;
    
    void centroid();
    double inner_product(double *, double *);
    void cal_hyperplane();
    void split();                       // 根据有向距离正负分离codes
  
    double dist_from_input;             // distance from input to hyperplane
    double part_distance(double *, double *, double);
    
public:
    ~TreeNode();
    TreeNode(const vector<CodeVector*> &codebook, const int dimension,
             const vector<int> codes, TreeNode *parent);
    int search(const uchar *);
};




#endif /* tree_struct_hpp */
