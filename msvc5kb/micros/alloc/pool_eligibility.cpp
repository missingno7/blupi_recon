extern void Use(int);
extern void Tick();
int Ltz(int x) { if(x<0) Use(31); return x; }
int Eqz(int x) { if(x==0) Use(31); return x; }
int Memz(int *p) { if(!p[0]) Use(31); return p[1]; }
int Memnot(int *p) { return !p[0]; }
int Localz(int x) { int a=0; if(x) a=x; Use(a); return a; }
void Callz() { Use(0); Tick(); Use(0); }
void Storez(volatile int *p) { p[0]=0; Tick(); p[1]=0; }
void Ops(volatile int *p,int x) {
 p[0]=x+10; p[1]=x-10; p[2]=x*10; p[3]=x/10; p[4]=x%10;
 p[5]=x&10; p[6]=x|10; p[7]=x^10; p[8]=x<<10; p[9]=x>>10;
 p[10]=x==10; p[11]=x<10; p[12]=x>10; p[13]=x!=10;
}
void Loop(volatile int *p,int n) { for(int i=0;i<n;++i) { p[3*i]=10; p[3*i+1]=10; p[3*i+2]=10; } }
void Flat(volatile int *p) { p[0]=10; p[1]=10; p[2]=10; }
void Separated(volatile int *p,int x) { p[0]=10; if(x) Tick(); p[1]=10; }
void Braced(volatile int *p) { { p[0]=10; } Tick(); { p[1]=10; } }
