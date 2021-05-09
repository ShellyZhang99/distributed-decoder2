#include "parallel_decoder.h"
#include "parallel_excl_decoder.h"
#include <iostream>
#include <stdlib.h>

#include <fstream>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <time.h>
using namespace std;
#ifdef _WIN32
#   define DLL_EXPORT __declspec(dllexport)
#else
#   define DLL_EXPORT
#endif

extern "C" DLL_EXPORT Parallel_decoder *Parallel_decoder_ctor();
Parallel_decoder *Parallel_decoder_ctor()
{
    return new Parallel_decoder();
}

extern "C" DLL_EXPORT Parallel_excl_decoder *Parallel_excl_decoder_get(Parallel_decoder *decoder, int i);
Parallel_excl_decoder *Parallel_excl_decoder_get(Parallel_decoder *decoder, int i)
{
    return &decoder->excl_decoder[i];
}

extern "C" DLL_EXPORT  int Parallel_decoder_add_excl_decoder(Parallel_decoder *decoder, char *config_filename, int primary, char* pt_filename)
{
    return 1;
}

extern "C" DLL_EXPORT  int Parallel_decoder_add_excl_decoder2(Parallel_decoder *decoder, char *config_filename, int primary, char* pt_filename)
{
    return decoder->add_excl_decoder(config_filename, primary, pt_filename);
}

extern "C" DLL_EXPORT Parallel_excl_decoder *Parallel_excl_decoder_ctor();
Parallel_excl_decoder *Parallel_excl_decoder_ctor()
{
    return new Parallel_excl_decoder();
}

extern "C" DLL_EXPORT  int Parallel_excl_decoder_decodePara(Parallel_excl_decoder* excl_decoder)
{
    return excl_decoder->decode();
}

extern "C" int decode2(int i)
{
    int cpunum = i/8;
    if(cpunum != 4)
        return 0;
    int peventnum = i%8;
    Parallel_decoder *decoder = Parallel_decoder_ctor();
    string a = "perf.data-aux-idx";
    char num = '0'+cpunum;
    a += num;

    a.append(".bin");
    char* str = (char*)a.data();
    string a2 = "perf-attr-config";
    char *str2 =  (char*)a2.data();
    Parallel_decoder_add_excl_decoder2(decoder, str2, cpunum, str);
    Parallel_excl_decoder *para_decoder = Parallel_excl_decoder_get(decoder, peventnum);
    return  Parallel_excl_decoder_decodePara(para_decoder);
}



extern "C" DLL_EXPORT  int Parallel_excl_decoder_decode(Parallel_excl_decoder* excl_decoder, int i)
{
    ofstream outfile;
    outfile.open("outputFile2.txt");

    outfile<<i<<endl;
    outfile.close();
    return decode2(i);
}


