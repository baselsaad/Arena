# Arena Memory Allocator

A lightweight, single-header C++ Arena Allocator with automatic destructor management.

Side project to refresh my C++ memory.

## Features

- Pre-allocates a fixed block of memory
- Alignment
- Allocates objects linearly from that block
- Calls destructors automatically on `Reset()` or ~Arena()
- Not thread-safe
- Automatic destructor on Reset() or ~Arena()
- O(1) allocations
- O(n) destruction on reset

## Usage

```cpp
#include "Arena.h"

MemoryManagement::Arena arena(ARENA_MB * 4); // 4 MB arena
MyType* obj = arena.AllocateObj<MyType>(arg1, arg2);
float* data = arena.AllocateRaw<float>(256); // 256 elements
arena.Reset();
