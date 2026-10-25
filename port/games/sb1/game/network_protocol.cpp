#include "game/network_protocol.h"
#include <cstring>

namespace blupi::sb1::net {
uint32_t U32(const uint8_t* p) { return uint32_t(p[0]) | uint32_t(p[1])<<8 | uint32_t(p[2])<<16 | uint32_t(p[3])<<24; }
int S16(const uint8_t* p) { return int16_t(uint16_t(p[0]) | uint16_t(p[1])<<8); }
Class Classify(const uint8_t* p,size_t n) {
    if (!p || n<2) return Class::Invalid;
    switch (p[1]) {
    case 1: if (n==2) return Class::Lobby; if (n>=20 && n<=60 && (n-20)%8==0) return Class::Gameplay; break;
    case 2: if (n==28) return Class::Lobby; break;
    case 3: case 5: case 6: if (n==12) return Class::Lobby; break;
    case 4: if (n==132) return Class::Lobby; break;
    case 7: if (n==2) return Class::Lobby; break;
    case 11: if (n==108) return Class::Lobby; break;
    case 8: case 9: case 10: if (n==4) return Class::Gameplay; break;
    case 12: if (n==3) return Class::Gameplay; break;
    }
    return Class::Invalid;
}
namespace {
bool String(const uint8_t* p,size_t n) {
    for (size_t i=0;i<n;++i) { if (!p[i]) return true; if (p[i]<32 || p[i]==127) return false; }
    return false; // Legacy cp1252 is preserved; embedded controls are not display text.
}
bool Tile(int n) { return n>=-1 && n<246; } // src/pixtables.h table_icon_object[0].
bool Position(int x,int y) {
    // Pilot acceptance envelope, not a physics clamp: one 64px sprite outside the
    // world, plus the fall interval (+1000, decblupi.cpp:3554-3570) and one tick.
    // Effect emitters offset by -45/-34 (decblupi.cpp:4010-4033, decmove.cpp:445).
    return x>=-128 && x<6528 && y>=-128 && y<7528;
}
bool Transient(int type) { return (type>=8 && type<=11) || type==14 || type==15 || type==27 || type==35 || type==36; }
bool StartType(int t,int speed) {
    // Exhaustive ObjectStart(..., TRUE) emitters: src/decblupi.cpp and decmove.cpp.
    // Dynamic personal bombs are TYPE_BOMBEPERSO1+m_team. BLUP/MAGICTRACK
    // use local allocation, not ObjectStart(..., TRUE), and are not accepted here.
    switch (t) {
    case 8: case 9: case 10: case 11: case 13: case 14: case 19: case 24:
    case 28: case 35: case 37: case 200: case 201: case 202: case 203: return speed==0;
    case 23: return speed==55 || speed==5 || speed==-5;
    case 34: case 38: return speed==55;
    case 36: return speed==20 || speed==-20;
    default: return false;
    }
}
bool DeleteType(int t) {
    // Dynamic ObjectDelete calls delete collectible/vehicle/enemy types tested in
    // decblupi.cpp:1570-1594,2704-3225 and decmove.cpp:432-488. No SB2-only types.
    switch (t) {
    case 2: case 3: case 4: case 5: case 6: case 7: case 13: case 16: case 17:
    case 18: case 19: case 20: case 21: case 22: case 23: case 24: case 25:
    case 26: case 28: case 29: case 30: case 31: case 32: case 33: case 34: case 37:
    case 38: case 200: case 201: case 202: case 203: return true;
    default: return false;
    }
}
bool Sound(int s) {
    // CSound::CacheAll (src/sound.cpp:395-408) loads consecutive files until
    // the first missing one. SB1 US ships SOUND000..SOUND065, unlike the
    // inherited SB2 SOUND_* constants later in def.h.
    return s>=0 && s<=65;
}
const char* Event(const uint8_t* p,const Context& c) {
    const int x=S16(p+2),y=S16(p+4),v=S16(p+6),speed=int8_t(p[1]);
    switch (p[0]) {
    case 20: case 21:
        if (!(p[0]==20 ? StartType(v,speed) : DeleteType(v))) return "object-type/speed";
        if (!Position(x,y)) return "object-position";
        if (!Transient(v) && !c.reliable) return "object-reliability";
        return nullptr;
    case 30:
        if (x<0 || x>=6400 || y<0 || y>=6400 || !Tile(v)) return "decor-range";
        return c.reliable ? nullptr : "decor-reliability";
    case 40: if (!Position(x,y)) return "sound-position"; [[fallthrough]];
    case 41: return Sound(v) ? nullptr : "sound-id";
    case 50: case 51:
        if (c.origin!=c.host || v<0 || v>=200 || !c.liveLift || !c.liveLift(v)) return "lift-index/type/owner";
        return nullptr;
    case 60: return v>=0 && v<4 && c.reliable ? nullptr : "cloud-rank/reliability";
    default: return "embedded-type";
    }
}
}
const char* Validate(const uint8_t* p,size_t n,const Context& c) {
    const Class kind=Classify(p,n);
    if (kind==Class::Invalid) return "type/length";
    const bool move=kind==Class::Gameplay && p[1]==1;
    if (p[0]!=(move ? 60 : n)) return "size";
    if (!c.origin || (!c.trusted && (!c.known || !c.known(c.origin)))) return "origin";
    int slot=-1,count=0; bool ready=true;
    for (int i=0;i<4;++i) if (c.players[i].present) {
        ++count; ready=ready && c.players[i].ready;
        if (c.players[i].id==c.origin) slot=i;
    }
    if (kind==Class::Lobby) {
        if (c.started && p[1]!=1 && p[1]!=3) return "lobby-after-start";
        if (!c.reliable) return "lobby-reliability";
        switch (p[1]) {
        case 1: return c.trusted && c.origin==c.host ? nullptr : "close-is-internal";
        case 2:
            // A JOIN goes to every member; only the host acts on it, the others redraw (event.cpp:4107).
            if (c.origin==c.host || U32(p+4)!=c.origin) return "join-owner";
            if (c.receiverHost && (slot!=-1 || count>=4)) return "join-duplicate/full";
            return String(p+8,20) ? nullptr : "join-name";
        case 4: {
            if (c.origin!=c.host || S16(p+2)<1 || S16(p+2)>10) return "lobby-owner/map";
            uint32_t ids[4]{}; int own=0,host=0;
            for (int i=0;i<4;++i) {
                const uint8_t* e=p+4+32*i;
                if (e[0]>1 || e[1]>1) return "lobby-flags";
                if (!e[0]) continue;
                if (S16(e+8)<0 || S16(e+8)>3 || !String(e+10,22)) return "lobby-team/name";
                ids[i]=U32(e+4);
                if (!ids[i] || !c.known || !c.known(ids[i])) return "lobby-member";
                for (int j=0;j<i;++j) if (ids[i]==ids[j]) return "lobby-duplicate";
                own+=ids[i]==c.local; host+=ids[i]==c.host;
            }
            return own==1 && host==1 ? nullptr : "lobby-local/host";
        }
        case 7:
            if (c.origin!=c.host || slot<0 || count<2 || !ready) return "start-owner/readiness";
            return nullptr;
        default:
            if (slot<0 || U32(p+4)!=c.origin) return "lobby-owned-dpid";
            if (p[1]==3) return p[8]<=1 && bool(p[8])==(c.origin==c.host) ? nullptr : "leave-host";
            if (p[1]==5) return !c.players[slot].ready && S16(p+8)>=0 && S16(p+8)<4 ? nullptr : "team-range/ready";
            if (p[1]==6) return p[8]<=1 ? nullptr : "ready-range";
            return String(p+8,100) ? nullptr : "chat-text";
        }
    }
    // Early gameplay is classified and retained by the adapter until its 60B
    // receive site runs. START and MOVE can arrive in the same service batch.
    if (slot<0) return "gameplay-member";
    if (!move) {
        if (!c.reliable) return "gameplay-reliability";
        if (p[1]==12) return p[2]<=1 ? nullptr : "pause-bool";
        if (p[1]==10) return S16(p+2)>=0 && S16(p+2)<=10 ? nullptr : "lives";
        return S16(p+2)==0 ? nullptr : "bare-data";
    }
    if (p[19]>5 || n!=20+8*size_t(p[19])) return "move-count/length";
    if (U32(p+4)>32767 || (S16(p+2)&~63) || p[18]>1 || (p[18] && !c.reliable)) return "move-time/keys/flag";
    if (!Position(S16(p+8),S16(p+10)) || S16(p+14)<0 || S16(p+14)>4) return "pose-range";
    // GetBlupiIcon (decdesign.cpp:1510) emits 2/10, Read(team) selects 11..13.
    // pixtables.h:20/565 give the sheet-specific icon counts.
    const int channel=p[16],icon=S16(p+12);
    if (channel!=2 && channel!=10 && channel!=11 && channel!=12 && channel!=13) return "pose-channel";
    if (icon< -1 || icon>=(channel==10 ? 187 : 291)) return "pose-icon";
    // Negative signed transport values from slots128..199 stay negative and
    // retain the original ignored-follow behavior (decmove.cpp:262).
    if (int8_t(p[17])>=0 && (!c.liveLift || !c.liveLift(p[17]))) return "pose-transport";
    for (unsigned i=0;i<p[19];++i) if (const char* error=Event(p+20+8*i,c)) return error;
    // Keep the historical 32767 rollover policy: poses beyond wrap are ignored
    // by the consumer's > comparison; embedded events still execute.
    return nullptr;
}
void Canonicalise(uint8_t* p,size_t n) {
    const Class kind=Classify(p,n);
    if (kind==Class::Lobby) {
        if (p[1]==2 || p[1]==3 || p[1]==5 || p[1]==6 || p[1]==11) p[2]=p[3]=0;
        if (p[1]==3 || p[1]==6) p[9]=p[10]=p[11]=0;
        if (p[1]==5) p[10]=p[11]=0;
        if (p[1]==4) for (int i=0;i<4;++i) {
            auto* e=p+4+32*i;
            if (!e[0]) std::memset(e,0,32);
            else { e[2]=e[3]=0; auto* end=static_cast<uint8_t*>(std::memchr(e+10,0,22)); if (end) std::memset(end,0,32-size_t(end-e)); }
        }
        if (p[1]==2 || p[1]==11) {
            auto* end=static_cast<uint8_t*>(std::memchr(p+8,0,n-8)); if (end) std::memset(end,0,n-size_t(end-p));
        }
    } else if (kind==Class::Gameplay && p[1]==1) {
        for (unsigned i=0;i<p[19] && i<5 && 28+8*i<=n;++i) {
            auto* e=p+20+8*i;
            if (e[0]!=20) e[1]=0;
        }
    }
}
} // namespace blupi::sb1::net
