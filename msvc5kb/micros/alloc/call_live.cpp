extern void Tick();
int Test(int *p) { int x=p[0]; Tick(); return x+p[1]; }
