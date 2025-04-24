/*
 * mh_tryCatch - Try Catch for C
 *
 * Copyright (C) 2025  mhahnFr
 *
 * This file is part of mh_tryCatch.
 *
 * mh_tryCatch is marked with CC0 1.0 Universal. To view a copy of this license,
 * see the file LICENSE or visit <https://creativecommons.org/publicdomain/zero/1.0/>.
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
type name = *(type*) tryCatch_getException(); { block }

#endif /* __mh_try_catch_h */
