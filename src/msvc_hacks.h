#ifndef MSVC_HACKS_H
#define MSVC_HACKS_H

#include <cassert>
#include <cstdio>
#include <cstdlib>

inline
void __IMPOSSIBLE_PATH() {
#ifdef NDEBUG
    fputs("Impossible execution path", stderr);
    abort();
#else
    assert(false);
#endif // NDEBUG
}
#define IMPOSSIBLE_PATH __IMPOSSIBLE_PATH()

#endif // MSVC_HACKS_H
