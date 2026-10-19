struct S { int x, y, flag, pos, rank; int Call(int, int, int); };
extern char T[4][8];
extern int g, h;
void Use(int);
int V00(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	s->pos++;
	if (T[s->rank][s->pos] == 0)
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
int Y1_armboth(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	s->pos++;
	if (T[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			int y; if (s->flag) { y = 0; s->flag = 0; } else { y = 1; s->flag = 1; }
			en = y;
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
int Y2_copystore(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	s->pos++;
	if (T[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			int y; if (s->flag) y = 0; else y = 1;
			s->flag = y; en = y;
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
int Y3_reload(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	s->pos++;
	if (T[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			int y; if (s->flag) y = 0; else y = 1;
			s->flag = y; en = s->flag;
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
int Y5_enarms(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	s->pos++;
	if (T[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			if (s->flag) { s->flag = 0; en = 0; } else { s->flag = 1; en = 1; }
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
int Y7_char(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	s->pos++;
	if (T[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			char y; if (s->flag) y = 0; else y = 1;
			s->flag = y; en = s->flag;
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
int Y8_tern(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	s->pos++;
	if (T[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			int y = s->flag ? 0 : 1;
			s->flag = y; en = s->flag;
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
int Y9_armsreload(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	s->pos++;
	if (T[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			int y; if (s->flag) { y = 0; s->flag = y; } else { y = 1; s->flag = y; }
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
int Y10_short(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	s->pos++;
	if (T[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			short y; if (s->flag) y = 0; else y = 1;
			s->flag = y; en = s->flag;
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
int Y11_uchar(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	s->pos++;
	if (T[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			unsigned char y; if (s->flag) y = 0; else y = 1;
			s->flag = y; en = y;
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
int Y12_bool(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	s->pos++;
	if (T[s->rank][s->pos] == 0)
	{
		en = 0;
		if (s->rank == 0)
		{
			bool y; if (s->flag) y = false; else y = true;
			s->flag = y; en = s->flag;
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
