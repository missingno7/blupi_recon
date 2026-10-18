struct Point { int x,y; };
struct Rect { int left,top,right,bottom; };
class Pix { public: int DrawPart(int,int,Point,Rect,int,int); };
class Scene { public: Pix *pix; Point Next(); void Test(Rect,Point); };
void Scene::Test(Rect rect, Point mouse)
{
    int i,j;
    Point tiny,pos,decor;
    decor=Next();
    pos.x=decor.x*2/3;
    pos.y=decor.y*2/3;
    int x=pos.x%640;
    rect.left=x;
    int y=pos.y%480;
    tiny.x=0;
    for(i=0;i<2;i++) {
        tiny.y=0;
        rect.top=y;
        rect.bottom=480;
        for(j=0;j<2;j++) {
            rect.right=640;
            pix->DrawPart(-1,4,tiny,rect,1,0);
            tiny.y+=480-rect.top;
            rect.top=0;
        }
        tiny.x+=640-rect.left;
        rect.left=0;
    }
}
