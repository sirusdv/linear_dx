#include <stdio.h>
#include <math.h>

typedef unsigned long long uint64_t;

const size_t SYMS = 7;
const size_t DIGITS = 10;

const float total_window = 47.0f;
const float markers = 9.0f*0.5f;
float total_window_minus_marks = total_window - markers;

const float slop = 0.2f;

float symbols[SYMS] = { 0.0f};
int scratch[DIGITS] = {0};

size_t total_found = 0;
bool print = false;
void init_symtab() {

    printf("#Symbols: ");
    float current = 0.79f;
    for(unsigned i=0; i<SYMS; i++) {
        symbols[i] = current;
        printf("%0.2f ", current);
        current += 1.24f;
    }
    printf("\n");
}

float evaluate() {
    float sum = 0.0f;

    for(unsigned i=0; i<DIGITS; i++) {
        sum += symbols[scratch[i]];
    }
    return sum;
}

void dump() {
    for(unsigned i=0; i<DIGITS; i++) {
        printf("%u", scratch[i]);
        //printf("%u", scratch[DIGITS-1-i]);
    }
    printf("\n");
}

void recurse(unsigned depth) {
    if(depth == 0) {
        if(fabs(total_window_minus_marks - evaluate()) < slop) {
            total_found++;
            if(print)
                dump();
        }
        return;
    }

    for(unsigned i=0; i<SYMS; i++) {
        scratch[depth-1] = i;
        recurse(depth - 1);
    }
}

int main(int argc, const char **argv) {
    init_symtab();
    
    if(argc >1) 
        print = true;

    printf("#Target: %.2f\n", total_window_minus_marks);
    recurse(DIGITS);
    printf("#Found valid keys: %lu\n", total_found);
    return 0;
}
