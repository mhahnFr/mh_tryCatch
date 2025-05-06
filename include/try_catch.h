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

//   P U B L I C   A P I

/**
 * @brief Returns the pointer to the currently active exception.
 *
 * The memory behind the exception is maintained by mh_tryCatch.
 *
 * @return the pointer to the currently active exception or @c NULL if no exception is
 * currently active
 */
void* tryCatch_getException(void);

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


// I M P L E M E N T A T I O N - S P E C I F I C   F U N C T I O N S
//
// Please do not call them directly.

/**
 * Stores the given @c jmp_buf and returns the previously stored one.
 *
 * @param buf the new @c jmp_buf to store
 * @return the previously stored one
 */
jmp_buf* tryCatch_setJmpBuf(jmp_buf* buf);

/**
 * Stores the given exception pointer.
 *
 * @param exception the exception pointer to be stored
 */
void tryCatch_setException(void* exception);

/**
 * Sets whether the currently active exception needs to be freed after use.
 *
 * @param needsFree whether to free the current exception
 */
void tryCatch_setNeedsFree(bool needsFree);

/**
 * Returns whether the currently active exception needs to be freed after use.
 *
 * @return whether to free the currently active exception after use
 */
bool tryCatch_getNeedsFree(void);

/**
 * @brief Frees the currently active exception.
 *
 * Only frees the exception if it is marked to be freed after use.
 *
 * @param force whether to free the memory without check
 */
void tryCatch_freeException(bool force);

/**
 * Performs the actual throwing of the given exception.
 *
 * @param exception the current exception pointer
 */
void tryCatch_throw(void* exception);

#endif /* __mh_try_catch_h */
