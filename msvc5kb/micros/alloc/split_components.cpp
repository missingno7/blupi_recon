extern void Tick();
extern int Mix(int,int,int,int,int,int);
void Test(volatile int *p,int n) {
 int a=p[0],b=p[1],c=p[2],d=p[3],e=p[4],f=p[5];
 p[7]=10;p[8]=1;Tick();p[9]=10;p[10]=1;
 if(n) {p[11]=Mix(a,b,c,d,e,f);p[12]=10;p[13]=1;}
 p[14]=a+b+c+d+e+f;p[15]=10;p[16]=1;
}
void Loop(volatile int *p,int n) {
 int a=p[0],b=p[1],c=p[2],d=p[3],e=p[4],f=p[5];
 for(int i=0;i<n;i++) {
  p[7]=10;p[8]=1;Tick();p[9]=10;p[10]=1;
  p[11]=Mix(a,b,c,d,e,f);p[12]=10;p[13]=1;
 }
 p[14]=a+b+c+d+e+f;p[15]=10;p[16]=1;
}
