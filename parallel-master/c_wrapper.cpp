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

extern "C" DLL_EXPORT Parallel_excl_decoder *Parallel_excl_decoder_get(Parallel_decoder *dec, int i);
Parallel_excl_decoder *Parallel_excl_decoder_get(Parallel_decoder *dec, int i)
{
    return &dec->excl_decoder[i];
}

extern "C" DLL_EXPORT  int Parallel_decoder_add_excl_decoder(Parallel_decoder *self, char *config_filename, int primary, char* pt_filename)
{
    return self->add_excl_decoder(config_filename, primary, pt_filename);
}

extern "C" DLL_EXPORT Parallel_excl_decoder *Parallel_excl_decoder_ctor();
Parallel_excl_decoder *Parallel_excl_decoder_ctor()
{
    return new Parallel_excl_decoder();
}

extern "C" DLL_EXPORT  string Parallel_excl_decoder_decode(Parallel_excl_decoder *self)
{
    return self->decode();
}