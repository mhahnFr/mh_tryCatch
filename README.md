# mh_tryCatch
This project aims to add a lightweight `try` and `catch` mechanism as known from other languages (such as C++) to
standard C.

## Building
To use this small library, add the file [`try_catch.c`][2] to your build system. It should compile without warnings.
> [!TIP]
> Example compilation:
> ```shell
> cc -Wall -Wextra -c try_catch.c -I <path/to/mh_tryCatch>/include
> ```

Then, simply include the header [`try_catch.h`][3]. 

## Usage
The following example illustrates how to use the `try` & `catch` system:
```c
// main.c

#include <try_catch.h>

#include <stdio.h> // For printf(...)

void thrower(void) {
    THROW("Message");
}

// Example structure used below
struct exception {
    int code;
    const char* message;
};

void throwStruct(void) {
    THROW(((struct exception) { 42, "Descriptive message" }));
}

void structHandling(void) {
    TRY({
        throwStruct();
    }, CATCH(struct exception*, e, {
        printf("Caught exception: %d - %s\n", e->code, e->message);
    }))
}    

void foo(void) {
    TRY({
        thrower();
    }, {
        // No need for CATCH, the exception being thrown is also available 
        // using tryCatch_getException()
        printf("Caught something: %p\n", tryCatch_getException());
        RETHROW;
    })
}

void bar(void) {
    printf("Will not be printed\n");
}

int main(void) {
    printf("Before the handling\n");
    
    TRY({
        foo();
        bar();
    }, CATCH(const char*, message, {
        printf("Caught an exception: %s\n", message);
    }))
    
    structHandling();
    
    printf("After the handling\n");
}
```

This example creates the following output:
```
Before the handling
Caught something: 0x600002150040
Caught an exception: Message
Caught exception: 42 - Descriptive message
After the handling
```

The detailed description follows below.

### Introduced macros
The header introduces the macros [`TRY`][5], [`CATCH`][4], [`THROW`][7] and [`RETHROW`][6].

#### `THROW`
The macro `THROW` takes one argument: the exception you wish to throw. It is copied.  
If `THROW` is used without a surrounding [`TRY`][5] macro invocation the program is halted.

Basic **example:**
```c
// main.c

#include <try_catch.h>

#include <stdio.h> // For printf(...)

int main(void) {
    printf("Before\n");

    THROW(1);

    // Never reached
    printf("After\n");
}
```

In order to throw any type, `THROW_TYPE` (or `THROW1` as alias) can be used as follows:
```c
// main.c

#include <try_catch.h>

#include <stdio.h> // For printf(...)

int main(void) {
    printf("Before\n");
    
    THROW_TYPE(char*, "Descriptive message");
    // Equivalent:
    THROW1(char*, "Descriptive message");
    
    printf("Never reached");
}
```

More implicitly supported types can be added by defining the macro `MH_TRY_CATCH_TYPES` before including
the header [`try_catch.h`][3] as follows:
```c
// main.c

#define MH_TRY_CATCH_TYPES MH_TRY_CATCH_TYPE(char*), MH_TRY_CATCH_TYPE(volatile int)
#include <try_catch.h>

int main(void) {
    THROW("Descriptive message"); // Now possible
}
```

#### `TRY`
The macro `TRY` takes two blocks as parameter: the first block that may throw an exception and the second block that is
called if an exception has been thrown.

Basic **example:**
```c
// main.c

#include <try_catch.h>

#include <stdio.h> // For printf(...)

int main(void) {
    printf("Before\n");

    TRY({
        printf("Within try\n");
        THROW1(char*, "Descriptive error message");
    }, {
        printf("Within the catch block, e. g. caught an exception\n");
    })

    printf("After\n");
}
```

`TRY` blocks may be nested:
```c
// main.c

#include <try_catch.h>

#include <stdio.h> // For printf(...)

int main(void) {
    printf("Before\n");

    TRY({
        printf("Begin outer try\n");
        
        TRY({
            printf("Within inner try\n");
            
            THROW1(char*, "Reaches the inner catch block");
        }, {
            printf("Caught exception from inner try block\n");
        })
        printf("Within outer try, after inner try\n");
    }, {
        printf("Caught exception from outer try block - In this example not called.\n");
    })
    printf("After outer try block\n");
}
```

They work across functions, too:
```c
// main.c

#include <try_catch.h>

#include <stdio.h> // For printf(...)

void bar2(void) {
    printf("bar2\n");
    THROW1(char*, "From bar2");
}

void foo2(void) {
    TRY({
        bar2();
    }, {
        printf("Caught exception in foo2\n");
    })
}

void bar(void) {
    foo2();
    THROW("Message of bar");
}

void foo(void) {
    TRY({
        bar();
    }, {
        printf("Caught in foo\n");
    })
    bar2();
}

int main(void) {
    TRY({
        foo();
    }, {
        printf("Caught in main\n");
    })
}
```
Output:
```
bar2
Caught exception in foo2
Caught in foo
bar2
Caught in main
```

#### `CATCH`
Instead of a code block called after an exception has been caught the macro `CATCH` may be used:
```c
// main.c

#include <try_catch.h>

#include <stdio.h> // For printf(...)

void thrower(void) { THROW1(char*, "Descriptive message"); }

int main(void) {
    TRY({
        thrower();
    }, CATCH(char*, message, {
        printf("Caught an exception: %s\n", message);
    }))
}
```

It takes the desired type and the name of the variable as well as the actual catch block to be invoked.

