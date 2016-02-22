//
//  vq_iface.cpp
//  rawCompress
//
//  Created by Jacob on 16/2/16.
//  Copyright © 2016年 Jacob. All rights reserved.
//

#include "vq_iface.h"
#include "vq.h"
#include <iostream>
#include <cassert>
using namespace std;

/*
 * turn a double value to the nearest unsigned char nubmer
 */
inline uchar roundoff(double input)
{
    return static_cast<uchar>(input + 0.5);
}

CodeBook* vqGenCodebook(const void *input, const int input_size,
                        const int dimension, const int max_code_size)
{
    CodeBook *book = new CodeBook;
    
    VQ *vq = new VQ(input, input_size, dimension, max_code_size);
    vq->train();
    
    book->code_size = static_cast<int>(vq->codebook.size());
    book->code_width = dimension;
    book->codewords = new uchar[book->code_size * book->code_width]();
    
    for (int i = 0; i != book->code_size; ++i) {
        for (int d = 0; d != dimension; ++d)
            book->codewords[i*dimension + d] = roundoff(vq->codebook[i]->code[d]);
    }
    
    delete vq;
    return book;
}

void vqEncode(const uchar* images, const int input_size, const int dimension,
              const uchar *codewords, const int code_size,
              void *output, int &last_byte_offset, const int index_bytes)
{
    VQ vq(codewords, dimension, code_size);
    
    assert(index_bytes == 2);        // the index type is currently set to short
    unsigned short *code_array = static_cast<unsigned short*>(output);
    int vectorp = 0;
    last_byte_offset = 0;
    int index;
    
    while (vectorp != input_size) {
        index = vq.search(images + vectorp);
        
        *code_array++ = static_cast<short>(index);

        vectorp += dimension;
        last_byte_offset += index_bytes;
    }
    
    assert(vectorp == input_size);
}