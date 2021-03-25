#include "parallel_decoder.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <time.h>
using namespace std;



/*
*decoder中是否有insn_decoder是之前没有free掉的
*/
int Parallel_decoder::pt2_have_decoder(){
    for (int i = 0; i < PARALLEL_LEVEL; i++){
        if (this->excl_decoder[i].insn)
            return 1;
    }
    return 0;
}

/*
*初始化decoder的可trace的image 和session，pevent
*/
Parallel_decoder::Parallel_decoder()
{
    memset(this, 0, sizeof(*this));
    Parallel_excl_decoder *excl_decoder2;
    for (int i = 0; i < PARALLEL_LEVEL; i++){
        excl_decoder2 = &this->excl_decoder[i];

        excl_decoder2->id = i;
        excl_decoder2->iscache = pt_iscache_alloc(NULL);

        excl_decoder2->session = pt_sb_alloc(excl_decoder2->iscache);
    }

    memset(&this->pevent, 0, sizeof(this->pevent));
    this->pevent.size = sizeof(this->pevent);
    this->pevent.kernel_start = UINT64_MAX;
    this->pevent.time_mult = 1;
    pt_config_init(&this->config);
}

Parallel_decoder::~Parallel_decoder()
{
    if (!this)
        return;

    Parallel_excl_decoder *excl_decoder;

    for (int i = 0; i < PARALLEL_LEVEL; i++){
        excl_decoder = &this->excl_decoder[i];
        pt_insn_free_decoder(excl_decoder->insn);
        pt_sb_free(excl_decoder->session);
        pt_iscache_free(excl_decoder->iscache);
    }


                    free(this->config.begin);

}

int Parallel_decoder::parse_range(const char *arg, uint64_t *begin, uint64_t *end)
{
    char *rest;

    if (!arg || !*arg)
        return 0;

    errno = 0;
    *begin = strtoull(arg, &rest, 0);
    if (errno)
        return -1;

    if (!*rest)
        return 1;

    if (*rest != '-')
        return -1;

    *end = strtoull(rest+1, &rest, 0);
    if (errno || *rest)
        return -1;

    return 2;
}

/* Preprocess a filename argument.
 *
 * A filename may optionally be followed by a file offset or a file range
 * argument separated by ':'.  Split the original argument into the filename
 * part and the offset/range part.
 *
 * If no end address is specified, set @size to zero.
 * If no offset is specified, set @offset to zero.
 *
 * Returns zero on success, a negative error code otherwise.
 */
int Parallel_decoder::preprocess_filename(char *filename, uint64_t *offset, uint64_t *size)
{
    uint64_t begin, end;
    char *range;
    int parts;

    if (!filename || !offset || !size)
        return -pte_internal;

    /* Search from the end as the filename may also contain ':'. */
    range = strrchr(filename, ':');
    if (!range) {
        *offset = 0ull;
        *size = 0ull;

        return 0;
    }

    /* Let's try to parse an optional range suffix.
     *
     * If we can, remove it from the filename argument.
     * If we can not, assume that the ':' is part of the filename, e.g. a
     * drive letter on Windows.
     */
    parts = parse_range(range + 1, &begin, &end);
    if (parts <= 0) {
        *offset = 0ull;
        *size = 0ull;

        return 0;
    }

    if (parts == 1) {
        *offset = begin;
        *size = 0ull;

        *range = 0;

        return 0;
    }

    if (parts == 2) {
        if (end <= begin)
            return -pte_invalid;

        *offset = begin;
        *size = end - begin;

        *range = 0;

        return 0;
    }

    return -pte_internal;
}

