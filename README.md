# wUtils

> A lightweight runtime for heterogeneous data structures in C.

`wUtils` is a lightweight C11 library that provides heterogeneous containers with automatic memory management through reference counting.
The project aims to simplify the manipulation of dynamic data structures in C while keeping the API clean, expressive and easy to use.

> **⚠️ Development Status**
>
> **wUtils is currently under active development.**
>
> The API is not considered stable and may change between releases.
> Core memory-management paths — including nesting, shared ownership, and automatic resizing — have been manually verified with `leaks`/Valgrind/AddressSanitizer (see [Testing](#testing)), but coverage is not exhaustive.
> The library is **not thread-safe** (see [Current Limitations](#current-limitations)) and is **not yet recommended for production use**.

---

## Features

- Dynamic dictionary (`wDict`) with **automatic hash table resizing**
- Dynamic list (`wList`) with automatic capacity growth
- Heterogeneous values (`Value`)
- Automatic reference counting, including nested and shared ownership
- Nested dictionaries and lists (dict-in-dict, dict-in-list, and any mix thereof)
- Generic constructors using C11 `_Generic` (`new_value`, `destroy`)
- Non-recursive (worklist-based) destruction of nested objects
- Error reporting through status codes
- Simple and lightweight API

### Supported value types

- Integer
- Double
- String
- Dictionary
- List

---

## Example

```c
#include <stdio.h>
#include "wUtils.h"

int main(void)
{
    wDict *dict = create_dictionary(16);
    insert_value(dict, "name", new_value("Walter"));
    insert_value(dict, "age", new_value(27));

    printf("%s\n", (char *)get_value(dict, "name"));
    printf("%d\n", *(int *)get_value(dict, "age"));

    destroy(&dict);
    return 0;
}
```

Nested objects are supported:

```c
wDict *parent = create_dictionary(16);
wDict *child = create_dictionary(16);

insert_value(child, "value", new_value(10));
insert_value(parent, "child", new_value(child));
child = NULL;     // ownership transferred — don't use `child` directly after this

destroy(&parent); // destroys parent AND child
```

---

## Memory Management

`wUtils` manages nested dictionaries and lists automatically through **reference counting**, so you generally don't need to think about when to free nested objects — only about calling `destroy()` on the container(s) you created directly.

### How it works

When a dictionary or list is inserted into another container (via `insert_value` or `append_element`), its reference count increases by one. When a container is destroyed, it decrements the reference count of everything it holds; an object is only actually freed once its reference count reaches zero.

```c
wDict *dict  = create_dictionary(16);
wDict *child = create_dictionary(16);

insert_value(dict, "child", new_value(child));
child = NULL;

destroy(&dict);   // destroys dict, then child, automatically
```

### `destroy()` behavior

- `destroy(&obj)` sets `obj` to `NULL` after destroying it. Calling `destroy()` again on the same (now-`NULL`) pointer is safe and simply does nothing.
- If `obj` still has other owners (its reference count is greater than zero — e.g. it is still stored inside another container), `destroy()` returns a warning status (`DICT_WARNING_DEFERENTIAL` or `LIST_WARNING_DEFERENTIAL`) and does not free the object. Only call `destroy()` on an object you own directly, not on one you've inserted somewhere else; destroying the top-level container will clean up everything nested inside it.

### Shared ownership

The same object can be inserted into more than one container. It is only freed once *every* owner has released it:

```c
wDict *shared = create_dictionary(4);
insert_value(shared, "x", new_value(1));

wDict *a = create_dictionary(4);
wDict *b = create_dictionary(4);
insert_value(a, "s", new_value(shared));   // shared is now owned by 2 containers
insert_value(b, "s", new_value(shared));

destroy(&a);   // shared is still owned by `b` — NOT freed yet
destroy(&b);   // now shared has no owners left — freed here
```

### Overwriting an existing key

Calling `insert_value` on a key that already exists **replaces** the old value (it does not add a duplicate). If the replaced value was itself a nested dictionary or list, it is released the same way as during a normal `destroy()` — freed immediately if it has no other owners left.

### Automatic resizing

Both `wDict` and `wList` grow on their own once full — you don't need to plan capacity carefully or call anything manually:

- `wDict` doubles its internal table and rehashes existing entries once it fills up. This works correctly even if the dictionary being resized is itself shared with other containers.
- `wList` doubles its internal storage the same way.

The `capacity` you pass to `create_dictionary`/`create_list` is just a starting size — it only affects *when* the first resize happens, not correctness.

---

## API Quick Reference

Full signatures are available in `wUtils.h`.

| Function | What to know |
|---|---|
| `create_dictionary(capacity)` / `create_list(capacity)` | Allocates a new container. Returns `NULL` on invalid capacity or allocation failure. |
| `insert_value(dict, key, value)` | Inserts or replaces a value. Returns `DICT_OK` on success or `DICT_ERR_MEMORY` if an allocation fails. |
| `append_element(list, value)` | Appends a value to the list. Returns `LIST_OK` on success or `LIST_ERR_MEMORY` if an allocation fails. |
| `resize_dict(dict)` / `resize_list(list)` | Automatically called when containers are full. Returns a status code indicating success or allocation failure. |
| `get_value(dict, key)` / `get_element(list, index)` | Return an untyped `void *`. The caller must know the expected type and cast it correctly. Returns `NULL` if the element does not exist. |
| `destroy(&obj)` | Releases a dictionary or list and all nested objects whose reference count reaches zero. Returns a status code. |

---

## Error Handling

Most modifying operations return a status code.

Example:

```c
wDict *dict = create_dictionary(16);

int status = insert_value(dict, "name", new_value("Walter"));

if(status != DICT_OK){
    printf("Insertion failed\n");
}

destroy(&dict);
```


## Testing

There's no automated test suite yet, but the core behaviors have been manually verified using `leaks` (macOS) — Valgrind or AddressSanitizer are recommended as portable alternatives on Linux. Verified scenarios:

- Multi-level nesting of mixed dictionaries and lists
- Shared ownership across multiple containers, destroyed in different orders
- Overwriting a key that holds a nested dictionary/list
- Allocation failures during creation, insertion, resizing and destruction paths
- Resizing a dictionary that is shared with another container


Error codes are exposed through:

- `DICT_OK`
- `DICT_ERR_MEMORY`
- `DICT_WARNING_DEFERENTIAL`

and:

- `LIST_OK`
- `LIST_ERR_MEMORY`
- `LIST_WARNING_DEFERENTIAL`

allowing callers to detect allocation failures or deferred destruction.

These checks confirm no leaks or use-after-free/double-free issues **on the scenarios tested** — this is not a guarantee of exhaustive correctness. An automated test suite (ideally run under ASan/Valgrind in CI) is very welcome as a contribution — see [Roadmap](#roadmap).

To reproduce the manual checks:

```bash
gcc -g -O0 main.c wUtils.c hash_fnv1a_64.c -o test_wdict

# macOS
leaks --atExit -- ./test_wdict

# Linux
valgrind --leak-check=full --show-leak-kinds=all ./test_wdict

# Cross-platform, where supported
gcc -fsanitize=address -g -O0 main.c wUtils.c hash_fnv1a_64.c -o test_wdict && ./test_wdict
```

---

## Current Limitations

This project is still experimental. Known limitations include:

- **Not thread-safe.** There is no internal locking mechanism. Sharing a `wDict` or `wList` across multiple threads requires external synchronization.
- **Untyped reads.** `get_value()` and `get_element()` return `void *` without runtime type checking. Reading a value with the wrong cast results in undefined behavior.
- **No cycle detection.** Reference counting cannot automatically collect cyclic references. A separate cycle detection mechanism would be required.
- **Debug output.** Development `printf()` statements are still present and should be replaced with a configurable logging system.
- **Partial error propagation.** Most allocation paths expose error codes, but some internal failures during resizing or cleanup are not yet fully propagated to the caller.
- **Testing is manual.** There is currently no automated CI test suite.
- **Performance** has not yet been benchmarked or optimized.

---

## Roadmap

- Iterator API
- Element removal (single key / list element)
- Deep copy support
- JSON serialization/deserialization
- Typed accessors instead of raw `void *`
- Improve and standardize error propagation across all APIs
- Debug output behind a compile-time flag
- Automated test suite running under ASan/Valgrind in CI
- Optional thread-safe wrapper (without adding locking overhead to the default API)
- Better documentation
- Performance optimizations

---

## Requirements

- C11
- `_Generic` support

Tested with GCC and Apple Clang (macOS, ARM64).

---

## Contributing

Contributions, bug reports and suggestions are always welcome.
Since the library is still evolving, feedback about the API design is particularly appreciated.

---

## License

This project is licensed under the **Apache License 2.0**.
See the `LICENSE` file for details.