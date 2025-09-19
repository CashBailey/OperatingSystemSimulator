# Reference 

## Language keywords, qualifiers & operators (no header)

### `static`
Storage-class specifier.  
- **File-scope on a function:** internal linkage (not visible outside the `.c` file).  
- **Block-scope on an object:** static storage duration (one persistent instance).
```c
static void helper(void);
void f(void){ static int hits; ++hits; }
```

### `void`
Incomplete type / keyword.  
- Function returns no value; `f(void)` means no parameters; `void *` is a generic pointer. You cannot define an object of type `void`.
```c
void tick(void);
void *buf;
```

### `sizeof`
Operator yielding size in bytes as `size_t`. For types, compile-time constant; for expressions, uses the expression’s type and does not evaluate it.
```c
printf("%zu\n", sizeof(int));
int a[10]; printf("%zu\n", sizeof a);
```

### `const char` / `const char *`
Type + qualifier.  
- Read-only character data. Use `const char *` for pointers to string literals to prevent writes.
```c
const char *msg = "Hello";
```

### `#define`
Preprocessor directive (no header).  
- Creates object-like or function-like macros. Scope is the current translation unit after the directive.
```c
#define MAX_INPUT 128
#define SQR(x) ((x)*(x))
```

---

## `<stdio.h>` (C) / `<cstdio>` (C++)

### `printf`
Formatted output to `stdout`. Returns number of characters written, negative on error.
```c
#include <stdio.h>
printf("x=%d\n", 42);
```

#### Common format specifiers (C `printf` / C++ `std::printf`)

| Placeholder | Prints | Example |
|---|---|---|
| `%s` | C-string (null-terminated) | `printf("user: %s\n", name);` |
| `%c` | Single character | `printf("%c\n", 'A');` |
| `%d`, `%i` | Signed int (base 10) | `printf("%d\n", -42);` |
| `%u` | Unsigned int (base 10) | `printf("%u\n", 42u);` |
| `%x`, `%X` | Unsigned int (hex) | `printf("0x%X\n", 48879); // 0xBEEF` |
| `%o` | Unsigned int (octal) | `printf("%o\n", 511);` |
| `%f` | Floating (fixed) | `printf("%f\n", 3.14);` *(argument type is `double`)* |
| `%e`, `%E` | Scientific notation | `printf("%e\n", 3.14);` |
| `%g`, `%G` | Shorter of `%f`/`%e` | `printf("%g\n", 3.14);` |
| `%p` | Pointer value | `printf("%p\n", (void*)ptr);` |
| `%%` | Literal percent sign | `printf("%% done\n");` |

**Length modifiers (match the argument type):** (match the argument type):**

- `h` → short: `%hd`, `%hu`
- `l` → long: `%ld`, `%lu`; for wide chars/strings with `printf`, `%lc`, `%ls`
- `ll` → long long: `%lld`, `%llu`
- `z` → `size_t`: **`%zu`**, `%zx`, `%zo`
- `t` → `ptrdiff_t`: `%td`
- `j` → `intmax_t`/`uintmax_t`: `%jd`, `%ju`
- `L` → `long double`: `%Lf`, `%Le`, `%Lg`

**Gotchas:**
- Default promotions mean **`float` becomes `double`** in varargs; use `%f` for `double`.
- Always match specifiers to the **exact** argument type (e.g., `size_t` → `%zu`).
- For C++ headers, include `<cstdio>` and call `std::printf` if you want names in `std::`.

### `fflush`
Flush buffered **output** for a stream; `fflush(NULL)` flushes all output streams. Using it on input streams is undefined.
```c
printf("Prompt> "); fflush(stdout);
```

### `stdout`
Macro expanding to a `FILE *` for standard output.
```c
fprintf(stdout, "hello\n");
```

### `stdin`
Macro expanding to a `FILE *` for standard input.
```c
char buf[64]; fgets(buf, sizeof buf, stdin);
```

### `fgets`
Reads a line into a buffer **including** the newline if there’s room; null-terminates on success; returns `NULL` on error/EOF.
```c
char line[128];
if (fgets(line, sizeof line, stdin)) {
    /* use line */
}
```

---

## `<string.h>` (C) / `<cstring>` (C++)

### `strcmp`
Lexicographic compare of two null-terminated byte strings. Returns `<0` if `a<b`, `0` if equal, `>0` if `a>b`.
```c
int ok = strcmp(user, "admin") == 0;
```

### `strcspn`
Length of the initial segment of `s` containing no chars from `reject`. Handy for trimming at first newline.
```c
s[strcspn(s, "\n")] = '\0';
```

---

## C++ linkage interop

### `extern "C"`
C++ linkage specifier (no header). Wrap C-compatible declarations so C++ does not mangle names—lets C and C++ link cleanly.
```c
#ifdef __cplusplus
extern "C" {
#endif

void api_init(void);

#ifdef __cplusplus
}
#endif
```

---

## Null pointer constants

### `NULL`
- **C:** declared in `<stddef.h>` (also available via several libc headers).  
- **C++:** from `<cstddef>`; prefer `nullptr` in C++ code.
```c
#include <stddef.h>
void *p = NULL;      // C
// C++:
#include <cstddef>
void *q = nullptr;   // preferred
```

---

## Focus: `static void` (together)

A **function** that **returns no value** and, when declared at file scope, has **internal linkage** (visible only within the same `.c` file).
```c
static void sanitize(char *s) { /* … */ }
```

---

## Quick lookup

| Name                                           | Where it lives |
|-----------------------------------------------|----------------|
| `printf`, `fflush`, `stdin`, `stdout`, `fgets`| `<stdio.h>` / `<cstdio>` |
| `strcmp`, `strcspn`                            | `<string.h>` / `<cstring>` |
| `static`, `void`, `sizeof`, `#define`         | Language / preprocessor (no header) |
| `const char`                                   | Language type + qualifier (no header) |
| `extern "C"`                                  | C++ language linkage (no header) |
| `NULL`                                         | C: `<stddef.h>` / C++: `<cstddef>` (prefer `nullptr` in C++) |