/*
*把file中的内容根据offset和size来填充进buffer里面
*/
int Parallel_decoder::load_file(uint8_t **buffer, size_t *psize, const char *filename,
		     uint64_t offset, uint64_t size, const char *prog)
{
    uint8_t *content;
    size_t read;
    FILE *file;
    long fsize, begin, end;
    int errcode;

    if (!buffer || !psize || !filename || !prog) {
        fprintf(stderr, "%s: internal error.\n", prog ? prog : "");
        return -1;
    }

    errno = 0;
    file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "%s: failed to open %s: %d.\n",
            prog, filename, errno);
        return -1;
    }

    errcode = fseek(file, 0, SEEK_END);
    if (errcode) {
        fprintf(stderr, "%s: failed to determine size of %s: %d.\n",
            prog, filename, errno);
        goto err_file;
    }

    fsize = ftell(file);
    if (fsize < 0) {
        fprintf(stderr, "%s: failed to determine size of %s: %d.\n",
            prog, filename, errno);
        goto err_file;
    }

    begin = (long) offset;
    if (((uint64_t) begin != offset) || (fsize <= begin)) {
        fprintf(stderr,
            "%s: bad offset 0x%" PRIx64 " into %s.\n",
            prog, offset, filename);
        goto err_file;
    }

    end = fsize;
    if (size) {
        uint64_t range_end;

        range_end = offset + size;
        if ((uint64_t) end < range_end) {
            fprintf(stderr,
                "%s: bad range 0x%" PRIx64 " in %s.\n",
            prog, range_end, filename);
            goto err_file;
        }

        end = (long) range_end;
    }

    fsize = end - begin;

    content = (uint8_t*) malloc((size_t) fsize);
    if (!content) {
        fprintf(stderr, "%s: failed to allocated memory %s.\n",
            prog, filename);
        goto err_file;
    }

    errcode = fseek(file, begin, SEEK_SET);
    if (errcode) {
        fprintf(stderr, "%s: failed to load %s: %d.\n",
            prog, filename, errno);
        goto err_content;
    }

    read = fread(content, (size_t) fsize, 1u, file);
    if (read != 1) {
        fprintf(stderr, "%s: failed to load %s: %d.\n",
            prog, filename, errno);
        goto err_content;
    }

    fclose(file);

    *buffer = content;
    *psize = (size_t) fsize;

    return 0;

err_content:
    free(content);

err_file:
    fclose(file);
    return -1;
}

int Parallel_decoder::load_pt(char *arg, const char *prog)
{
    uint64_t foffset, fsize;
    uint8_t *buffer;
    size_t size;
    int errcode;

    errcode = preprocess_filename(arg, &foffset, &fsize);
    if (errcode < 0) {
        fprintf(stderr, "%s: bad file %s: %s.\n", prog, arg,
            pt_errstr(pt_errcode(errcode)));
        return -1;
    }

    errcode = load_file(&buffer, &size, arg, foffset, fsize, prog);
    if (errcode < 0)
        return errcode;

    this->config.begin = buffer;
    this->config.end = buffer + size;

    return 0;
}

int Parallel_decoder::pt_cpu_parse(struct pt_cpu *cpu, const char *s)
{
	const char sep = '/';
	char *endptr;
	long family, model, stepping;

	if (!cpu || !s)
		return -pte_invalid;

	family = strtol(s, &endptr, 0);
	if (s == endptr || *endptr == '\0' || *endptr != sep)
		return -pte_invalid;

	if (family < 0 || family > WINT_MAX)
		return -pte_invalid;

	/* skip separator */
	s = endptr + 1;

	model = strtol(s, &endptr, 0);
	if (s == endptr || (*endptr != '\0' && *endptr != sep))
		return -pte_invalid;

	if (model < 0 || model > WCHAR_MAX)
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

		if (stepping < 0 || stepping > WCHAR_MAX)
			return -pte_invalid;
	}

	cpu->vendor = pcv_intel;
	cpu->family = (uint16_t) family;
	cpu->model = (uint8_t) model;
	cpu->stepping = (uint8_t) stepping;

	return 0;
}


