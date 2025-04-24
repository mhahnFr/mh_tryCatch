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

#include <stdio.h>

#include "try_catch.h"

static jmp_buf* __tryCatch_lastJmpBuf = NULL;
static void* __tryCatch_lastException = NULL;
static bool __tryCatch_exceptionNeedsFree = false;

jmp_buf* tryCatch_setJmpBuf(jmp_buf* buf) {
    jmp_buf* toReturn = __tryCatch_lastJmpBuf;
    __tryCatch_lastJmpBuf = buf;
    return toReturn;
}

void* tryCatch_getException(void) {
    return __tryCatch_lastException;
}

void tryCatch_setException(void* ex) {
    __tryCatch_lastException = ex;
}

void tryCatch_setNeedsFree(bool f) {
    __tryCatch_exceptionNeedsFree = f;
}

bool tryCatch_getNeedsFree(void) {
    return __tryCatch_exceptionNeedsFree;
}

void tryCatch_freeException(bool force) {
    if (force || __tryCatch_exceptionNeedsFree) {
        free(__tryCatch_lastException);
    }
}

void tryCatch_throw(void* exception) {
    tryCatch_setException(exception);
    if (__tryCatch_lastJmpBuf == NULL) {
        fputs("mhahnFr's try_catch: Terminating due to uncaught exception!\n", stderr);
        abort();
    }
    longjmp(*__tryCatch_lastJmpBuf, 1);
}
