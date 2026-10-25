#include "net/session.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <thread>
using namespace blupi::net;
static void Check(bool ok,const char* what) { if (!ok) { std::fprintf(stderr,"FAIL %s\n",what); std::exit(1); } }
template<class F> static bool Until(F f,unsigned ms=1000) {
    auto end=std::chrono::steady_clock::now()+std::chrono::milliseconds(ms);
    do { ServiceAll(); if (f()) return true; std::this_thread::sleep_for(std::chrono::milliseconds(1)); } while (std::chrono::steady_clock::now()<end);
    return false;
}
int main() {
    Configure({}); Session host(123,1),client(123,1),other(123,1);
    uint16_t port=28641;
    while (port<28700 && !host.Create(port,"test room","host")) ++port;
    Check(port<28700,"host bind");
    auto ads=client.Probe({{"127.0.0.1",port}}); Check(ads.size()==1 && ads[0].name=="test room","bounded enumeration");
    Check(client.Join(ads[0],"client"),"handshake"); Check(client.Id()==2 && host.Id()==1,"assigned IDs");
    Check(other.Join(ads[0],"other"),"third member for relay");
    Message m;
    for (uint8_t i=0;i<20;++i) { uint8_t b[]={i,42}; Check(client.Send(b,2,true),"send reliable"); }
    for (int i=0;i<20;++i) {
        Check(Until([&]{return host.Receive(m);}),"host receives reliable"); Check(m.origin==2 && m.bytes[0]==i,"reliable order");
        Check(Until([&]{return other.Receive(m);}),"relay receives"); Check(m.origin==2 && m.bytes[0]==i,"relay original ID/order");
    }
    Check(!client.Receive(m),"no echo");
    uint8_t unreliable[]={99,42}; Check(host.Send(unreliable,2,false),"unsequenced send");
    Check(Until([&]{return client.Receive(m);}),"unsequenced receive"); Check(!m.reliable,"channel1 flags");
    Check(Until([&]{return other.Receive(m);}),"unsequenced other receive");
    Session mismatch(124,1); Check(!mismatch.Join(ads[0],"wrong",300),"protocol refusal");
    Session revision(123,2); Check(!revision.Join(ads[0],"wrong",300),"version refusal");
    Session fourth(123,1),fifth(123,1); Check(fourth.Join(ads[0],"fourth"),"four members");
    Check(!fifth.Join(ads[0],"fifth",300),"full refusal");
    const auto before=std::chrono::steady_clock::now(); other.Close(80);
    Check(std::chrono::steady_clock::now()-before<std::chrono::milliseconds(250),"bounded close");
    Departure d; Check(Until([&]{return host.Departed(d);}) && d.member==3 && !d.hostLost,"host disconnect event");
    Check(Until([&]{return client.Departed(d);}) && d.member==3,"relayed disconnect event");
    host.Start(); Check(!fifth.Join(ads[0],"late",300),"late join refused");
    Check(fifth.Probe({{"127.0.0.1",port}}).empty(),"started session not enumerated");
    host.Close(80); Check(Until([&]{return client.Departed(d);}) && d.hostLost,"host loss event");
    client.Close(); fourth.Close();
    std::puts("ENet handshake/order/relay/refusals/disconnect/close passed");
}
