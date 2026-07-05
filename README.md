# wUtils

> A lightweight runtime for heterogeneous data structures in C.

`wUtils` is a lightweight C11 library that provides heterogeneous containers with automatic memory management through reference counting.

The project aims to simplify the manipulation of dynamic data structures in C while keeping the API clean, expressive and easy to use.

> **⚠️ Development Status**
>
> **wUtils is currently under active development.**
>
> The API is not considered stable and may change between releases.
> Bugs may still exist and the library is **not yet recommended for production use**.

---

## Features

- Dynamic dictionary (`wDict`)
- Dynamic list (`wList`)
- Heterogeneous values (`Value`)
- Automatic reference counting
- Nested dictionaries and lists
- Generic constructors using C11 `_Generic`
- Non-recursive destruction of nested objects
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

Nested objects are supported.

```c
wDict *parent = create_dictionary(16);
wDict *child = create_dictionary(16);

insert_value(child, "value", new_value(10));

insert_value(parent, "child", new_value(child));

child = NULL;     // Ownership transferred

destroy(&parent);
```

---

## Memory Management

`wUtils` automatically manages nested dictionaries and lists using **reference counting**.

When a dictionary or list is inserted into another container, its reference count is increased automatically.

Objects whose reference count reaches zero are destroyed automatically.

```c
wDict *dict = create_dictionary(16);
wDict *child = create_dictionary(16);

insert_value(dict, "child", new_value(child));

child = NULL;

destroy(&dict);
```

---

## Current Limitations

This project is still experimental.

Known limitations include:

- No automatic hash table resizing
- No thread safety
- Cyclic references are not detected
- API may change
- Limited unit testing
- Performance has not been fully optimized

---

## Roadmap

Planned features include:

- Automatic hash table resizing
- Iterator API
- Element removal
- Deep copy support
- JSON serialization/deserialization
- Improved error handling
- Extensive unit tests
- Better documentation
- Performance optimizations

---

## Requirements

- C11
- `_Generic` support

Tested with GCC.

---

## Contributing

Contributions, bug reports and suggestions are always welcome.

Since the library is still evolving, feedback about the API design is particularly appreciated.

---

## License

This project is licensed under the **Apache License 2.0**.

See the `LICENSE` file for details.