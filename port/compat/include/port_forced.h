// Forced include for every TU compiled with the historical-source context
// (shared ../src TUs and port/game). Keeps the historical source unchanged
// while routing its C runtime calls to deterministic, portable port services:
//
//  * fopen      -> blupi_fopen: '\\' paths, case-insensitive lookup in the
//                  read-only game data, writes redirected to the user directory.
//  * rand/srand -> the Microsoft C runtime LCG the original was linked with
//                  (VC5 LIBC), so level generation and demos replay identically
//                  on every C library.
#ifndef BLUPI_PORT_FORCED_H
#define BLUPI_PORT_FORCED_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __cplusplus
// <cstdio>/<cstdlib> #undef the C names; include them first so a later
// include (e.g. through <string>) is a no-op and the macros below survive.
#include <cstdio>
#include <cstdlib>
#endif

#ifdef __cplusplus
extern "C" {
#endif
FILE* blupi_fopen(const char* path, const char* mode);
int   blupi_rand(void);
void  blupi_srand(unsigned int seed);
#ifdef __cplusplus
}
#endif

#define fopen  blupi_fopen
#define rand   blupi_rand
#define srand  blupi_srand

#endif
