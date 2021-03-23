#ifndef PARALLEL_EXCL_DECODER_H
#define PARALLEL_EXCL_DECODER_H

#include "libipt/internal/include/pt_cpu.h"
#include "libipt/include/intel_pt.h"
#include "sideband/include/libipt_sb.h"
#include "profiler/include/profiler.h"
#include "profiler/internal/include/pt_profiler.h"
#include "include/threads.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <time.h>
using namespace std;

class Parallel_excl_decoder{
public:
    /* thread exclusive identifier */
    uint32_t  id;

    /* the actual decoder. */
    struct pt_insn_decoder *insn;

    /* the sideband session */
    struct pt_sb_session *session;

    /* The image section cache. */
    struct pt_image_section_cache *iscache;

    struct pt_profiler *profiler;

    /* thread exectuion status */
    int retStatus;

    uint8_t *begin;
    uint8_t *end;

    int pt2_sb_event(const struct pt_event *event);

    void diagnose(uint64_t ip, const char *errtype, int errcode);

    int drain_events_insn(uint64_t *time, int status);

    int decode_insn();

    string decode();
};

#endif