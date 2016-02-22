//
//  lifview.cpp
//  View a light field.
//
//  Created by Jacob on 16/2/21.
//  Copyright © 2016年 Jacob. All rights reserved.
//

#include "lightfield.h"
//#include <cstdlib>
#include <map>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    lfVerbose = true;
    
    Descriptor read_desc;
    read_desc.insert(make_pair(LF_NULL, nullptr));
    
    string basename = argv[1];
    cout << "Reading light field " << basename << endl;
    
    lfInitContext();
    
    lfBegin(LF_FIELD);
    lfRead(LF_LIGHTFIELD, read_desc, basename);
    lfEnd();
}