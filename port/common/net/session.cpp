#include "net/session.h"
#include <enet/enet.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <filesystem>
#include <fstream>
#include <map>
#include <thread>

namespace blupi::net {
namespace {
using Clock = std::chrono::steady_clock;
constexpr uint32_t Magic = 0x4e455431;
enum Kind { Hello = 1, Welcome, Refuse, Members, Data, Query, Answer, Gone };
constexpr size_t Header = 36, MaxQueue = 1024;
Options options;
uint32_t currentTick = 0;
uint32_t testSent = 0;
std::vector<Session*> sessions;
bool Init() { static bool ok = enet_initialize() == 0; return ok; }
void Put(std::vector<uint8_t>& b, size_t at, uint32_t n) {
    for (int i = 0; i < 4; ++i) b[at+i] = uint8_t(n >> (8*i));
}
uint32_t Get(const uint8_t* b, size_t at) {
    uint32_t n = 0; for (int i = 0; i < 4; ++i) n |= uint32_t(b[at+i]) << (8*i); return n;
}
bool Text(const uint8_t* b, size_t n) { return n > 0 && n <= 100 && b[n-1] == 0 && std::memchr(b, 0, n) == b+n-1; }
}
bool ParseEndpoint(const std::string& value, Endpoint& out) {
    auto colon = value.find(':');
    if (value.empty() || (colon != std::string::npos && value.find(':', colon+1) != std::string::npos)) return false;
    out.host = value.substr(0, colon);
    if (out.host.empty()) return false;
    if (colon != std::string::npos) {
        const std::string port = value.substr(colon+1);
        if (port.empty() || port.find_first_not_of("0123456789") != std::string::npos) return false;
        unsigned long n = std::strtoul(port.c_str(), nullptr, 10);
        if (n == 0 || n > 65535) return false;
        out.port = uint16_t(n);
    }
    return true;
}
void Configure(const Options& o) { options = o; }
const Options& Configuration() { return options; }

struct Session::Impl {
    uint32_t protocol, version, epoch = 0, id = 0, nextId = 2, sequence = 0;
    ENetHost* socket = nullptr;
    ENetPeer* server = nullptr;
    bool host = false, active = false, started = false;
    std::string name, player, error;
    std::array<uint32_t,4> members{};
    struct Peer { uint32_t id = 0; unsigned count = 0; Clock::time_point since = Clock::now(); };
    std::map<ENetPeer*,Peer> peers;
    std::deque<Message> incoming;
    std::deque<Departure> departures;
    std::map<uint32_t,uint32_t> received;
    uint32_t released[2] = {0,0};

