//
//  vq_iface.hpp
//  rawCompress
//
//  Created by Jacob on 16/2/16.
//  Copyright © 2016年 Jacob. All rights reserved.
//

#ifndef vq_iface_h
#define vq_iface_h

typedef unsigned char uchar;

#include <vector>
using namespace std;

typedef struct CodeBook{
    uchar *codewords;
    int code_size;
    int code_width;
    
} CodeBook;


CodeBook* vqGenCodebook(const void *input, const int input_size,
                        const int dimension, const int max_code_size);

void vqEncode(const uchar *images, const int input_size, const int dimension,
              const uchar *codewords, const int code_size, 
               void *output, int &last_byte_offset, const int index_bytes = 2);


#endif /* vq_iface_hpp */
