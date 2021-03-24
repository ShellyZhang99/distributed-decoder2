#include "parallel_decoder.h"
#include "parallel_excl_decoder.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <time.h>
using namespace std;
#ifdef _WIN32
#define  EXPORT_API extern "C" __declspec(dllexport)
#else
#define  EXPORT_API extern "C"  __attribute__ ((visibility("default")))
#endif
static Parallel_decoder iter;
EXPORT_API Parallel_decoder *Parallel_decoder_ctor();
Parallel_decoder *Parallel_decoder_ctor()
{
    return &iter;
}

EXPORT_API Parallel_excl_decoder *Parallel_excl_decoder_get(int i);
Parallel_excl_decoder *Parallel_excl_decoder_get(int i)
{

    return &iter.excl_decoder[i];
}

EXPORT_API  int Parallel_decoder_add_excl_decoder(Parallel_decoder *self, char *config_filename, int primary, char* pt_filename)
{
    return self->add_excl_decoder(config_filename, primary, pt_filename);
}

EXPORT_API Parallel_excl_decoder *Parallel_excl_decoder_ctor();
Parallel_excl_decoder *Parallel_excl_decoder_ctor()
{
    return new Parallel_excl_decoder();
}

EXPORT_API  string Parallel_excl_decoder_decode(Parallel_excl_decoder *self)
{
    return self->decode();
}
