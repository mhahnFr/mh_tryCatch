# mh_tryCatch
This project aims to add a lightweight `try` and `catch` mechanism as known from other languages (such as C++) to
standard C.

## Usage
### Building
To use this small library, add the file [`try_catch.c`][2] to your build system. It should compile with the widely used
warnings enabled.
> [!TIP]
> Example compilation:
> ```shell
> cc -Wall -Wextra -c try_catch.c
> ```

Then, simply include the header [`try_catch.h`][3]. 

### Introduced macros
The header introduces the macros `TRY`, `CATCH`, `THROW` and `RETHROW`.

#### `THROW`
The macro `THROW` takes one argument: the exception you wish to throw. It is copied.  
If `THROW` is used without a surrounding `TRY` macro invocation the program is halted.

> [!NOTE]
> **Example:**
> ```c
> // main.c
> 
> #include <try_catch.h>
> 
> #include <stdio.h> // For printf(...)
> 
> int main(void) {
>     printf("Before\n");
> 
>     THROW("Something descriptive");
> 
>     // Never reached
>     printf("After\n");
> }
> ```

#### `TRY`
The macro `TRY` takes two blocks as parameter: the first block that may throw an exception and the second block that is
called if an exception has been thrown.

> [!NOTE]
> **Example:**
> ```c
> // main.c
>
> #include <try_catch.h>
> 
> #include <stdio.h> // For printf(...)
> 
> int main(void) {
>     printf("Before\n");
> 
>     TRY({
>         printf("Within try\n");
>         THROW("Descriptive error message");
>     }, {
>         printf("Within the catch block, e. g. caught an exception\n");
>     })
> 
>     printf("After\n");
> }
> ```

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
            
            THROW("Reaches the inner catch block");
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
    THROW("From bar2");
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
_Coming soon!_

#### `RETHROW`
_Coming soon!_

#### `void* tryCatch_getException(void)`
_Coming soon!_

### Compatibility
This `try` and `catch` mechanism is not compatible with the implementation of C++ nor Objective-C. However, used
carefully it can be used encapsulated in C parts called by other languages (potentially within their `try` and `catch`
mechanisms).

## Dependencies
C99
free? malloc? typeof? bool?

## Final notes
This project is marked with CC0 1.0 Universal.

Written in 2025 by [mhahnFr][1]

[1]: https://github.com/mhahnFr
[2]: /src/try_catch.c
[3]: /include/try_catch.h
