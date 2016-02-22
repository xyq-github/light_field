/*
 * main.cpp: compress raw image data and write out to a lightfield 
 * in the new file format
 *
 */

#include <iostream>
#include <cstdlib>
#include <string>
#include "lightfield.h"

using namespace std;

void Usage(string progName);
bool CheckPow2(int *array, int first, int last);

int main(int argc, char *argv[])
{
    string basename;
    string destname;
    string progname;
    string suffixPtr;
    
    Descriptor read_descr;
    Descriptor write_descr;
    read_descr.insert(make_pair(LF_NULL, nullptr));
    write_descr.insert(make_pair(LF_NULL, nullptr));
    
    /* vector-quantization default profile */
    bool vqP = true;
    float train_size[1] = { 0.04 };
    int tile_size[4] = { 1, 1, 1, 1 };
    int code_size[1] = { 4000 };
    Descriptor compress_descr;
    compress_descr.insert(make_pair(LF_VQ_TRAIN_SIZE, train_size));
    compress_descr.insert(make_pair(LF_VQ_TILESIZE, tile_size));
    compress_descr.insert(make_pair(LF_VQ_CODESIZE, code_size));
    compress_descr.insert(make_pair(LF_NULL, nullptr));
    
    /* Parse the arguments */
    progname = argv[0];
    argv++;
    argc--;
    
    while (argc > 2) {
        /* -raw: No compression */
        if (string(argv[0]) == "-novq") {
            vqP = false;
            argv++;
            argc--;
        }
        /* -vq: Vector quantization compression */
        else if (string(argv[0]) == "-vq") {
            argv++;
            argc--;
        }
        /* -v: verbose */
        else if (string(argv[0]) == "-v") {
            lfVerbose = true;
            argv++;
            argc--;
        }
        /* -vqtrain:  size of the training set (fraction of entire set) */
        else if (string(argv[0]) == "-vqtrain") {
            float train_size = atof(argv[1]);
            argv += 2;
            argc -= 2;
            if (train_size <= 0) {
                cerr << "Warning: vq training set must be greater than 0.\n";
                train_size = 0.04;
            }
            else if (train_size >= 1) {
                cerr << "Warning: vq training set cannot be greater than 1.\n";
                train_size = 1.0;
            }
            compress_descr[LF_VQ_TRAIN_SIZE] = new float(train_size);
        }
        /* -vqtile: size (dimensions) of the vq tile */
        else if (string(argv[0]) == "-vqtile") {
            if (argc < 5)
                Usage(progname);
            int *tile_size = new int();
            tile_size[0] = atoi(argv[0]);
            tile_size[1] = atoi(argv[1]);
            tile_size[2] = atoi(argv[2]);
            tile_size[3] = atoi(argv[3]);
            argv += 5;
            argc -= 5;
            if (!CheckPow2(tile_size, 0, 3)) {
                cerr << "Warning:  vq tile size must be a power of 2 "
                        "in each dimension.\n";
                exit(1);
            }
            compress_descr[LF_VQ_TILESIZE] = tile_size;
        }
        /* -vqcode: size of the codebook (number of entries */
        else if (string(argv[0]) == "-vqcode") {
            int code_size = atoi(argv[1]);
            argv += 2;
            argc -= 2;
            /* For now, disallow codebooks smaller than 257.
             * The code doesn't handle 1-byte code indices correctly yet.
             */
            if (code_size < 257) {
                cerr << "Warning: vqcodebook too small!\n";
                cerr << "Warning: Current minimum codebook size: 257\n";
                code_size = 257;
            }
            else if (code_size > 65536) {
                cerr << "Warning: vqcodebook is very large. "
                        "This may take a while...\n";
            }
            compress_descr[LF_VQ_CODESIZE] = new int(code_size);
        }
        /* default */
        else {
            Usage(progname);
            exit(1);
        }
    }
    
    /* Now only the two file names should be left */
    if( argc != 2) {
        Usage(progname);
        //exit(2);
    }
    
    /* Ok, now do it. */
    basename = argv[0];
    destname = argv[1];
    
    lfInitContext();
    
    /* Read in the file, VQ compress it */
    lfBegin(LF_FIELD);
    /* SuffixPtr points to last 4 characters of the file name */
    suffixPtr = basename.substr(basename.length() > 4 ? basename.length() - 4 : 0);
    /* Read in the lightfield */
    if (suffixPtr == ".lif" || suffixPtr == ".LIF") {
        cout << "Converting lightfield " << basename << " to " << destname
            << ".\n";
        lfRead(LF_LIGHTFIELD, read_descr, basename);
    }
    else if (suffixPtr == ".lid" || suffixPtr == ".LID") {
        cout << "Converting light field description " << basename << " to "
             << destname << ".\n";
        lfRead(LF_SLICE_ST, read_descr, basename);
    }
    else {
        Usage(progname);
        exit(5);
    }

    /* compress, if required */
    if (vqP) {
        lfCompress(LF_VQ, compress_descr);
    }
    
    /* Write the file out */
    lfWrite(LF_LIGHTFIELD, write_descr, destname);
    lfEnd();
    
    /* Notify user that it succeeded. */
    cout << "\nDone\n";
    
    return 0;
}

/*
 * This function makes sure that every element of the array between
 * first and last (inclusive) is a power of 2, between 1 and 256.
 */
bool CheckPow2(int *array, int first, int last)
{
    bool found = true;
    
    for (int i = first; i != last; ++i) {
        found = false;
        for (int pow = 0; pow <= 8; ++pow) {
            if (array[i] == (1 << pow)) {
                found = true;
                break;
            }
        }
        if (!found)
            return false;
    }
    
    return true;
}

void Usage(string progName)
{
    cerr << "Usage:\n"
        << progName << " [options] <infield> <outfield.lif>\n\n"
        <<"arguments:\n\n"
        "    -v                      Verbose mode on, prints progress.\n"
        "\n"
        "    -vq                     Use vector-quantization compression.\n"
        "                            This is the default. \n"
        "\n"
        "    -novq                   Don't use vector-quantization compression\n"
        "\n"
        "    -vqtrain <fraction>     Specify fraction of lightfield to be used\n"
        "                            as a training set.  The fraction should be\n"
        "                            between 0 and 1.  Default is 0.04.\n"
        "\n"
        "    -vqtile <u> <v> <s> <t> Specify the size of each vq tile.  Each\n"
        "                            of the four numbers should be a fairly\n"
        "                            small power of 2.  Default is 2 2 2 2.\n"
        "\n"
        "    -vqcode <size>          Specify the number of tiles in the\n"
        "                            codebook.  Depending on the tile size,\n"
        "				             good numbers would be between 256 and\n"
        "                            65536.  Default is 16384.\n"
        "\n"
        "    <infield>		         Should be either a .lid file\n"
        "                            (LIght field Description file), which\n"
        "                            specifies a set of .rgb files, or a .lif\n"
        "                            file, which is the native light field\n"
        "                            format.\n"
        "\n"
        "    <outfield.lif>	         The output file.\n"
        "\n";
}
