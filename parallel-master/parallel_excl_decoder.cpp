
#include "parallel_excl_decoder.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <time.h>
#include <fstream>
using namespace std;



    int Parallel_excl_decoder::pt2_sb_event(const struct pt_event *event)
    {
        struct pt_image *image;
        int errcode;

        if (!this || !event)
            return -pte_internal;

        image = NULL;

        errcode = pt_sb_event(session, &image, event, sizeof(*event),
                        stdout, 0);

        if (errcode < 0)
            return errcode;

        if (!image)
            return 0;

        if (insn){
            return pt_insn_set_image(insn, image);
        }

        return -pte_internal;
    }

   void Parallel_excl_decoder::diagnose(uint64_t ip,
    		     const char *errtype, int errcode)
    {
    	int err;
    	uint64_t pos;

    	err = -pte_internal;
    	pos = 0;


    	err = pt_insn_get_offset(insn, &pos);

    	if (err < 0) {
    		//fprintf(stderr, "%u could not determine offset: %s\n",
    		       //id, pt_errstr(pt_errcode(err)));
    		//fprintf(stderr, "%u [?, %" PRIx64 ": %s: %s]\n", id,
                    //ip, errtype, pt_errstr(pt_errcode(errcode)));
    	} else{
    		//fprintf(stderr, "%u [%" PRIx64 ", %" PRIx64 ": %s: %s]\n", id,
                   //pos, ip, errtype, pt_errstr(pt_errcode(errcode)));
        }
    }

    int Parallel_excl_decoder::drain_events_insn(uint64_t *time, int status)
    {
        struct pt_insn_decoder *ptdec;
        int errcode;

        if (!this || !time)
            return -pte_internal;

        ptdec = insn;

        while (status & pts_event_pending) {
            struct pt_event event;

            status = pt_insn_event(ptdec, &event, sizeof(event));
            if (status < 0)
                return status;

            *time = event.tsc;

            errcode = pt2_sb_event(&event);
            if (errcode < 0)
                return errcode;
        }

        return status;
    }

    int Parallel_excl_decoder::decode_insn(){
       /* if (!data){
            fprintf(stderr, "[internal error]\n");
            return -1;
        }
        struct pt2_excl_decoder *decoder = (struct pt2_excl_decoder *)data;
        */
        struct pt_excl_profiler *excl_profiler = pt_excl_profiler_alloc();
        if (!excl_profiler){
            //fprintf(stderr, "failed alloc excl profiler\n");
            return -pte_nomem;
        }

        struct pt_insn_decoder *ptdec;
        uint64_t offset, sync, time;

        int status;
        if(!this->insn)
            return -1;
        ptdec = this->insn;
        offset = 0;
        sync = 0;
        time = 0;
        const char *binary_name;
        for (;;) {
            struct pt_insn temp_insn;

            /* Initialize the IP - we use it for error reporting. */
            temp_insn.ip = 0;

            status = pt_insn_sync_forward(ptdec);

            if (status < 0) {
                uint64_t new_sync;
                int errcode;

                if (status == -pte_eos)
                    break;

                diagnose(temp_insn.ip, "sync error", status);

                /* Let's see if we made any progress.  If we haven't,
                 * we likely never will.  Bail out.
                 *
                 * We intentionally report the error twice to indicate
                 * that we tried to re-sync.  Maybe it even changed.
                 */
                errcode = pt_insn_get_offset(ptdec, &new_sync);
                if (errcode < 0 || (new_sync <= sync))
                    break;

                sync = new_sync;
                continue;
            }

            for (;;) {
                status = drain_events_insn(&time, status);
                if (status < 0)
                    break;

                if (status & pts_eos) {
                    status = -pte_eos;

                    break;
                }

                status = pt_insn_next(ptdec, &temp_insn, sizeof(temp_insn));
                if (status < 0) {
                    /* Even in case of errors, we may have succeeded
                     * in decoding the current instruction.
                    */
                    if (temp_insn.iclass != ptic_error){
                        pt_profiler_add(this->profiler, excl_profiler, &temp_insn);
                    }

                    break;
                }
                pt_profiler_add(this->profiler, excl_profiler, &temp_insn);

            }

            /* We shouldn't break out of the loop without an error. */
            if (!status)
                status = -pte_internal;

            /* We're done when we reach the end of the trace stream. */
            if (status == -pte_eos)
                break;

            diagnose(temp_insn.ip, "error", status);
        }

        pt_excl_profiler_free(excl_profiler);
        return status;
    }

///gaixie func
    int Parallel_excl_decoder::decode(int no)
    {
        int result = decode_insn();
       ofstream outfile;
                   string fileName = "/home/bigdataflow/DistributedDecoder/Test0/outputFileTemp";
                   fileName+=to_string(no);
                   fileName.append(".txt");
                   outfile.open(fileName);
               struct sourcefile *source = profiler->source_list;
                outfile<<"realy opened";
               while(source){
               outfile<<"\n\n";
               outfile<<source->filename;
               outfile<<"\n";
                   //dest = dest+ "\n\n"+source->filename + "\n";
                   //printf("%s\n", source->filename);
                   for (int i = 0; i < source->vol; i++){
                       if (source->cnt[i])
                          outfile<<"\t\tline:"<<i<<"\t\t"<<source->cnt[i]<<"\n";
                           //printf("\t\tline:%d\t\t%d\n",i, source->cnt[i]);
                   }
                   struct function *func = source->func_list;
                   while(func){
                       outfile<< "\t\t\t"<<func->functionname<<"   "<<to_string(func->cnt)<<"\n";
                       //printf("\t\t\t%s:  %d\n", func->functionname, func->cnt);
                       func = func->next;
                   }
                   source = source->next;
               }

                   outfile.close();
        return result;
    }



