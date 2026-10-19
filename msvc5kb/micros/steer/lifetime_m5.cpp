struct S { int x, y, flag, pos, rank; int Call(int, int, int); };
extern char T[4][8];
extern int g, h;
void Use(int);
enum E2 { E2_0, E2_1 };
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
int R_int(S* s, unsigned m, int w, int lp)
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
int C_int(S* s, unsigned m, int w, int lp)
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
int E_int(S* s, unsigned m, int w, int lp)
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
			en = y; s->flag = y;
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
int R_uint(S* s, unsigned m, int w, int lp)
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
			unsigned int y; if (s->flag) y = 0; else y = 1;
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
int C_uint(S* s, unsigned m, int w, int lp)
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
			unsigned int y; if (s->flag) y = 0; else y = 1;
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
int E_uint(S* s, unsigned m, int w, int lp)
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
			unsigned int y; if (s->flag) y = 0; else y = 1;
			en = y; s->flag = y;
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
int R_long(S* s, unsigned m, int w, int lp)
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
			long y; if (s->flag) y = 0; else y = 1;
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
int C_long(S* s, unsigned m, int w, int lp)
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
			long y; if (s->flag) y = 0; else y = 1;
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
int E_long(S* s, unsigned m, int w, int lp)
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
			long y; if (s->flag) y = 0; else y = 1;
			en = y; s->flag = y;
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
int R_ulong(S* s, unsigned m, int w, int lp)
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
			unsigned long y; if (s->flag) y = 0; else y = 1;
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
int C_ulong(S* s, unsigned m, int w, int lp)
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
			unsigned long y; if (s->flag) y = 0; else y = 1;
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
int E_ulong(S* s, unsigned m, int w, int lp)
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
			unsigned long y; if (s->flag) y = 0; else y = 1;
			en = y; s->flag = y;
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
int R_enum(S* s, unsigned m, int w, int lp)
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
			E2 y; if (s->flag) y = E2_0; else y = E2_1;
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
int C_enum(S* s, unsigned m, int w, int lp)
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
			E2 y; if (s->flag) y = E2_0; else y = E2_1;
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
int E_enum(S* s, unsigned m, int w, int lp)
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
			E2 y; if (s->flag) y = E2_0; else y = E2_1;
			en = y; s->flag = y;
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
int R_int3(S* s, unsigned m, int w, int lp)
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
int C_int3(S* s, unsigned m, int w, int lp)
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
int E_int3(S* s, unsigned m, int w, int lp)
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
			en = y; s->flag = y;
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
