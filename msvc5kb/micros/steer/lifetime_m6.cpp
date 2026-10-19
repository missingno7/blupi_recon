extern int g, h;
void Use(int);
int V00(int m, int w, int lp)
{
	if (m == 1) { Use(1); return 0; }
	g = lp;
	Use(w);
	return h;
}
int P1_samearms(int m, int w, int lp)
{
	if (m == 1) { Use(1); return 0; }
	if (w) g = lp; else g = lp;
	Use(w);
	return h;
}
int P2_phi(int m, int w, int lp)
{
	if (m == 1) { Use(1); return 0; }
	int t; if (w) t = lp; else t = lp; g = t;
	Use(w);
	return h;
}
int P3_tern(int m, int w, int lp)
{
	if (m == 1) { Use(1); return 0; }
	g = w ? lp : lp;
	Use(w);
	return h;
}
int P4_condsame(int m, int w, int lp)
{
	if (m == 1) { Use(1); return 0; }
	int t = lp; if (w) t = lp; g = t;
	Use(w);
	return h;
}
int P5_switch(int m, int w, int lp)
{
	if (m == 1) { Use(1); return 0; }
	switch (w) { case 1: g = lp; break; default: g = lp; break; }
	Use(w);
	return h;
}
int P6_loop1(int m, int w, int lp)
{
	if (m == 1) { Use(1); return 0; }
	int t = lp; for (int q = 0; q < 1; q++) t = lp; g = t;
	Use(w);
	return h;
}
int P8_goto(int m, int w, int lp)
{
	if (m == 1) { Use(1); return 0; }
	if (w) goto L; L: g = lp;
	Use(w);
	return h;
}
int P9_deadfirst(int m, int w, int lp)
{
	if (m == 1) { Use(1); return 0; }
	if (w) g = lp; g = lp;
	Use(w);
	return h;
}
int P10_redund(int m, int w, int lp)
{
	if (m == 1) { Use(1); return 0; }
	g = lp; if (w) g = lp;
	Use(w);
	return h;
}
int P11_nested(int m, int w, int lp)
{
	if (m == 1) { Use(1); return 0; }
	if (w) { if (lp) g = lp; else g = lp; } else g = lp;
	Use(w);
	return h;
}
int P12_phih(int m, int w, int lp)
{
	if (m == 1) { Use(1); return 0; }
	int t; if (w) t = h; else t = h; g = lp; h = t;
	Use(w);
	return h;
}
