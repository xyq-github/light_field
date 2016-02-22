//
//  lf_funcs.h
//  rawCompress
//
//  Created by Jacob on 16/2/9.
//  Copyright © 2016年 Jacob. All rights reserved.
//

#ifndef lf_funcs_h
#define lf_funcs_h

#include <string>

void lfBegin(int /*id*/);
void lfCompress(int /*method*/, Descriptor /*compress_descr*/);
void lfEnd();
void lfRead(int /*method*/, Descriptor /*read_descr*/, std::string /*filename*/);
void lfWrite(int /*method*/, Descriptor /*write_descr*/, std::string /*filename*/);
void lfInitContext(void);


#endif /* lf_funcs_h */
