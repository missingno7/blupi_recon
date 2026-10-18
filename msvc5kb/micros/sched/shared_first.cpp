struct C { int *pix; int index; char names[100]; };
extern void Copy(char*,char*);
extern void Draw(int*,char*,int);
void Test(C* p,int i) {
 char text[100]; Copy(text,p->names);
 char* first=text; first[i]=0;
 Draw(p->pix,first,p->index==i?2:0);
}
