#include "parallel_decoder.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <time.h>
using namespace std;
int main(){
    printf("0\n");
    Parallel_decoder* decoder = new Parallel_decoder();
    decoder->add_excl_decoder("perf-attr-config", 2, "perf.data-aux-idx2.bin");

    int temp = decoder->excl_decoder[0].decode();
    cout<<temp<<endl;
    return 0;
}