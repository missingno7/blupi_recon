extern void Tick();
int Test(int *p) { int x=p[0]; p[1]=x+3; Tick(); return p[1]; }
