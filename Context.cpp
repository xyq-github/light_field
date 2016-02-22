//
//  Context.cpp
//  stuff related to lightfield context
//
//  Created by Jacob on 16/2/9.
//  Copyright © 2016年 Jacob. All rights reserved.
//
#include "lightfield.h"

LFContext *__lf_context;

static LFField *allocField(int id)
{
    LFField *field = new LFField();
    field->id = id;
    return field;
}

void lfInitContext(void)
{
    /* allocate memory */
    __lf_context = new LFContext();
    
    /* initialize default lightfield */
    __lf_context->lf_list = __lf_context->field = allocField(0);
}