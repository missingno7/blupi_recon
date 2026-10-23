"""RTM key/hash at 407f5c/40801a; same algebra as SP3 405301/4053bf.

General tuples XOR their children's CACHED key low halves (408134), rather
than recursively recomputing them. A shift preserves each cached constant.
"""
def fold(n):
    n &= 0xffffffff
    return ((n&65535)^(n>>16))&65535

def hash16(t,delta=0,legacy=False):
    k=t['kind']
    if k in [1,2,3]:
        if t.get('expr'):return hash16(t['expr'],delta,legacy)
        if t.get('desc_kind') in ('03',3):return t.get('desc_number',t['key'])&65535
        s=t['sym']['id']
        return fold(s+delta) if s else t['key']&65535
    if k==4:return fold(t['sym']['id']+delta)
    if k in [5,6]:
        return fold(t['offset'])^(t['op']&65535)^(hash16(t['base'],delta,legacy) if t.get('base') else 0)
    if k==7:return fold(t['value'][0]^t['value'][1])
    if k==8:return fold(int.from_bytes(bytes.fromhex(t['value_bytes'])[:4],'little'))
    if k in [9,10]:return 0
    h=t['op']&65535
    for x in t.get('inputs',[]):
        if legacy:
            h ^= hash16(x,delta,True)
        else:
            h ^= x['key']&65535
            if delta:h ^= hash16(x,0)^hash16(x,delta)
    return h

def key(t,delta=0):return (t['key']&0xffff0000)|hash16(t,delta)

if __name__=='__main__':
    import json,sys
    from pathlib import Path
    for f in sys.argv[1:]:
        d=json.loads(Path(f).read_text());n=bad=0;examples=[]
        for x in d:
            for name in ['left','right']:
                t=x[name];n+=1
                if hash16(t)!=(t['key']&65535):
                    bad+=1
                    if len(examples)<6:examples.append([t['kind'],hex(t['key']),hex(hash16(t))])
        print(f,n,'roots',bad,'mismatched',examples)