/* split trace stream into segments */
int Parallel_decoder::split_trace(struct pt_config *config){
    struct pt_packet_decoder *pktdec;

    pktdec = pt_pkt_alloc_decoder(config);
    if (!pktdec){
        fprintf(stderr,
                "failed to create pkt decoder.\n");
        return -pte_nomem;
    }

    /* count synchronnizing points */
    int cnt = 0;
    /* default max synchronizing points */
    uint32_t size = 64;
    /* memorize synchronizing point offsets */
    uint64_t *offset_buf = (uint64_t*)malloc( 8 * size );
    if (!offset_buf){
        fprintf(stderr, "failed to create offset buffer\n");
        return -pte_nomem;
    }
    memset(offset_buf, 0, 8*size);

    int status, errcode;
    uint64_t offset = 0ull;
    for (;;) {
        status = pt_pkt_sync_forward(pktdec);
        if (status < 0){
            if (status != -pte_eos)
                fprintf(stderr, "pkt sync error: %s\n",
                    pt_errstr(pt_errcode(status)));
            break;
        }

        errcode = pt_pkt_get_sync_offset(pktdec, &offset);
        if (errcode < 0){
            fprintf(stderr, "pkt get sync error %s\n",
                    pt_errstr(pt_errcode(errcode)));
            break;
        }

        if (cnt+1 >= size){
            uint32_t new_size = 2*size;
            uint64_t *new_buf = (uint64_t*)malloc(8*new_size);
            if (!new_buf){
                free(offset_buf);
                fprintf(stderr, "failed to create offset buffer\n");
                return -pte_nomem;
            }
            memset(new_buf, 0, new_size * 8);
            memcpy(new_buf, offset_buf, size * 8);
            free(offset_buf);
            offset_buf = new_buf;
            size = new_size;
        }
        offset_buf[cnt++] = offset;
    }
    /* we also memorize the end's offset */
    offset_buf[cnt++] = config->end - config->begin;

    pt_pkt_free_decoder(pktdec);

    Parallel_excl_decoder *excl_decoder;
    int ind = 0;
    for (int i = 0; i < PARALLEL_LEVEL; i++){
        excl_decoder = &this->excl_decoder[i];
        excl_decoder->begin = config->begin + offset_buf[ind];
        ind += ((cnt-1) / PARALLEL_LEVEL);
        if (i < (cnt-1) % PARALLEL_LEVEL)
            ind++;
        if (ind >= cnt)
            break;
        excl_decoder->end = config->begin + offset_buf[ind];
    }

    free(offset_buf);
    return 0;
}

/* parallel decoder alloc */
int Parallel_decoder::alloc_decoder(const char *prog)
{
    struct pt_config p_config;
    int errcode;

    if (!this || !(&this->config) || !prog)
        return -pte_internal;

    p_config = this->config;

    p_config.flags = this->flags;

    /* split trace stream */
    if (split_trace( &p_config) < 0)
        return -1;

    /* alloc parallel decoder */
    Parallel_excl_decoder *excl_decoder;
    for (int i = 0; i < PARALLEL_LEVEL; i++){
        excl_decoder = &this->excl_decoder[i];
        if (!excl_decoder->end)
            break;
        p_config.begin = excl_decoder->begin;
        p_config.end = excl_decoder->end;

        excl_decoder->insn = pt_insn_alloc_decoder(&p_config);
        if (!excl_decoder->insn)
            return -pte_nomap;
    }

    return 0;
}

/*
*给decoder分配sb_session和处理file对应的offset和size
*/
int Parallel_decoder::pt2_sb_pevent(char *filename, const char *prog)
{
    struct pt_sb_pevent_config sb_config;
    uint64_t foffset, fsize, fend;
    int errcode;

    if (!this || !prog) {
        fprintf(stderr, "%s: internal error.\n", prog ? prog : "?");
        return -1;
    }

    errcode = preprocess_filename(filename, &foffset, &fsize);
    if (errcode < 0) {
        fprintf(stderr, "%s: bad file %s: %s.\n", prog, filename,
            pt_errstr(pt_errcode(errcode)));
        return -1;
    }

    if (SIZE_MAX < foffset) {
        fprintf(stderr,
            "%s: bad offset: 0x%" PRIx64 ".\n", prog, foffset);
        return -1;
    }

    sb_config = this->pevent;
    sb_config.filename = filename;
    sb_config.begin = (size_t) foffset;
    sb_config.end = 0;

    if (fsize) {
        fend = foffset + fsize;
        if ((fend <= foffset) || (SIZE_MAX < fend)) {
            fprintf(stderr,
                "%s: bad range: 0x%" PRIx64 "-0x%" PRIx64 ".\n",
                prog, foffset, fend);
            return -1;
        }

        sb_config.end = (size_t) fend;
    }

    Parallel_excl_decoder *excl_decoder;
    for (int i = 0; i < PARALLEL_LEVEL; i++){
        excl_decoder = &this->excl_decoder[i];
        errcode = pt_sb_alloc_pevent_decoder(
                            excl_decoder->session, &sb_config);
        if (errcode < 0) {
            fprintf(stderr, "%s: error loading %s: %s.\n", prog,
                    filename, pt_errstr(pt_errcode(errcode)));
            return -1;
        }
    }

    return 0;
}

