#include <jni.h>
#include <stdio.h>
#include "org_example_SimpDecoder.h"
#include "parallel_decoder.h"
#include "parallel_excl_decoder.h"
#include <iostream>
#include <stdlib.h>

#include <fstream>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <time.h>
using namespace std;

Parallel_decoder *Parallel_decoder_ctor()
{
    return new Parallel_decoder();
}
Parallel_excl_decoder *Parallel_excl_decoder_get(Parallel_decoder *decoder, int i)
{
    return &decoder->excl_decoder[i];
}

int Parallel_decoder_add_excl_decoder(Parallel_decoder *decoder, char *config_filename, int primary, char* pt_filename)
{
    return decoder->add_excl_decoder(config_filename, primary, pt_filename);
}
Parallel_excl_decoder *Parallel_excl_decoder_ctor()
{
    return new Parallel_excl_decoder();
}

int Parallel_excl_decoder_decodePara(Parallel_excl_decoder* excl_decoder, int i)
{
    return excl_decoder->decode(i);
}
JNIEXPORT jint JNICALL Java_org_example_SimpDecoder_decode
  (JNIEnv *env, jobject obj, jint i)
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
      FILE* fp;
              fp = fopen(str,"r");
            if (fp != NULL){
      Parallel_decoder_add_excl_decoder(decoder, str2, cpunum, str);
      Parallel_excl_decoder *para_decoder = Parallel_excl_decoder_get(decoder, peventnum);
      return  Parallel_excl_decoder_decodePara(para_decoder, i);
      }
      else
      return 0;
  }