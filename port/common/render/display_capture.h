// Cross-build oracle: raw rows at each game Display, before presentation.
// Log/hash and BLPF dump format match port/oracle/ddraw_proxy.cpp.
#ifndef BLUPI_RENDER_DISPLAY_CAPTURE_H
#define BLUPI_RENDER_DISPLAY_CAPTURE_H
#include <cstdio>
#include "render/surface.h"
namespace blupi::render {
class DisplayCapture {
public:
    void configure(FILE* log, const char* dumpDir, long dumpEvery);
    void write(const Surface& back, const Palette& pal);
private:
    FILE* m_log = nullptr;
    char m_dumpDir[260] = "";
    long m_dumpEvery = 0;
    long m_index = 0;
};
}
#endif
