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

#include <stdio.h>

#include "try_catch.h"

jmp_buf* __tryCatch_lastJmpBuf = NULL;
void* __tryCatch_lastException = NULL;
bool __tryCatch_exceptionNeedsFree = false;

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
