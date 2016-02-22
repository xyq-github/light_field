//
//  rmanpp.cpp
//  rmanpp
//
//  Created by Jacob on 16/2/3.
//  Copyright © 2016年 Jacob. All rights reserved.
//

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <stack>

using namespace std;

typedef struct {
    string filename;
    ifstream::pos_type pos;
} stream_point;

int main(int argc, const char * argv[]) {
    if (argc != 2) {
        cerr << "Usage: rmanpp <rib file>" << endl;
        exit(-1);
    }
    
    string world_file = argv[1];
    string cur_file = world_file;
    string line = "";
    ifstream main_stream(cur_file);
    stack< stream_point > streams;
    
    while (true) {
        if (!getline(main_stream, line)) {
            if (streams.size() == 0)
                break;
            
            stream_point sp = streams.top();
            streams.pop();
            cur_file = sp.filename;
            
            main_stream.close();
            main_stream.open(cur_file);
            main_stream.seekg(sp.pos);
        }
        
        if (line.substr(0, 9) == "##Include") {
            string filename;
            istringstream ss(line);
            ss >> filename >> filename;
            
            stream_point sp = { cur_file, main_stream.tellg() };
            streams.push(sp);
            
            main_stream.close();
            cur_file = filename;
            main_stream.open(cur_file);
        }
        else {
            cout << line << endl;
        }
    }
    
    return 0;
}
