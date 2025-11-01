#include "threading.h"
#include <stdio.h>
#include <sys/ucontext.h>
#include <ucontext.h>
#include <stdbool.h>

void t_init()
{
    for (int i = 0; i < NUM_CTX; i++) {
        contexts[i].state = INVALID;
        contexts[i].context.uc_stack.ss_size = STK_SZ;
        contexts[i].context.uc_stack.ss_flags = 0;
        contexts[i].context.uc_link = NULL;
    }

    // Set context 0 as main context
    contexts[0].state = VALID;
    contexts[0].context.uc_stack.ss_sp = (char *) malloc(STK_SZ);
    current_context_idx = 0;
}

int32_t t_create(fptr foo, int32_t arg1, int32_t arg2)
{
    for (volatile int i = 0; i < NUM_CTX; i++) {
        if (contexts[i].state == INVALID) {
            getcontext(&contexts[i].context);
            contexts[i].context.uc_stack.ss_sp = (char *) malloc(STK_SZ);
            makecontext(&contexts[i].context, (void (*)())foo, 2, arg1, arg2);
            contexts[i].state = VALID;
            return 0;
        }
    }
    return 1;
}

int32_t t_yield()
{
    int32_t count = 0;
    bool first = true;
    struct worker_context context;
    uint8_t curr_idx = current_context_idx;

    // Find next valid context and swap
    for (uint8_t i = 1; i < NUM_CTX; i++) {
        context = contexts[(i+current_context_idx)%NUM_CTX];
        
        if (context.state == VALID) {
            if (first) {
                current_context_idx = (i+current_context_idx)%NUM_CTX;
                swapcontext(&contexts[curr_idx].context,
                            &contexts[current_context_idx].context);
                current_context_idx = curr_idx;
                first = false;
            }
            count++;
        }
    }

    // If main is the only context, clean up all other contexts
    if (count == 0) {
        for (int i = 0; i < NUM_CTX; i++) {
            if (contexts[i].context.uc_stack.ss_sp != NULL) {
                free(contexts[i].context.uc_stack.ss_sp);
                contexts[i].context.uc_stack.ss_sp = NULL;
            }
        }
    }

    return count;
}

void t_finish()
{
    int prev = current_context_idx;
    contexts[current_context_idx].state = DONE;
    current_context_idx = 0;
    swapcontext(&contexts[prev].context, &contexts[current_context_idx].context);

    free(contexts[prev].context.uc_stack.ss_sp);
}
