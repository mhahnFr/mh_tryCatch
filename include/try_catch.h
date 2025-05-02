/*
 * mh_tryCatch - Try Catch for C
 *
 * Written in 2025 by mhahnFr
 *
 * This file is part of mh_tryCatch.
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication along with mh_tryCatch,
 * see the file LICENSE. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

#ifndef __mh_try_catch_h
#define __mh_try_catch_h

#include <setjmp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

jmp_buf* tryCatch_setJmpBuf(jmp_buf* buf);

void tryCatch_setException(void* exception);
void* tryCatch_getException(void);

void tryCatch_setNeedsFree(bool needsFree);
bool tryCatch_getNeedsFree(void);

void tryCatch_freeException(bool force);

void tryCatch_throw(void* exception);

#define TRY(block, catchBlock) {                  \
    jmp_buf __env;                                \
    jmp_buf* __prev = tryCatch_setJmpBuf(&__env); \
    void* __lastExc = tryCatch_getException();    \
    int __result = setjmp(__env);                 \
    if (__result == 0) {                          \
        { block }                                 \
    } else {                                      \
        tryCatch_setJmpBuf(__prev);               \
        tryCatch_setNeedsFree(true);              \
        { catchBlock }                            \
        tryCatch_setNeedsFree(false);             \
    }                                             \
    tryCatch_setJmpBuf(__prev);                   \
    tryCatch_freeException(true);                 \
    tryCatch_setException(__lastExc);             \
}

#define RETHROW do {                         \
    tryCatch_setNeedsFree(false);            \
    tryCatch_throw(tryCatch_getException()); \
} while (0)

#define THROW(value) do {                        \
    tryCatch_freeException(false);               \
    void* __exception = malloc(sizeof((value))); \
    typeof((value)) __vl = (value);              \
    memcpy(__exception, &__vl, sizeof((value))); \
    tryCatch_throw(__exception);                 \
} while (0)

#define CATCH(type, name, block) \
type name = (type) tryCatch_getException(); { block }

#endif /* __mh_try_catch_h */