int Parallel_decoder::get_arg_uint64(uint64_t *value, const char *option, const char *arg,
			  const char *prog)
{
    char *rest;

    if (!value || !option || !prog) {
        fprintf(stderr, "%s: internal error.\n", prog ? prog : "?");
        return 0;
    }

    if (!arg || arg[0] == 0 || (arg[0] == '-' && arg[1] == '-')) {
        fprintf(stderr, "%s: %s: missing argument.\n", prog, option);
        return 0;
    }

    errno = 0;
    *value = strtoull(arg, &rest, 0);
    if (errno || *rest) {
        fprintf(stderr, "%s: %s: bad argument: %s.\n", prog, option,
            arg);
        return 0;
    }

    return 1;
}

int Parallel_decoder::get_arg_uint32(uint32_t *value, const char *option, const char *arg,
                const char *prog)
{
    uint64_t val;

    if (!get_arg_uint64(&val, option, arg, prog))
        return 0;

    if (val > UINT32_MAX) {
        fprintf(stderr, "%s: %s: value too big: %s.\n", prog, option,
            arg);
        return 0;
    }

    *value = (uint32_t) val;

    return 1;
}


int Parallel_decoder::get_arg_uint16(uint16_t *value, const char *option, const char *arg,
			  const char *prog)
{
    uint64_t val;

    if (!get_arg_uint64(&val, option, arg, prog))
        return 0;

    if (val > UINT16_MAX) {
        fprintf(stderr, "%s: %s: value too big: %s.\n", prog, option,
            arg);
        return 0;
    }

    *value = (uint16_t) val;

    return 1;
}


int Parallel_decoder::get_arg_uint8(uint8_t *value, const char *option, const char *arg,
			 const char *prog)
{
    uint64_t val;

    if (!get_arg_uint64(&val, option, arg, prog))
        return 0;

    if (val > UINT8_MAX) {
        fprintf(stderr, "%s: %s: value too big: %s.\n", prog, option,
            arg);
        return 0;
    }

    *value = (uint8_t) val;

    return 1;
}

