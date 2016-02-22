//
//  util.cpp
//  utility stuff
//
//  Created by Jacob on 16/2/9.
//  Copyright © 2016年 Jacob. All rights reserved.
//
#include <cstdio>
#include <string>
#include "lightfield.h"
bool lfVerbose = false;

/* Print error messages (if lfVerbose) */
void
lfError(std::string format, ...)
{
    const char *fmt = format.c_str();
    va_list args;
    
    va_start(args, format);
    fprintf(stderr, "Error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

/* Print output messages */
void lfOutput(std::string format, ...)
{
    const char *fmt = format.c_str();
    va_list args;
    
    if (lfVerbose) {
        va_start(args, format);
        vfprintf(stdout, fmt, args);
        va_end(args);
    }
}