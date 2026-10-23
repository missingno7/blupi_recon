// SB2 disk ABI, compiled against the unchanged SB2 headers (never SB1's).
#include "def.h"
#include "decor.h"
#include "event.h"
#include "pixtables.h"

static_assert(sizeof(POINT) == 8 && sizeof(BOOL) == 4, "Win32 disk scalar sizes");
static_assert(sizeof(Cellule) == 2 && sizeof(MoveObject) == 48, "world cells/objects");
static_assert(sizeof(DescFile) == 868, "SB2 world header");
static_assert(sizeof(DescGameFile) == 0xdeb0, "SB2 save header (docs/SB2.md section 4)");
static_assert(sizeof(DemoHeader) == 204 && sizeof(DemoEvent) == 4, "SB2 demo records");
static_assert(sizeof(table_icon_blupi)/sizeof(short) == 1+336*6, "SB2 Blupi atlas");
static_assert(sizeof(table_icon_object)/sizeof(short) == 1+441*6, "SB2 object atlas");
static_assert(sizeof(table_icon_element)/sizeof(short) == 1+289*6, "SB2 element atlas");
static_assert(sizeof(table_icon_explo)/sizeof(short) == 1+100*6, "SB2 explosion atlas");
int main() { return 0; }
