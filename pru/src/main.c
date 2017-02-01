#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <libgen.h>

#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <linux/limits.h>


#include "util.h"
FILE* fout = NULL;


int pal_clock_gettime(struct timespec* tv) {
    return clock_gettime(CLOCK_MONOTONIC, tv);
}

void debug_log(const char* fmt, ...) {
        va_list arg;
        struct timespec tv;

        pal_clock_gettime(&tv);

        fprintf(fout, "%ld.%3.3ld ", tv.tv_sec, tv.tv_nsec / 1000000);

        va_start(arg, fmt);
        vfprintf(fout, fmt, arg);
        va_end(arg);

        fflush(fout);
}


static void* pru_init(const void *inptr, size_t size) {
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
    char firmware[PATH_MAX];
    char *self;
    int err;
    void* ptr;

   err = prussdrv_init(); 
   if(err) {
       LOG(("Failed to init prussdrv: %s\n", strerror(errno)));
       return NULL;
   }
   err = prussdrv_open(PRU_EVTOUT_0);
    if (err) {
        LOG(("Failed to open PRU0 interrupt: %s\n", strerror(errno)));
        goto err_open_pru;
    }

    err = prussdrv_pruintc_init(&pruss_intc_initdata);
    if (err) {
        LOG(("Failed init PRU0 interrupt: %s\n", strerror(errno)));
        goto err_open_pru;
    }

    //shuttle data to PRU
    err = prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM,
                                    0,
                                    (const unsigned int*)inptr,
                                    size);
    if (err < 0) {
        LOG(("Failed write PRU0 memory: %s\n", strerror(errno)));
        goto err_open_pru;
    }

    err = prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, &ptr);
    if (err < 0) {
        LOG(("Failed map PRU0 memory: %s\n", strerror(errno)));
        goto err_open_pru;
    }

    self = realpath("/proc/self/exe", NULL);
    if (!self) {
        LOG(("Couldn't locate PRU firmware\n"));
        goto err_open_pru;
    }

    strcpy(firmware, dirname(self));
    strcat(firmware, "/firmware.bin");
    free(self);

    err = prussdrv_exec_program(PRU0, firmware);
    if (err < 0) {
        LOG(("Failed start PRU0 firmware: %s\n", strerror(errno)));
        goto err_open_pru;
    }

    return ptr;
    


err_open_pru:
    prussdrv_exit();
    return NULL;
}
static int pru_cleanup(void) {
   int rtn = 0;

   /* clear the event (if asserted) */
   if(prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT)) {
      fprintf(stderr, "prussdrv_pru_clear_event() failed\n");
      rtn = -1;
   }

   /* halt and disable the PRU (if running) */
   if((rtn = prussdrv_pru_disable(PRU0)) != 0) {
      fprintf(stderr, "prussdrv_pru_disable() failed\n");
      rtn = -1;
   }

   /* release the PRU clocks and disable prussdrv module */
   if((rtn = prussdrv_exit()) != 0) {
      fprintf(stderr, "prussdrv_exit() failed\n");
      rtn = -1;
   }

   return rtn;
}

void cleanup() {
    fprintf(stderr, "bailing!\n");
    pru_cleanup();
}


const size_t SYMS = 8;
const size_t DIGITS = 10;
const float markers = 9.0f*0.5f;

float symbols[SYMS] ={0};

size_t total_found = 0;
bool print = false;
void init_symtab() {

    printf("#Symbols: ");
    float current = 0.79f;
    for(unsigned i=0; i<SYMS; i++) {
        symbols[i] = current;

        //Last one is the large between packet delay
        if(i == SYMS-1)
            symbols[i] = 16.91f;

        printf("(%d)%0.2f ", i, symbols[i]);
        current += 1.24f;
    }
    printf("\n");
}



int main(int argc, const char** argv) {

    void* prumem;    
    int rtn;

    fout = stdout;

    init_symtab();

    if(argc != 2) {
        LOG(("Not enough arguments\n"));
        return 1;
    }
    
    size_t inlen = strlen(argv[1]);
    size_t allocsize = sizeof(unsigned) * (inlen+1); 
    unsigned *delays = (unsigned int*)malloc(allocsize);

    memset(delays, 0, allocsize);
    
    unsigned *delayiter= delays;
    for(const char *i=argv[1];*i;i++) {
        if(*i < '0' || *i > '0'+SYMS-1) {
            LOG(("Skipping bad char: %c\n", *i));
            continue;
        }
        int idx = *i - '0';
        float us = symbols[idx] * 1000.f;
        *delayiter++ = (unsigned)(us);
    }
    
    //unsigned test[] = { 500, 1000, 2000,4000, 0 };
    //prumem = pru_init(test, sizeof(test));
    
    prumem = pru_init(delays, allocsize);
    if(!prumem) {
        pru_cleanup();
        return 1;
    }
    
    atexit(cleanup);

    LOG(("got mem at %p waiting for interupt.. \n",prumem));

    LOG(("WAVEFORM ACTIVE.\n"));

    
    rtn = prussdrv_pru_wait_event(PRU_EVTOUT_0);

    LOG(("got interupt %d\n", rtn));

    pru_cleanup();
    return 0;
}