    std::vector<uint8_t> Packet(Kind kind, uint32_t origin, uint32_t seq, const void* body, size_t n) {
        std::vector<uint8_t> b(Header+n);
        Put(b,0,Magic); Put(b,4,protocol); Put(b,8,version); Put(b,12,kind);
        Put(b,16,epoch); Put(b,20,origin); Put(b,24,seq); Put(b,28,currentTick); Put(b,32,uint32_t(n));
        if (n) std::memcpy(b.data()+Header,body,n);
        return b;
    }
    bool Write(ENetPeer* peer, const std::vector<uint8_t>& b, bool reliable = true) {
        ENetPacket* packet = enet_packet_create(b.data(), b.size(), reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED);
        if (!packet) return false;
        if (enet_peer_send(peer, reliable ? 0 : 1, packet) < 0) { enet_packet_destroy(packet); return false; }
        return true;
    }
    void Broadcast(const std::vector<uint8_t>& b, bool reliable = true, ENetPeer* except = nullptr) {
        for (auto& [peer, state] : peers) if (state.id && peer != except && !Write(peer,b,reliable)) Fail("reliable enqueue failed");
    }
    void Fail(const char* why) {
        if (active) departures.push_back({1,true});
        active = false; error = why;
    }
    void Membership() {
        std::vector<uint8_t> b(16);
        for (unsigned i = 0; i < 4; ++i) Put(b,4*i,members[i]);
        Broadcast(Packet(Members,1,0,b.data(),b.size()));
    }
    void Reject(ENetPeer* peer, const char* why) {
        Write(peer,Packet(Refuse,1,0,why,std::strlen(why)+1));
        enet_peer_disconnect_later(peer,0);
    }
    void Accept(ENetPeer* peer, const uint8_t* b, size_t n, bool reliable) {
        if (n < Header || Get(b,0) != Magic || Get(b,32) != n-Header) return;
        const auto kind = Kind(Get(b,12));
        const uint32_t origin = Get(b,20), seq = Get(b,24);
        const uint8_t* body = b+Header; const size_t length = n-Header;
        if (Get(b,4) != protocol || Get(b,8) != version) {
            if (host) Reject(peer,"protocol/version mismatch");
            else if (kind == Refuse) { error = "protocol/version mismatch"; }
            return;
        }
        if (host && (kind == Hello || kind == Query)) {
            if (!Text(body,length)) { Reject(peer,"invalid name"); return; }
            auto& state = peers[peer];
            if (state.id) return;
            if (started) { Reject(peer,"session started"); return; }
            auto slot = std::find(members.begin(),members.end(),0);
            if (slot == members.end() && kind != Query) { Reject(peer,"session full"); return; }
            uint32_t assigned = 0;
            if (kind == Hello) { assigned = nextId++; *slot = assigned; state.id = assigned; }
            std::vector<uint8_t> welcome(20+name.size()+1);
            Put(welcome,0,assigned);
            for (unsigned i=0;i<4;++i) Put(welcome,4+4*i,members[i]);
            std::memcpy(welcome.data()+20,name.c_str(),name.size()+1);
            // A welcome carries the host's current data sequence: the joiner never receives earlier
            // data, so the test ingress gate must not wait for it.
            Write(peer,Packet(kind == Query ? Answer : Welcome,1,kind == Query ? 0 : sequence,welcome.data(),welcome.size()));
            if (kind == Query) enet_peer_disconnect_later(peer,0); else Membership();
            return;
        }
        if (!host && kind == Refuse) {
            if (Text(body,length)) error = reinterpret_cast<const char*>(body);
            return;
        }
        if (!host && kind == Welcome && length >= 21 && length <= 120 && Text(body+20,length-20)) {
            id = Get(body,0); epoch = Get(b,16); name = reinterpret_cast<const char*>(body+20);
            for (unsigned i=0;i<4;++i) members[i] = Get(body,4+4*i);
            received[1] = released[0] = Get(b,24);
            active = id > 1 && members[0] == 1 && std::find(members.begin(),members.end(),id) != members.end();
            return;
        }
        if (Get(b,16) != epoch || !active) return;
        if (!host && kind == Members && origin == 1 && length == 16) {
            for (unsigned i=0;i<4;++i) members[i] = Get(body,4*i);
            return;
        }
        if (!host && kind == Gone && origin == 1 && length == 4) {
            departures.push_back({Get(body,0),false});
            return;
        }
        if (kind != Data || length < 2 || length > 200 || seq == 0) return;
        if (host) {
            auto& state = peers[peer];
            if (!state.id || origin != state.id) return;
            if (Clock::now()-state.since >= std::chrono::seconds(1)) { state.count=0; state.since=Clock::now(); }
            if (++state.count > 512) { Reject(peer,"payload rate exceeded"); return; }
        } else if (origin == id || std::find(members.begin(),members.end(),origin) == members.end()) return;
        if (incoming.size() >= MaxQueue) { Fail("receive queue exceeded"); return; }
        incoming.push_back({origin,seq,Get(b,28),reliable,std::vector<uint8_t>(body,body+length)});
        received[origin] = std::max(received[origin],seq);
        if (host) Broadcast(std::vector<uint8_t>(b,b+n),reliable,peer);
    }
};

Session::Session(uint32_t protocol,uint32_t version) : p(new Impl) {
    p->protocol=protocol; p->version=version; sessions.push_back(this);
}
Session::~Session() { Close(); sessions.erase(std::remove(sessions.begin(),sessions.end(),this),sessions.end()); }
bool Session::Create(uint16_t port,const std::string& name,const std::string& player) {
    Close();
    if (!Init() || name.empty() || name.size()>99 || player.size()>19) return false;
    ENetAddress address{ENET_HOST_ANY,port};
    p->socket=enet_host_create(&address,8,2,0,0);
    if (!p->socket) { p->error="cannot bind UDP port"; return false; }
    p->host=true; p->active=true; p->id=1; p->members[0]=1; p->name=name; p->player=player;
    p->epoch=uint32_t(Clock::now().time_since_epoch().count()) | 1u;
    return true;
}
bool Session::Join(const Advertisement& ad,const std::string& player,unsigned deadlineMs) {
    Close(); if (!Init() || player.size()>19) return false;
    ENetAddress address{}; address.port=ad.endpoint.port;
    if (enet_address_set_host(&address,ad.endpoint.host.c_str()) < 0) return false;
    p->socket=enet_host_create(nullptr,1,2,0,0); if (!p->socket) return false;
    p->server=enet_host_connect(p->socket,&address,2,0); if (!p->server) { Close(); return false; }
    p->player=player;
    const auto deadline=Clock::now()+std::chrono::milliseconds(deadlineMs);
    while (!p->active && p->error.empty() && Clock::now()<deadline) { Service(2); ServiceAll(); }
    if (!p->active) { if (p->error.empty()) p->error="join timed out"; const auto error=p->error; Close(); p->error=error; return false; }
    return true;
}
std::vector<Advertisement> Session::Probe(const std::vector<Endpoint>& endpoints,unsigned deadlineMs) {
    std::vector<Advertisement> found;
    if (!Init()) return found;
    for (const auto& endpoint : endpoints) {
        ENetAddress address{}; address.port=endpoint.port;
        if (enet_address_set_host(&address,endpoint.host.c_str())<0) continue;
        ENetHost* probe=enet_host_create(nullptr,1,2,0,0); if (!probe) break;
        ENetPeer* peer=enet_host_connect(probe,&address,2,0);
        const auto deadline=Clock::now()+std::chrono::milliseconds(deadlineMs);
        bool done=false;
        while (peer && !done && Clock::now()<deadline) {
            ServiceAll();
            ENetEvent event{};
            if (enet_host_service(probe,&event,1)<=0) continue;
            if (event.type==ENET_EVENT_TYPE_CONNECT) {
                const char query[]="probe";
                p->Write(peer,p->Packet(Query,0,0,query,sizeof(query)));
            } else if (event.type==ENET_EVENT_TYPE_RECEIVE) {
                const auto* b=event.packet->data; const size_t n=event.packet->dataLength;
                if (n>=Header+21 && n<=Header+120 && Get(b,0)==Magic && Get(b,4)==p->protocol && Get(b,8)==p->version &&
                    Get(b,12)==Answer && Get(b,32)==n-Header && Text(b+Header+20,n-Header-20))
                    found.push_back({endpoint,reinterpret_cast<const char*>(b+Header+20),Get(b,16)});
                enet_packet_destroy(event.packet); done=true;
            } else if (event.type==ENET_EVENT_TYPE_DISCONNECT) done=true;
        }
        if (peer) enet_peer_reset(peer);
        enet_host_destroy(probe);
    }
    return found;
}
void Session::Service(unsigned waitMs) {
    if (!p->socket) return;
    ENetEvent event{};
    for (unsigned count=0;count<512 && enet_host_service(p->socket,&event,count ? 0 : waitMs)>0;++count) {
        if (event.type==ENET_EVENT_TYPE_CONNECT) {
            enet_peer_timeout(event.peer,8,1500,3000);
            if (p->host) p->peers.emplace(event.peer,Impl::Peer{});
            else p->Write(event.peer,p->Packet(Hello,0,0,p->player.c_str(),p->player.size()+1));
        } else if (event.type==ENET_EVENT_TYPE_RECEIVE) {
            p->Accept(event.peer,event.packet->data,event.packet->dataLength,(event.packet->flags & ENET_PACKET_FLAG_RELIABLE)!=0);
            enet_packet_destroy(event.packet);
        } else if (event.type==ENET_EVENT_TYPE_DISCONNECT) {
            if (p->host) {
                auto it=p->peers.find(event.peer);
                if (it!=p->peers.end()) {
                    uint32_t id=it->second.id;
                    p->peers.erase(it);
                    if (id) {
                        p->departures.push_back({id,false});
                        std::vector<uint8_t> gone(4); Put(gone,0,id);
                        p->Broadcast(p->Packet(Gone,1,0,gone.data(),gone.size()));
                        for (auto& member:p->members) if (member==id) member=0;
                        p->Membership();
                    }
                }
            } else p->Fail("host disconnected; host migration unsupported");
        }
    }
    enet_host_flush(p->socket);
}
bool Session::Send(const void* data,size_t n,bool reliable) {
    if (!p->active || n<2 || n>200 || !data) return false;
    reliable = reliable || options.testSync;
    auto b=p->Packet(Data,p->id,++p->sequence,data,n);
    if (options.testSync) testSent=p->sequence;
    if (p->host) p->Broadcast(b,reliable);
    else if (!p->Write(p->server,b,reliable)) { p->Fail("send enqueue failed"); return false; }
    enet_host_flush(p->socket);
    return p->active;
}
bool Session::Receive(Message& message) {
    Service();
    if (options.testSync) {
        std::stable_sort(p->incoming.begin(),p->incoming.end(),[](const Message& a,const Message& b) {
            if (a.tick!=b.tick) return a.tick<b.tick;
            if (a.origin!=b.origin) return a.origin<b.origin;
            return a.sequence<b.sequence;
        });
    }
    if (p->incoming.empty()) return false;
    const auto& first=p->incoming.front();
    if (options.testSync && (first.tick>=currentTick || first.origin>2 || first.sequence>p->released[first.origin-1])) return false;
    message=std::move(p->incoming.front()); p->incoming.pop_front(); return true;
}
bool Session::Departed(Departure& d) { if (p->departures.empty()) return false; d=p->departures.front(); p->departures.pop_front(); return true; }
void Session::Start() { p->started=true; }
void Session::Close(unsigned deadlineMs) {
    if (p->socket) {
        if (p->host) for (auto& [peer,state]:p->peers) enet_peer_disconnect_later(peer,0);
        else if (p->server) enet_peer_disconnect_later(p->server,0);
        // enet_peer_disconnect lowers connectedPeers as soon as the disconnect is queued, before it
        // is sent: wait until every peer has acknowledged it (state DISCONNECTED), within the deadline.
        auto pending=[this] {
            for (size_t i=0;i<p->socket->peerCount;++i)
                if (p->socket->peers[i].state!=ENET_PEER_STATE_DISCONNECTED) return true;
            return false;
        };
        const auto deadline=Clock::now()+std::chrono::milliseconds(deadlineMs);
        while (pending() && Clock::now()<deadline) { Service(1); ServiceAll(); }
        enet_host_flush(p->socket);
        enet_host_destroy(p->socket);
    }
    const auto protocol=p->protocol,version=p->version;
    p.reset(new Impl); p->protocol=protocol; p->version=version;
}
bool Session::Active() const { return p->active; }
bool Session::IsHost() const { return p->host; }
uint32_t Session::Id() const { return p->id; }
bool Session::Knows(uint32_t id) const { return id && std::find(p->members.begin(),p->members.end(),id)!=p->members.end(); }
const std::string& Session::Error() const { return p->error; }
void ServiceAll() { for (auto* s:sessions) s->Service(); }

namespace {
std::string ControlPath(const char* kind,long tick) {
    const char* dir=std::getenv("BLUPI_NET_TEST_DIR"); const char* role=std::getenv("BLUPI_NET_TEST_ROLE");
    if (!dir || !role || (std::strcmp(role,"0") && std::strcmp(role,"1"))) return {};
    return std::string(dir)+"/"+role+"-"+kind+"-"+std::to_string(tick);
}
}
bool BeginTestTick(long tick) {
    if (!options.testSync) return true;
    const auto path=ControlPath("grant",tick); if (path.empty()) return false;
    const auto deadline=Clock::now()+std::chrono::seconds(10);
    uint32_t counts[2]{};
    for (;;) {
        ServiceAll();
        std::ifstream file(path);
        if (file >> counts[0] >> counts[1]) break;
        if (Clock::now()>=deadline) return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    currentTick=uint32_t(tick);
    for (auto* s:sessions) {
        auto& p=*s->p;
        for (uint32_t origin=1;origin<=2;++origin) {
            while (p.active && origin!=p.id && p.received[origin]<counts[origin-1]) {
                ServiceAll(); if (Clock::now()>=deadline) return false;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            p.released[origin-1]=counts[origin-1];
        }
    }
    return true;
}
bool EndTestTick(long tick) {
    if (!options.testSync) return true;
    ServiceAll();
    uint32_t sent=testSent;
    for (auto* s:sessions) sent=std::max(sent,s->p->sequence);
    const auto path=ControlPath("done",tick); if (path.empty()) return false;
    std::ofstream file(path+".tmp"); file << sent << '\n'; file.close();
    std::error_code error; std::filesystem::rename(path+".tmp",path,error); return !error;
}
} // namespace blupi::net