Multiple catch blocks can be chained:
```c
// main.c

#include <try_catch.h>

#include <stdio.h> // For prtinf(...)

void thrower(void) { THROW1(char*, "Descriptive message"); }

int main(void) {
    TRY({
        thrower();
    }, CATCH(int, code, {
        printf("Caught exception code: %d\n", code);
    }, CATCH(char*, message, {
        printf("Caught message: %s\n", message);
    })))
}
```

If the thrown exception does not match a catch block, it is passed to the next [`TRY`][5] block:
```c
// main.c

#include <try_catch.h>

#include <stdio.h> // For printf(...)

void first(void) {
    TRY({
        THROW(1.0f);
    }, CATCH(char*, message, {
        printf("Caught message: %s\n", message);
    }))
}

int main(void) {
    TRY({
        first();
    }, CATCH(float, f, {
        printf("Caught float: %f\n", f);
    }))
}
```
Output:
```
Caught float: 1.000000
```

To prevent exceptions from escaping a [`TRY`][5] block invocation, `CATCH_ALL` can be used:
```c
// main.c

#include <try_catch.h>

#include <stdio.h> // For printf(...)

void maybeThrower(void) {
    TRY({
        THROW1(char*, "Descriptive message");
    }, CATCH_ALL(exceptionPtr, {
        printf("Caught an exception: %p\n", exceptionPtr);
    }))
}

int main(void) {
    TRY({
        maybeThrower();
    }, CATCH(char*, message, {
        printf("Caught message: %s\n", message);
    }))
}
```

Output:
```
Caught an exception: 0x6000005ec038
```

It can be used wherever the regular [`CATCH`][4] macro is appropriate.

Throwing an exception from within the catch block is also possible:
```c
// main.c

#include <try_catch.h>

#include <stdio.h> // For printf(...)

void sampleFunc(void) {
    TRY({
        THROW1(char*, "A message");
    }, CATCH(char*, message, {
        printf("Caught exception: %s\n", message);
        
        THROW1(char*, "But the caller needs to notice, too.");
    }))
}

int main(void) {
    TRY({
        sampleFunc();
    }, CATCH(char*, message, {
        printf("Caught exception: %s\n", message);
    }))
}
```

#### `RETHROW`
Instead of having to throw a new exception, the current exception can be rethrown:
```c
// main.c

#include <try_catch.h>

#include <stdio.h> // For printf(...)

void sampleFunc(void) {
    TRY({
        THROW1(char*, "Another message");
    }, CATCH(char*, message, {
        printf("Caught exception: %s\n", message);
        
        RETHROW;
    }))
}

int main(void) {
    TRY({
        sampleFunc();
    }, CATCH(char*, message, {
        // Prints the same message as sampleFunc above:
        printf("Caught exception in main: %s\n", message);
    }))
}
```

### Introduced functions
#### `void* tryCatch_getException(void)`
Using the introduced function `tryCatch_getException` the pointer to the current exception can be obtained. Outside a
catch block it is the `NULL` pointer.  
This way, the thrown exception can be accessed without the usage of the [`CATCH`][4] macro:
```c
// main.c

#include <try_catch.h>

#include <stdio.h> // For printf(...)

void thrower(void) { THROW1(char*, "The error message"); }

int main(void) {
    TRY({
        thrower();
    }, {
        printf("Caught exception: %p\n", tryCatch_getException());
    })
}
```

This function may prove more useful in functions called within the catch block that do not have the access to the
exception variable of the [`CATCH`][4] macro:
```c
// main.c

#include <try_catch.h>

#include <stdio.h> // For printf(...)

void someFunc(void) {
    // Do we handle an exception? Check the value of tryCatch_getException:
    if (tryCatch_getException() != NULL) {
        printf("Handling exception %p\n", tryCatch_getException());
    } else {
        printf("No exception handling ongoing.\n");
    }
}

int main(void) {
    TRY({
        someFunc();
        THROW1(char*, "A descriptive message");
    }, CATCH(char*, message, {
        printf("Caught exception: %s\n", message);
        
        someFunc(); // No access to the message variable above.
    }))
} 
```

#### `void tryCatch_setTerminateHandler(tryCatch_TerminateHandler handler)`
Using this function, a terminate handler function can be installed. That handler is called when an uncaught exception
will halt the program.  
By registering `NULL`, a previously registered handler can be deregistered.

Illustration:
```c
// main.c

#include <try_catch.h>

#include <stdio.h> // For printf(...)

void terminateHandler(void) {
    printf("Terminating with uncaught exception: %p\n", tryCatch_getException());
    
    // No need to terminate the program here, will be done by my_tryCatch.
}

int main(void) {
    tryCatch_setTerminateHandler(terminateHandler);

    THROW(1);
}
```

## Compatibility
This `try` and `catch` mechanism is not compatible with the implementation of C++ nor Objective-C. However, it can
carefully be used encapsulated in C parts called by other languages (potentially within their `try` and `catch`
mechanisms).

## Dependencies
[mh_tryCatch][8] adheres to the C99 standard with one exception: the operator `typeof` is used.  
This operator is defined by the widely used GNU C compiler and the LLVM Clang C compiler. Furthermore, it is part of the
standard C23.

## Final notes
This project is marked with CC0 1.0 Universal.

Written in 2025 by [mhahnFr][1]

[1]: https://github.com/mhahnFr
[2]: /src/try_catch.c
[3]: /include/try_catch.h
[4]: #catch
[5]: #try
[6]: #rethrow
[7]: #throw
[8]: https://github.com/mhahnFr/mh_tryCatch