int Parallel_decoder::parallel_decode(char *config_filename, int primary, char* pt_filename)
{
    const char *prog = "pt2_decoder";
    int errcode, i;

    if (!config_filename){
        fprintf(stderr, "%s: error config file %s not set\n",
                prog, config_filename);
        return -1;
    }


    FILE *config_file = fopen(config_filename, "r");

    if (!config_file){
        fprintf(stderr, "%s: error config file : %s\n",
                prog, config_filename);
        goto err;
    }

    int argc;
    char arg[256];
    for (;;){
        argc = fscanf(config_file, "%s", arg);
        if (argc != 1)
            break;

        if (strcmp(arg, "--sideband") == 0) {
            int cpu_num;
            argc = fscanf(config_file, "%d%s", &cpu_num, arg);
            if (argc != 2) {
                fprintf(stderr, "%s: sideband: "
                    "missing argument.\n", prog);
                goto err;
            }
            this->pevent.primary = (cpu_num == primary)? 1 : 0;
            errcode = pt2_sb_pevent(arg, prog);
            if (errcode < 0)
                goto err;

            continue;
        }

        if (strcmp(arg, "--sample-type") == 0) {
            argc = fscanf(config_file, "%s", arg);
            if (argc != 1)
                goto err;

            if (!get_arg_uint64(&this->pevent.sample_type,
                            "sample-type",
                            arg, prog))
                goto err;

            continue;
        }
        if (strcmp(arg, "--time-zero") == 0) {
            argc = fscanf(config_file, "%s", arg);
            if (argc != 1)
                goto err;

            if (!get_arg_uint64(&this->pevent.time_zero,
                            "time-zero",
                            arg, prog))
                goto err;

            continue;
        }
        if (strcmp(arg, "--time-shift") == 0) {
            argc = fscanf(config_file, "%s", arg);
            if (argc != 1)
                goto err;
            if (!get_arg_uint16(&this->pevent.time_shift,
                            "time-shift", arg, prog))
                goto err;

            continue;
        }
        if (strcmp(arg, "--time-mult") == 0) {
            argc = fscanf(config_file, "%s", arg);
            if (argc != 1)
                goto err;
		    if (!get_arg_uint32(&this->pevent.time_mult,
                            "time-mult", arg, prog))
                goto err;

            continue;
        }

        if (strcmp(arg, "--cpu") == 0) {
            /* override cpu information before the decoder
             * is initialized.
             */

            argc = fscanf(config_file, "%s", arg);
            if (argc != 1) {
                fprintf(stderr,
                    "%s: --cpu: missing argument.\n", prog);
                goto out;
            }
            const char *tmp = arg;
            errcode = pt_cpu_parse(&this->config.cpu, tmp);
            if (errcode < 0) {
                fprintf(stderr,
                    "%s: cpu must be specified as f/m[/s]\n",
                    prog);
                goto err;
            }
            continue;
        }

        if (strcmp(arg, "--mtc-freq") == 0) {
            argc = fscanf(config_file, "%s", arg);
            if (argc != 1)
                goto err;

            if (!get_arg_uint8(&this->config.mtc_freq, "--mtc-freq",
                            arg, prog))
                goto err;

            continue;
        }
        if (strcmp(arg, "--nom-freq") == 0) {
            argc = fscanf(config_file, "%s", arg);
            if (argc != 1)
                goto err;

            if (!get_arg_uint8(&this->config.nom_freq, "--nom-freq",
                        arg, prog))
                goto err;

            continue;
        }
        if (strcmp(arg, "--cpuid-0x15.eax") == 0) {
            argc = fscanf(config_file, "%s", arg);
            if (argc != 1)
                goto err;

            if (!get_arg_uint32(&this->config.cpuid_0x15_eax,
                            "--cpuid-0x15.eax", arg,
                            prog))
                goto err;

            continue;
        }
        if (strcmp(arg, "--cpuid-0x15.ebx") == 0) {
            argc = fscanf(config_file, "%s", arg);
            if (argc != 1)
                goto err;

            if (!get_arg_uint32(&this->config.cpuid_0x15_ebx,
                            "--cpuid-0x15.ebx", arg,
                            prog))
                goto err;

            continue;
        }
    }
    fclose(config_file);

    /*request event tick for finer grain sideband correlation */
    this->flags.variant.insn.enable_tick_events = 1;

    for (i = 0; i < PARALLEL_LEVEL; i++){
        Parallel_excl_decoder *excl_decoder;
        excl_decoder = &this->excl_decoder[i];

        pt_sb_notify_error(excl_decoder->session, pt2_sb_output_error,
                           (void *)&excl_decoder->id);
        errcode = pt_sb_init_decoders(excl_decoder->session);
        if (errcode < 0) {
            fprintf(stderr,
                "%s: error initializing sideband decoders: %s.\n",
                prog, pt_errstr(pt_errcode(errcode)));
            goto err;
        }
    }

    /* pt decoder */
    if (this->config.cpu.vendor) {
        errcode = pt_cpu_errata(&this->config.errata,
                        &this->config.cpu);
        if (errcode < 0)
            fprintf(stderr, "[0, 0: config error: %s]\n",
                pt_errstr(pt_errcode(errcode)));
    }

    errcode = load_pt(pt_filename, prog);
    if (errcode < 0)
        goto err;

    errcode = alloc_decoder(prog);
    if (errcode < 0)
        goto err;

    if (!pt2_have_decoder()) {
        fprintf(stderr, "%s: no pt file.\n", prog);
        goto err;
    }
out:
    return 0;
err:
    return -1;


}

int Parallel_decoder::add_excl_decoder(char *config_filename, int primary, char* pt_filename)
{
    printf("\n %s %d %s \n", config_filename, primary, pt_filename);
    return parallel_decode(config_filename, primary, pt_filename);

}
