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
static jmp_buf* privateTryCatch_lastJmpBuf = NULL;
/** The lastly thrown exception.                                 */
static void* privateTryCatch_lastException = NULL;
/** Indicates whether the last exception needs to be freed.      */
static bool privateTryCatch_exceptionNeedsFree = false;
/** The terminate handler called if no catch block is reachable. */
static tryCatch_TerminateHandler privateTryCatch_terminateHandler = NULL;

/**
 * @brief Terminates the program.
 *
 * If the given message is @c NULL, the active exception is handled or the
 * optionally set terminate handler is called.<br>
 * Otherwise, the given message is printed.
 *
 * @param message the internal error message
 */
MH_TC_NORETURN static inline void privateTryCatch_terminate(const char* message) {
    if (message == NULL) {
        if (privateTryCatch_terminateHandler == NULL) {
            fprintf(stderr, "mhahnFr's try_catch: Terminating due to uncaught exception of type %s!\n",
                *(char**) privateTryCatch_lastException);
        } else {
            privateTryCatch_terminateHandler();
        }
    } else {
        fprintf(stderr, "mhahnFr's try_catch: Terminating abnormally because %s.\n",
            message);
    }
    abort();
}

jmp_buf* privateTryCatch_setJmpBuf(jmp_buf* buf) {
    jmp_buf* toReturn = privateTryCatch_lastJmpBuf;
    privateTryCatch_lastJmpBuf = buf;
    return toReturn;
}

void* tryCatch_getException(void) {
    if (privateTryCatch_lastException == NULL) {
        return NULL;
    }
    return (char*) privateTryCatch_lastException + TRY_CATCH_OVERHEAD;
}

void tryCatch_setTerminateHandler(const tryCatch_TerminateHandler handler) {
    privateTryCatch_terminateHandler = handler;
}

void privateTryCatch_setException(void* exception) {
    privateTryCatch_lastException = (char*) exception - TRY_CATCH_OVERHEAD;
}

void privateTryCatch_setNeedsFree(const bool needsFree) {
    privateTryCatch_exceptionNeedsFree = needsFree;
}

bool privateTryCatch_getNeedsFree(void) {
    return privateTryCatch_exceptionNeedsFree;
}

void* privateTryCatch_allocateException(const size_t size) {
    char* toReturn = malloc(size + TRY_CATCH_OVERHEAD);
    if (toReturn == NULL) {
        privateTryCatch_terminate("exception allocation failed");
    }
    return toReturn + TRY_CATCH_OVERHEAD;
}

bool privateTryCatch_exceptionIsType(const char* type) {
    return strcmp(type, *(char**) privateTryCatch_lastException) == 0;
}

void privateTryCatch_setExceptionType(void* exception, const char* type) {
    *(const char**) ((char*) exception - TRY_CATCH_OVERHEAD) = type;
}

void privateTryCatch_freeException(const bool force) {
    if (force || privateTryCatch_exceptionNeedsFree) {
        free(privateTryCatch_lastException);
    }
}

MH_TC_NORETURN void privateTryCatch_throw(void* exception) {
    if (exception == NULL) {
        privateTryCatch_terminate("thrown exception is NULL.\n"
            "                     This is most likely caused by using RETHROW without an active exception");
    }
    privateTryCatch_setException(exception);
    if (privateTryCatch_lastJmpBuf == NULL) {
        privateTryCatch_terminate(NULL);
    }
    longjmp(*privateTryCatch_lastJmpBuf, 1);
}
