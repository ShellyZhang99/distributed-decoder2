#ifndef PARALLEL_DECODER_H
#define PARALLEL_DECODER_H
#include "parallel_excl_decoder.h"
#include "pt_cpu.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <time.h>
using namespace std;
#define PARALLEL_LEVEL 8
class Parallel_decoder{
public:
    struct pt_conf_flags flags;

    /* The perf event sideband decoder configuration. */
    struct pt_sb_pevent_config pevent;

    /* decoder exclusive for each thread parallel decoding*/
    Parallel_excl_decoder excl_decoder[PARALLEL_LEVEL];

    struct pt_config config;

    //Func: chushihuahanshuhejiexihanshu
    Parallel_decoder();

    ~Parallel_decoder();

    int pt2_have_decoder();

    int parse_range(const char *arg, uint64_t *begin, uint64_t *end);

    int preprocess_filename(char *filename, uint64_t *offset, uint64_t *size);

    int load_file(uint8_t **buffer, size_t *psize, const char *filename, uint64_t offset, uint64_t size, const char *prog);

    int load_pt(char *arg, const char *prog);

    int pt2_sb_pevent(char *filename, const char *prog);

    int pt_cpu_parse(struct pt_cpu *cpu, const char *s);

    static int pt2_sb_output_error(int errcode, const char *filename, uint64_t offset, void *priv)
    {
    	const char *errstr, *severity;

        int id = -1;
        if (priv)
            id = *(uint32_t *)priv;

    	if (errcode >= 0)
    		return 0;

    	if (!filename)
    		filename = "<unknown>";

    	severity = errcode < 0 ? "error" : "warning";

    	errstr = errcode < 0
    		? pt_errstr(pt_errcode(errcode))
    		: pt_sb_errstr((enum pt_sb_error_code) errcode);

    	if (!errstr)
    		errstr = "<unknown error>";

    	fprintf(stderr, "[%s:%016" PRIx64 " sideband %d %s: %s]\n", filename, offset,
    	       id, severity, errstr);

    	return 0;
    }

    int get_arg_uint64(uint64_t *value, const char *option, const char *arg, const char *prog);

    int get_arg_uint32(uint32_t *value, const char *option, const char *arg, const char *prog);

    int get_arg_uint16(uint16_t *value, const char *option, const char *arg, const char *prog);

    int get_arg_uint8(uint8_t *value, const char *option, const char *arg, const char *prog);

    int split_trace(struct pt_config *config);

    int alloc_decoder(const char *prog);

    int add_excl_decoder(char *config_filename, int primary, char* pt_filename);

    int parallel_decode(char *config_filename, int primary, char* pt_filename);

    int pt2_cpu_parse(struct pt_cpu *cpu, char *s)
    {
    	const char sep = '/';
    	char *endptr;
    	long family, model, stepping;

    	if (!cpu || !s)
    		return -pte_invalid;

    	family = strtol(s, &endptr, 0);
    	if (s == endptr || *endptr == '\0' || *endptr != sep)
    		return -pte_invalid;

    	if (family < 0 || family > 65535)
    		return -pte_invalid;

    	/* skip separator */
    	s = endptr + 1;

    	model = strtol(s, &endptr, 0);
    	if (s == endptr || (*endptr != '\0' && *endptr != sep))
    		return -pte_invalid;

    	if (model < 0 || model > 255)
    		return -pte_invalid;

    	if (*endptr == '\0')
    		/* stepping was omitted, it defaults to 0 */
    		stepping = 0;
    	else {
    		/* skip separator */
    		s = endptr + 1;

    		stepping = strtol(s, &endptr, 0);
    		if (*endptr != '\0')
    			return -pte_invalid;

    		if (stepping < 0 || stepping > 255)
    			return -pte_invalid;
    	}

    	cpu->vendor = pcv_intel;
    	cpu->family = (uint16_t) family;
    	cpu->model = (uint8_t) model;
    	cpu->stepping = (uint8_t) stepping;

    	return 0;
    }
};
#endif