#include "net/game_protocol.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
namespace wire=blupi::net::game;
using Bytes=std::vector<uint8_t>;
static void Check(bool ok,const char* what) { if (!ok) { std::fprintf(stderr,"FAIL %s\n",what); std::exit(1); } }
static Bytes Packet(int type,size_t n) { Bytes b(n); b[0]=uint8_t(n); b[1]=type; return b; }
static void Short(Bytes& b,int at,int v) { b[at]=uint8_t(v); b[at+1]=uint8_t(v>>8); }
int main() {
    wire::Context c; c.rules=&wire::SB1; c.origin=2; c.local=1; c.receiverHost=true; c.reliable=true;
    c.known=[](uint32_t id){return id==1 || id==2;}; c.liveLift=[](int i){return i==0 || i==199;};
    c.players[0]={true,true,1,0}; c.players[1]={true,true,2,1};
    auto good=[&](const Bytes& b){ const char* error=wire::Validate(b.data(),b.size(),c); if (error) std::fprintf(stderr,"type %d: %s\n",b[1],error); Check(!error,"valid packet"); };
    auto bad=[&](const Bytes& b){Check(wire::Validate(b.data(),b.size(),c)!=nullptr,"malformed packet accepted");};
    Bytes join=Packet(2,28); join[4]=2; join[8]='B'; c.players[1].present=false; good(join);
    auto b=join; b[4]=1; bad(b); b=join; for (int i=8;i<28;++i)b[i]='x'; bad(b);
    b=join; b.pop_back(); bad(b); c.players[1].present=true; bad(join);
    c.receiverHost=false; good(join); c.receiverHost=true;   // relayed to a client: the legacy code only redraws
    for (int type:{3,5,6,11}) {
        b=Packet(type,type==11 ? 108 : 12); b[4]=2;
        if (type==5)c.players[1].ready=false;
        good(b); auto wrong=b; wrong[4]=1; bad(wrong);
        wrong=b; wrong.pop_back(); bad(wrong);
        if (type==11) { for (size_t i=8;i<wrong.size();++i) b[i]='x'; b.back()='x'; bad(b); }
        else { b[8]=5; bad(b); }
    }
    c.players[1].ready=true; c.origin=1;
    auto lobby=Packet(4,132); Short(lobby,2,1);
    for (int i=0;i<2;++i) { int at=4+32*i; lobby[at]=lobby[at+1]=1; lobby[at+4]=i+1; lobby[at+8]=i; lobby[at+10]='A'+i; }
    good(lobby); b=lobby; b[2]=11; bad(b); b=lobby; b[40]=1; bad(b);
    b=lobby; b[4+32+8]=4; bad(b); b=lobby; b.pop_back(); bad(b);
    b=lobby; for (int i=14;i<36;++i)b[i]='x'; bad(b);
    auto start=Packet(7,2); good(start); c.players[1].ready=false; bad(start); c.players[1].ready=true;
    c.origin=2; bad(start); c.origin=1;
    auto close=Packet(1,2); bad(close); c.trusted=true; good(close); c.trusted=false;
    c.origin=2;
    for (int type:{8,9,10,12}) {
        b=Packet(type,type==12 ? 3 : 4); good(b);
        auto wrong=b; wrong.push_back(0); bad(wrong);
        b[2]=type==10 ? 11 : 2; bad(b);
    }
    auto move=Packet(1,20); move[0]=60; move[16]=2; move[17]=255; good(move);
    b=move; b[19]=6; bad(b); b=move; b[2]=64; bad(b); b=move; b[16]=3; bad(b);
    b=move; Short(b,12,291); bad(b); b=move; Short(b,14,5); bad(b); b=move; b[17]=198; good(b);
    b=move; b[17]=127; bad(b); b=move; Short(b,10,8000); bad(b); b=move; b[7]=1; bad(b);
    for (int type:{20,21,30,40,41,50,51,60}) {
        b=move; b.resize(28); b[19]=1; b[20]=type;
        if (type==20) Short(b,26,13);
        if (type==21) Short(b,26,6);
        if (type==50 || type==51) c.origin=1;
        good(b); auto wrong=b; Short(wrong,26,32767); bad(wrong);
        wrong=b; wrong.pop_back(); bad(wrong);
        if (type==30) { wrong=b; Short(wrong,22,-1); bad(wrong); wrong=b; Short(wrong,24,6400); bad(wrong); }
        if (type==50 || type==51) {
            wrong=b; Short(wrong,26,199); good(wrong); Short(wrong,26,200); bad(wrong);
            Short(wrong,26,-1); bad(wrong); c.origin=2; bad(b);
        }
        c.origin=2;
    }
    // Atomic validation: a valid first event never masks a bad final one.
    b=move; b.resize(60); b[19]=5;
    for (int i=0;i<5;++i) b[20+8*i]=30;
    good(b); b[52]=255; bad(b);
    c.origin=99; bad(move);
    Check(wire::Classify(move.data(),20)==wire::Class::Gameplay,"MOVE namespace");
    Check(wire::Classify(close.data(),2)==wire::Class::Lobby,"CLOSE namespace");
    // Speedy Blupi 2: the same vocabulary under its own rules.
    auto event=[&](int type,int speed,int v){ Bytes e=move; e.resize(28); e[19]=1; e[20]=type; e[21]=uint8_t(speed); Short(e,26,v); return e; };
    auto rules=[&](const wire::Rules& r,const Bytes& e,bool ok){ c.rules=&r; if (ok) good(e); else bad(e); };
    c.origin=1; b=lobby; Short(b,2,12); rules(wire::SB2,b,true); rules(wire::SB1,b,false);
    Short(b,2,13); rules(wire::SB2,b,false); c.origin=2;
    b=move; Short(b,12,335); rules(wire::SB2,b,true); rules(wire::SB1,b,false);
    Short(b,12,336); rules(wire::SB2,b,false);
    b=move; b[16]=10; Short(b,12,288); rules(wire::SB2,b,true); rules(wire::SB1,b,false);
    Short(b,12,289); rules(wire::SB2,b,false);
    rules(wire::SB2,event(30,0,440),true); rules(wire::SB1,event(30,0,440),false); rules(wire::SB2,event(30,0,441),false);
    rules(wire::SB2,event(40,0,92),true); rules(wire::SB1,event(40,0,92),false); rules(wire::SB2,event(40,0,93),false);
    rules(wire::SB2,event(20,0,46),true); rules(wire::SB1,event(20,0,46),false);         // OVER
    rules(wire::SB2,event(20,58,36),true); rules(wire::SB1,event(20,58,36),false);       // POLLUTION
    rules(wire::SB2,event(20,-3,93),true); rules(wire::SB2,event(20,5,93),false);        // EXPLO8
    rules(wire::SB2,event(21,0,100),true); rules(wire::SB2,event(21,0,204),false);       // delete any type
    c.reliable=false;
    rules(wire::SB2,event(20,0,90),true); rules(wire::SB2,event(20,0,46),false);         // EXPLO5 / OVER
    c.reliable=true;
    c.rules=nullptr; bad(move);
    std::puts("SB1 and SB2 packet corpora passed before legacy layout entry");
}
