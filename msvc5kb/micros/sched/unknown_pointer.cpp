struct C { int *pix; int index; char names[100]; };
extern void Copy(char*,char*);
extern void Draw(int*,char*,int);
void Test(C* p,int i,char* text) {
 text[i]=0;
 Draw(p->pix,text,p->index==i?2:0);
}
