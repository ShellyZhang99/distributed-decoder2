#include "parallel_decoder.h"
#include "parallel_excl_decoder.h"
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

extern "C" DLL_EXPORT  Parallel_excl_decoder* Parallel_decoder_add_excl_decoder(Parallel_decoder *self, const char *config_filename, int primary, char* pt_filename)
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
