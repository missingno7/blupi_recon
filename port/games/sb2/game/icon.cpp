// blupi2's window icon: games/sb2/res/icon.png (tools/make_icon.py --game sb2: the
// waving Blupi of SB1's icon, from SB2's green player sheet), embedded at build
// time and passed to host::init (main.cpp). Windows takes the icon from the
// executable's resources (games/sb2/res/icon.ico).

extern const unsigned char g_sb2IconPng[] = {
#include "sb2_icon_png.inc"
};
extern const unsigned g_sb2IconPngSize = sizeof(g_sb2IconPng);
