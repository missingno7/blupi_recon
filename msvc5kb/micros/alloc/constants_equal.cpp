extern void Tick();
void Test(volatile int *p) {
    p[0]=0x12345678; p[1]=0x23456789;
    Tick();
    p[2]=0x12345678; p[3]=0x23456789;
    p[4]=0x12345678; p[5]=0x23456789;
    p[6]=0x12345678; p[7]=0x23456789;
}
