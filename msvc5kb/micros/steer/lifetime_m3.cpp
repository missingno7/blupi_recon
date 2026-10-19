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
int T1_and(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	s->pos++;
	if (T[s->rank][s->pos] == 0 && T[s->rank][s->pos] == 0)
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
int T2_assign(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	s->pos++;
	if (T[s->rank][s->pos] == 0 && T[s->rank][s->pos] + 0 == 0)
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
int T4_notnot(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	s->pos++;
	if (!T[s->rank][s->pos] && !T[s->rank][s->pos])
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
int T5_or(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	s->pos++;
	if (T[s->rank][s->pos] == 0 || T[s->rank][s->pos] == 0)
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
int T7_uchar(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	s->pos++;
	if ((unsigned char)T[s->rank][s->pos] == 0)
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
int T8_int(S* s, unsigned m, int w, int lp)
{
	int en = 0;
	if (m == 1)
	{
		if (s->Call(0x100, w, lp)) return 1;
	}
	s->pos++;
	if ((int)T[s->rank][s->pos] == 0)
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
int O1_and(S* s, unsigned m, int w, int lp)
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
			if (s->flag && s->flag) s->flag = 0; else s->flag = 1;
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
int O2_or(S* s, unsigned m, int w, int lp)
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
			if (s->flag != 0 || s->flag != 0) s->flag = 0; else s->flag = 1;
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
int O3_elseif(S* s, unsigned m, int w, int lp)
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
			if (s->flag) s->flag = 0; else if (!s->flag) s->flag = 1;
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
int O4_nested(S* s, unsigned m, int w, int lp)
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
			if (s->flag) { if (s->flag) s->flag = 0; } else s->flag = 1;
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
int O5_dup(S* s, unsigned m, int w, int lp)
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
			if (s->flag == s->flag) {}
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
