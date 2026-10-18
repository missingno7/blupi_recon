#include <string.h>
#include <stdio.h>
struct Point { int x,y; };
extern void Draw(int*,Point,char*,int);
extern void Load(char*);
class C { public: int* pix; int fileIndex; char names[6][100]; void Test(); };
void C::Test() {
 int i; char res[100],text[100]; Point pos;
 Load(res); pos.x=280; pos.y=79; Draw(pix,pos,res,1);
 pos.y=122;
 for(i=0;i<6;i++) {
  pos.x=296; sprintf(text,"%d",i+1); Draw(pix,pos,text,fileIndex==i?0:1);
  pos.x=330; char* line2=strchr(names[i],10);
  if(line2==0) { Draw(pix,pos,names[i],fileIndex==i?2:0); pos.y+=40; }
  else {
   strcpy(text,names[i]);
   text[line2-names[i]]=0;
   pos.x=330; pos.y-=8;
   Draw(pix,pos,text,fileIndex==i?2:0);
   ++line2;
   pos.y+=16; pos.x=330;
   Draw(pix,pos,line2,fileIndex==i?2:0);
   pos.y+=32;
  }
 }
}
