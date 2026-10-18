extern void Fill(int*,int*);
extern void Use(int);
void Test() { int a; int b; Fill(&a,&b); Use(a); Use(b); Use(a); Use(b); }
