struct S { int x, y, flag, pos, rank; char code[4][8]; int Call(int, int, int); };
extern int g, h;
void Use(int);
int V00(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int A1_if0(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	if (0) { g = w + lp; }
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int A2_kzero(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	{ int k = 0; if (k) g = w + lp; }
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int A3_wnew(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	if (w != w) g = lp;
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int A4_m12(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	if (m == 1 && m == 2) g = lp;
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int A5_lpnot(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	if (lp && !lp) g = w;
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int B1_unused(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	{ int u = w + lp; }
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int B2_unload(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	{ int u = s->x + 1; }
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int B3_untbyte(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	{ char t = s->code[s->rank][s->pos]; }
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int B4_tbyteuse(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	{ char t = s->code[s->rank][s->pos]; if (t) h = h; }
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int B5_mself(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	s->x = s->x;
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int B6_gself(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	g = g;
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int C1_decl(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	{ int unused; }
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int C2_extern(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	{ extern int q9; }
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int D1_deadcall(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	if (0) Use(lp);
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int E1_en0(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	en = 0;
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int E2_enen(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	en = en;
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int E3_enor(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	en |= 0;
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int F1_ldst(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	{ int t = s->x; s->x = t; }
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int F2_lpcopy(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	{ int t = lp; lp = t; }
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int F3_lp0(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	lp = lp + 0;
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int F4_z(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	{ int z = 0; en = z; }
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int M1_if0(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (0) en = s->flag;
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int M2_en0(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			en = 0;
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
int M3_old(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	if (s->code[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			{ int old = s->flag; }
			if (s->flag) s->flag = 0; else s->flag = 1;
			en = s->flag;
		}
		if (s->rank == 1)
		{
			s->y = !s->y;
			Use(s->y);
			en = s->y;
		}
	}
	Use(en);
	return en;
}
