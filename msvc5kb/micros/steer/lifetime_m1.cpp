// Steering micro 1: lifetime of code-neutral constructs (diagnostic only).
struct S { int x, y, flag, pos, rank; char code[4][8]; int Call(int, int, int); };
extern int g, h;
void Use(int);

#define BODY_HEAD \
	int en = 0; \
	if (m == 1) { if (s->Call(0x100, w, lp)) return 1; }
#define BODY_TAIL \
	if (s->code[s->rank][s->pos] == 0) { \
		en = 0; \
		if (s->rank == 0) { if (s->flag) s->flag = 0; else s->flag = 1; en = s->flag; } \
		if (s->rank == 1) { s->y = !s->y; Use(s->y); en = s->y; } \
	} \
	Use(en); return en;

int V00(S* s, unsigned m, int w, int lp) { BODY_HEAD
	BODY_TAIL }
int V01(S* s, unsigned m, int w, int lp) { BODY_HEAD
	if (0) { g = w + lp; }
	BODY_TAIL }
int V02(S* s, unsigned m, int w, int lp) { BODY_HEAD
	int k = 0; if (k) { g = w + lp; }
	BODY_TAIL }
int V03(S* s, unsigned m, int w, int lp) { BODY_HEAD
	int u = w + lp;
	BODY_TAIL }
int V04(S* s, unsigned m, int w, int lp) { BODY_HEAD
	int t = s->x; t = t;
	BODY_TAIL }
int V05(S* s, unsigned m, int w, int lp) { BODY_HEAD
	s->x = s->x;
	BODY_TAIL }
int V06(S* s, unsigned m, int w, int lp) { BODY_HEAD
	int t; if (w) t = lp; else t = lp; g = t;
	BODY_TAIL }
int V07(S* s, unsigned m, int w, int lp) { BODY_HEAD
	g = lp;
	BODY_TAIL }
int V08(S* s, unsigned m, int w, int lp) { BODY_HEAD
	if (w == w + 0 && w != w) { g = lp; }
	BODY_TAIL }
int V09(S* s, unsigned m, int w, int lp) { BODY_HEAD
	int q; for (q = 0; q < 1; q++) { }
	BODY_TAIL }
int V10(S* s, unsigned m, int w, int lp) { BODY_HEAD
	char t = s->code[s->rank][s->pos]; if (t == 0) { t = 0; }
	BODY_TAIL }
int V11(S* s, unsigned m, int w, int lp) { BODY_HEAD
	int dead = s->x; dead = s->y;
	BODY_TAIL }
