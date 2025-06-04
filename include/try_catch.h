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

#ifdef __cplusplus
extern "C" {
#endif

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

/**
 * The required function signature for terminate handlers called when an
 * uncaught exception will halt the program.
 */
typedef void (*tryCatch_TerminateHandler)(void);

/**
 * @brief Sets a handler that is called when an uncaught exception will halt the
 * program.
 *
 * Pass @c NULL to unregister the handler.
 *
 * @param handler the handler function to be called
 */
void tryCatch_setTerminateHandler(tryCatch_TerminateHandler handler);

#define TRY(block, catchBlock) {                       \
    jmp_buf _env;                                      \
    jmp_buf* _prev = privateTryCatch_setJmpBuf(&_env); \
    void* _lastExc = tryCatch_getException();          \
    int _result = setjmp(_env);                        \
    if (_result == 0) {                                \
        { block }                                      \
    } else {                                           \
        privateTryCatch_setJmpBuf(_prev);              \
        privateTryCatch_setNeedsFree(true);            \
        bool _handled = false;                         \
        { catchBlock }                                 \
        privateTryCatch_setNeedsFree(false);           \
    }                                                  \
    privateTryCatch_setJmpBuf(_prev);                  \
    privateTryCatch_freeException(true);               \
    privateTryCatch_setException(_lastExc);            \
}

#define MH_TC_TYPE(type) type:#type

#ifdef MH_TRY_CATCH_TYPES
# define MH_TRY_CATCH_TYPE_COMMA ,
#else
# define MH_TRY_CATCH_TYPES
# define MH_TRY_CATCH_TYPE_COMMA
#endif

#define MH_TC_TYPE_STRING(type) _Generic(type, MH_TC_TYPE(int), MH_TC_TYPE(float), MH_TC_TYPE(void*) MH_TRY_CATCH_TYPE_COMMA MH_TRY_CATCH_TYPES)

#define THROW_IMPL(typeString, value) do {                             \
    privateTryCatch_freeException(false);                              \
    typeof((value)) _vl = (value);                                     \
    void* _exception = privateTryCatch_allocateException(sizeof(_vl)); \
    memcpy(_exception, &_vl, sizeof(_vl));                             \
    privateTryCatch_setExceptionType(_exception, typeString);          \
    privateTryCatch_throw(_exception);                                 \
} while (0)

#define THROW(value) THROW_IMPL(MH_TC_TYPE_STRING(value), value)
#define THROW_TYPE(type, value) THROW_IMPL(#type, (type) value)
#define THROW1(type, value) THROW_IMPL(#type, (type) value)

#define CATCH(type, name, block, ...)                               \
    if (privateTryCatch_exceptionIsType(#type)) {                   \
        const type name = *((const type*) tryCatch_getException()); \
        _handled = true;                                            \
        { block }                                                   \
    } else { __VA_ARGS__ if (!_handled) RETHROW; }

/**
 * @brief Rethrows the currently active exception.
 *
 * If used without active exception, the program is halted.
 */
#define RETHROW do {                                \
    privateTryCatch_setNeedsFree(false);            \
    privateTryCatch_throw(tryCatch_getException()); \
} while (0)

#define CATCH_ALL(name, block, ...)             \
    _handled = true;                            \
    const void* name = tryCatch_getException(); \
    { block }                                   \
    if (!_handled) { __VA_ARGS__ }


// I M P L E M E N T A T I O N - S P E C I F I C   F U N C T I O N S
//
// Please do not call them directly.

/**
 * Stores the given @c jmp_buf and returns the previously stored one.
 *
 * @param buf the new @c jmp_buf to store
 * @return the previously stored one
 */
jmp_buf* privateTryCatch_setJmpBuf(jmp_buf* buf);

/**
 * Stores the given exception pointer.
 *
 * @param exception the exception pointer to be stored
 */
void privateTryCatch_setException(void* exception);

/**
 * Stores the type name for the given exception.
 *
 * @param exception the user exception pointer
 * @param type the type name of the exception
 */
void privateTryCatch_setExceptionType(void* exception, const char* type);

/**
 * Returns whether the current exception is of the given type.
 *
 * @param type the type to check
 * @return whether the currently active exception is of the given type
 */
bool privateTryCatch_exceptionIsType(const char* type);

/**
 * Sets whether the currently active exception needs to be freed after use.
 *
 * @param needsFree whether to free the current exception
 */
void privateTryCatch_setNeedsFree(bool needsFree);

/**
 * Allocates an exception able to hold at least the given amount of memory.
 *
 * @param size the requested size for the exception
 * @return the allocated exception memory
 */
void* privateTryCatch_allocateException(size_t size);

/**
 * @brief Frees the currently active exception.
 *
 * Only frees the exception if it is marked to be freed after use.
 *
 * @param force whether to free the memory without check
 */
void privateTryCatch_freeException(bool force);

#ifdef __cplusplus
# if __cplusplus >= 201103L
#  define MH_TC_NORETURN [[noreturn]]
# else
#  define MH_TC_NORETURN
# endif
#elif defined(__STDC_VERSION__)
# if __STDC_VERSION__ < 202311L
#  define MH_TC_NORETURN _Noreturn
# else
#  define MH_TC_NORETURN [[noreturn]]
# endif
#else
# define MH_TC_NORETURN
#endif

/**
 * Performs the actual throwing of the given exception.
 *
 * @param exception the current exception pointer
 */
MH_TC_NORETURN void privateTryCatch_throw(void* exception);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __mh_try_catch_h */
