extern void Fill(int*,int*);
extern void Use(int);
void Test() { int b; int a; Fill(&a,&b); Use(a); Use(b); Use(a); Use(b); }
