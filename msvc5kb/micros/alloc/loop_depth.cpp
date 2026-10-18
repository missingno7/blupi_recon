void Test(volatile int *p, int n) { for (int i=0; i<n; ++i) { p[3*i]=0x12345678; p[3*i+1]=0x12345678; p[3*i+2]=0x12345678; } }
