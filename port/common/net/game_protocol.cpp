#include "net/game_protocol.h"
#include <cstring>

namespace blupi::net::game {
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
bool Position(int x,int y) {
    // Pilot acceptance envelope, not a physics clamp (both games: 100x100 cells of 64 px): one
    // sprite outside the world, plus the fall interval (+1000, src/decblupi.cpp:3554-3570) and one
    // tick. Effect emitters offset by -45/-34 (src/decblupi.cpp:4010-4033, src/decmove.cpp:445).
    return x>=-128 && x<6528 && y>=-128 && y<7528;
}

// Speedy Blupi (src/).
bool Sb1Transient(int type) { return (type>=8 && type<=11) || type==14 || type==15 || type==27 || type==35 || type==36; }
bool Sb1StartType(int t,int speed) {
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
bool Sb1DeleteType(int t) {
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

// Speedy Blupi 2 (games/sb2/src/; TYPE_* values from its def.h).
bool Sb2Transient(int type) {
    // The unreliable list of CDecor::NetMessageSend (games/sb2/src/decmove.cpp:89-111): EXPLO1..10,
    // SPLOUTCH1..3, TENTACULE, PLOUF, BLUP, MAGICTRACK, SHIELDTRACK, TRESORTRACK, TIPLOUF,
    // INVERTSTART, INVERTSTOP, POLLUTION.
    switch (type) {
    case 8: case 9: case 10: case 11: case 90: case 91: case 92: case 93: case 94: case 95:
    case 98: case 99: case 100: case 53: case 14: case 15: case 27: case 57: case 39: case 35:
    case 41: case 42: case 36: return true;
    default: return false;
    }
}
bool Sb2StartType(int t,int speed) {
    // Exhaustive ObjectStart(..., TRUE) emitters of games/sb2/src (decblupi.cpp, decmove.cpp,
    // decnet.cpp, decor.cpp); personal bombs are TYPE_BOMBEPERSO1+m_team.
    switch (t) {
    case 93: // EXPLO8 (decor.cpp:177-185): speed[] of decblock.cpp:917, or half of it
        return speed==0 || speed==2 || speed==-2 || speed==3 || speed==-3 || speed==4 || speed==-4 ||
               speed==6 || speed==-6 || speed==8 || speed==-8;
    case 8: case 9: case 10: case 11: case 90: case 91: case 92:    // EXPLO1..7
    case 13: case 46: case 19: case 28: case 24: case 56:           // HELICO OVER JEEP TANK SKATE DYNAMITEf
    case 200: case 201: case 202: case 203:                         // BOMBEPERSO1..4
    case 53: case 52: case 14: case 35: case 37:                    // TENTACULE BRIDGE PLOUF TIPLOUF CLEAR
    case 98: case 99: case 100: return speed==0;                    // SPLOUTCH1..3
    case 23: return speed==55 || speed==5 || speed==-5;             // BALLE
    case 38: case 34: return speed==55;                             // ELECTRO GLU
    case 36: return speed==20 || speed==-20 || speed==58 || speed==-58; // POLLUTION
    default: return false;
    }
}
bool Sb2DeleteType(int t) {
    // The receiver only deletes an object of that type found at that position
    // (games/sb2/src/decnet.cpp:131 MoveObjectSearch); any object type is harmless.
    return t>=1 && t<=203;
}
}

// SB1: src/pixtables.h table_icon_object/element/blupi[0]; CSound::CacheAll (src/sound.cpp:395-408)
// loads consecutive files until the first missing one, and SB1 US ships SOUND000..SOUND065; worlds
// 101..110 (src/event.cpp, m_multi + 100).
const Rules SB1{"sb1",0x53423155,100,10,246,187,291,65,Sb1Transient,Sb1StartType,Sb1DeleteType};
// SB2: games/sb2/src/pixtables.h, SOUND000..SOUND092, worlds 201..212 (games/sb2/src/event.cpp,
// m_multi + 200, 1..12).
const Rules SB2{"sb2",0x53423255,200,12,441,289,336,92,Sb2Transient,Sb2StartType,Sb2DeleteType};

namespace {
const char* Event(const uint8_t* p,const Context& c) {
    const Rules& r=*c.rules;
    const int x=S16(p+2),y=S16(p+4),v=S16(p+6),speed=int8_t(p[1]);
    switch (p[0]) {
    case 20: case 21:
        if (!(p[0]==20 ? r.startType(v,speed) : r.deleteType(v))) return "object-type/speed";
        if (!Position(x,y)) return "object-position";
        if (!r.transient(v) && !c.reliable) return "object-reliability";
        return nullptr;
    case 30:
        if (x<0 || x>=6400 || y<0 || y>=6400 || v< -1 || v>=r.tiles) return "decor-range";
        return c.reliable ? nullptr : "decor-reliability";
    case 40: if (!Position(x,y)) return "sound-position"; [[fallthrough]];
    case 41: return v>=0 && v<=r.lastSound ? nullptr : "sound-id";
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
    if (!c.rules) return "rules";
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
            if (c.origin!=c.host || S16(p+2)<1 || S16(p+2)>c.rules->maps) return "lobby-owner/map";
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
    // GetBlupiIcon (src/decdesign.cpp:1510, games/sb2/src/decblupi.cpp:154) emits 2/10, Read(team)
    // selects 11..13; the rules give the sheet-specific icon counts.
    const int channel=p[16],icon=S16(p+12);
    if (channel!=2 && channel!=10 && channel!=11 && channel!=12 && channel!=13) return "pose-channel";
    if (icon< -1 || icon>=(channel==10 ? c.rules->elementIcons : c.rules->blupiIcons)) return "pose-icon";
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
} // namespace blupi::net::game
