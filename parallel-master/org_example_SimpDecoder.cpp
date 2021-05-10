#include "parallel_decoder.h"
#include "parallel_excl_decoder.h"
#include "org_example_SimpDecoder.h"
#include <iostream>
#include <stdlib.h>

#include <fstream>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <time.h>
using namespace std;
#ifdef __cplusplus
extern "C" {
#endif

Parallel_decoder *Parallel_decoder_ctor()
{
    return new Parallel_decoder();
}
Parallel_excl_decoder *Parallel_excl_decoder_get(Parallel_decoder *decoder, int i)
{
    return &decoder->excl_decoder[i];
}

extern "C" int Parallel_decoder_add_excl_decoder(Parallel_decoder *decoder, char *config_filename, int primary, char* pt_filename)
{
    return 1;
}

extern "C" int Parallel_decoder_add_excl_decoder2(Parallel_decoder *decoder, char *config_filename, int primary, char* pt_filename)
{
    return decoder->add_excl_decoder(config_filename, primary, pt_filename);
}

extern "C" Parallel_excl_decoder *Parallel_excl_decoder_ctor();
Parallel_excl_decoder *Parallel_excl_decoder_ctor()
{
    return new Parallel_excl_decoder();
}

extern "C" int Parallel_excl_decoder_decodePara(Parallel_excl_decoder* excl_decoder, int i)
{
    return excl_decoder->decode(i);
}

extern "C" int decode2(int i)
{
    int cpunum = i/8;
    int peventnum = i%8;
    Parallel_decoder *decoder = Parallel_decoder_ctor();
    string a = "/home/bigdataflow/DistributedDecoder/Test0/perf.data-aux-idx";
    char num = '0'+cpunum;
    a += num;

    a.append(".bin");
    char* str = (char*)a.data();
    string a2 = "/home/bigdataflow/DistributedDecoder/Test0/perf-attr-config";
    char *str2 =  (char*)a2.data();
    Parallel_decoder_add_excl_decoder2(decoder, str2, cpunum, str);
    Parallel_excl_decoder *para_decoder = Parallel_excl_decoder_get(decoder, peventnum);
    return  Parallel_excl_decoder_decodePara(para_decoder, i);
}



extern "C" int Parallel_excl_decoder_decode(Parallel_excl_decoder* excl_decoder, int i)
{
    ofstream outfile;
    outfile.open("outputFile2.txt");

    outfile<<i<<endl;
    outfile.close();
    return decode2(i);
}

/*
 * Class:     org_example_SimpDecoder
 * Method:    decode
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_org_example_SimpDecoder_decode
  (JNIEnv *env, jobject object, jint i)
  {
  return decode2(i);
  }

#ifdef __cplusplus
}
#endif


