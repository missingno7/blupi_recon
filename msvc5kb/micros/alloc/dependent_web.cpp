extern void Tick();
extern int Mix(int,int,int,int);
int Test(int *p) { int x=p[0]; int y=x+3; Tick(); return Mix(y,y,y,x); }
