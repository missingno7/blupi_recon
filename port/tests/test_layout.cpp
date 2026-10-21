// Checks that the port compiles the historical on-disk structures with the
// original sizes (world/mission files, demos) and links the original C
// runtime random generator. With a game root argument, also checks the sizes
// against the shipped data files.

#include "def.h"
#include "decor.h"
#include "event.h"

#include <stdio.h>
#include <string>
#include <sys/stat.h>

// [EVIDENCE] WORLD*.BLP are 50468 bytes = sizeof(DescFile) + 2*100*100*2 + 200*sizeof(MoveObject)
// with the VC5 layout derived by hand below; DEMO200.BLP = 204 + 288 * 4.
static_assert(sizeof(POINT) == 8, "POINT is two 32-bit LONGs");
static_assert(sizeof(BOOL) == 4, "BOOL is a 32-bit int");
static_assert(sizeof(Cellule) == 2, "Cellule");
static_assert(sizeof(MoveObject) == 48, "MoveObject");
static_assert(sizeof(DescFile) == 868, "DescFile");
// [EVIDENCE] BLUPI.EXE pushes the immediates 0xd490 (54416, six sites) and
// 0x364 (868, four sites) in the CDecor read/write code: the malloc/fread/
// fwrite sizes of DescGameFile (saved games) and DescFile.
static_assert(sizeof(DescGameFile) == 54416, "DescGameFile (saved game)");
static_assert(sizeof(DemoHeader) == 204, "DemoHeader");
static_assert(sizeof(DemoEvent) == 4, "DemoEvent");

static int failures = 0;

static void check(bool ok, const char* what)
{
	printf("%s %s\n", ok ? "ok  " : "FAIL", what);
	if (!ok) failures++;
}

static long file_size(const std::string& path)
{
	struct stat st;
	return stat(path.c_str(), &st) == 0 ? (long)st.st_size : -1;
}

int main(int argc, char** argv)
{
	// Microsoft LIBC rand() after srand(1): 41, 18467, 6334, 26500, 19169.
	srand(1);
	int a = rand(), b = rand(), c = rand(), d = rand(), e = rand();
	check(a == 41 && b == 18467 && c == 6334 && d == 26500 && e == 19169, "rand() is the MS C runtime LCG");

	if (argc > 1)
	{
		std::string root = argv[1];
		long world = file_size(root + "/DATA/WORLD001.BLP");
		check(world == (long)(sizeof(DescFile) + 2 * MAXCELX * MAXCELY * sizeof(short) +
							  MAXMOVEOBJECT * sizeof(MoveObject)),
			  "WORLD001.BLP size matches DescFile + cells + move objects");
		long demo = file_size(root + "/DATA/DEMO200.BLP");
		check(demo > 0 && (demo - (long)sizeof(DemoHeader)) % (long)sizeof(DemoEvent) == 0,
			  "DEMO200.BLP is a DemoHeader followed by whole DemoEvents");
	}
	return failures == 0 ? 0 : 1;
}
