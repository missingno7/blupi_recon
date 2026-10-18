struct C { int *pix; int index; char names[100]; };
extern void Copy(char*,char*);
extern void Draw(int*,char*,int);
void Test(C* p,int i) {
 char text[100]; Copy(text,p->names); text[i]=0;
 char* first=text;
 Draw(p->pix,first,p->index==i?2:0);
}
