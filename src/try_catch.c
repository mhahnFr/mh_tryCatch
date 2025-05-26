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

#define TRY_CATCH_OVERHEAD sizeof(char*)

/** Pointer to the previous jump buffer.                         */
static jmp_buf* __tryCatch_lastJmpBuf = NULL;
/** The lastly thrown exception.                                 */
static void* __tryCatch_lastException = NULL;
/** Indicates whether the last exception needs to be freed.      */
static bool __tryCatch_exceptionNeedsFree = false;
/** The terminate handler called if no catch block is reachable. */
static tryCatch_TerminateHandler __tryCatch_terminateHandler = NULL;

jmp_buf* tryCatch_setJmpBuf(jmp_buf* buf) {
    jmp_buf* toReturn = __tryCatch_lastJmpBuf;
    __tryCatch_lastJmpBuf = buf;
    return toReturn;
}

void* tryCatch_getException(void) {
    return (char*) __tryCatch_lastException + TRY_CATCH_OVERHEAD;
}

void tryCatch_setTerminateHandler(const tryCatch_TerminateHandler handler) {
    __tryCatch_terminateHandler = handler;
}

void tryCatch_setException(void* ex) {
    __tryCatch_lastException = (char*) ex - TRY_CATCH_OVERHEAD;
}

void tryCatch_setNeedsFree(bool f) {
    __tryCatch_exceptionNeedsFree = f;
}

bool tryCatch_getNeedsFree(void) {
    return __tryCatch_exceptionNeedsFree;
}

void* tryCatch_allocateException(const size_t size) {
    char* toReturn = malloc(size + TRY_CATCH_OVERHEAD);

    return toReturn + TRY_CATCH_OVERHEAD;
}

bool tryCatch_exceptionIsType(const char* type) {
    return strcmp(type, *(char**) __tryCatch_lastException) == 0;
}

void tryCatch_setExceptionType(void* exception, const char* type) {
    *(const char**) ((char*) exception - TRY_CATCH_OVERHEAD) = type;
}

void tryCatch_freeException(bool force) {
    if (force || __tryCatch_exceptionNeedsFree) {
        free(__tryCatch_lastException);
    }
}

void tryCatch_throw(void* exception) {
    tryCatch_setException(exception);
    if (__tryCatch_lastJmpBuf == NULL) {
        if (__tryCatch_terminateHandler == NULL) {
            fprintf(stderr, "mhahnFr's try_catch: Terminating due to uncaught exception of type %s!\n",
                *(char**) __tryCatch_lastException);
        } else {
            __tryCatch_terminateHandler();
        }
        abort();
    }
    longjmp(*__tryCatch_lastJmpBuf, 1);
}
