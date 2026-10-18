extern void Tick();
extern int Mix(int,int,int,int,int,int);
int Test(int *p) { int a=p[0],b=p[1],c=p[2],d=p[3],e=p[4],f=p[5]; Tick(); return Mix(a,b,c,d,e,f)+p[6]; }
