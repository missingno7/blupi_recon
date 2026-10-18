"""Independent P5 priority, ready insertion, and U/V issue-rule predictions.
Consumes observed DAGs and state, not recorded decisions, to predict each check.
This is a bounded model, not a replacement c2 or a claim of STRICT equality.
"""
import json,sys
import copy
from pathlib import Path

def shift(v,s):return (v<<s)&0xffffffff if s>=0 else v>>-s
def priority(n,w):
    f=n['flags']
    return sum([shift(n['height'],w[1]),shift(n['outgoing'],w[2]),
                shift((f>>5)&1,w[5]),shift((f>>3)&1,w[3]),shift((f>>4)&1,w[0]),
                shift((f>>6)&1,w[6]) if n['tuple'] and n['tuple']['flags'].startswith('0x400') else 0])&0xffffffff

def insertion(before,n):
    # The original ordinal is ascending; exact ordinal ties pass existing nodes.
    k=next((i for i,x in enumerate(before) if n['priority']>x['priority'] or
        n['priority']==x['priority'] and n['ordinal']<x['ordinal']),len(before))
    return [x['ptr'] for x in before[:k]]+[n['ptr']]+[x['ptr'] for x in before[k:]]

def can_pair(a,b):
    # Type flags live in the table's third dword. GP: 0=both,100=U,200=V,300=neither.
    if a['tuple']['flags'].startswith('0x400'):
        return a['pair_class']==0x102 and b['tuple']['op']==0x5f
    return a['pair_class'] in (0,0x100) and b['pair_class'] in (0,0x200)

def pipe(e):
    n=e['node'];kind=n['flags']&7;resources=e['resources']
    if n['tuple']['op']==0x46 and e['rep_block']>0:return -1
    free=lambda k:k<len(resources) and resources[k]['busy']<resources[k]['capacity']
    if kind>=len(resources):return 0 if all(free(i) for i in range(len(resources))) else -1
    if not e['slot']:return kind if free(kind) and free(kind+1) else -1
    return kind+1 if can_pair(e['first'],n) else -1

def schedule(w):
    """Replay a complete P5 all-hard-edge window from the input DAG alone."""
    if w['model']!=2 or w['preallocation'] or w['weights'][4]>=0:
        return {'excluded':'requires P5 post-allocation, zero priority boost'}
    ns=copy.deepcopy(w['nodes']);nodes={n['ptr']:n for n in ns}
    if any(n['soft_incoming'] or any(e['kind'] for e in n['edges']) for n in ns):
        return {'excluded':'soft/address-rewrite dependencies need additional modeling'}
    ready=[];busy=[0]*4;caps=[1,1,3,1];choices=[];history=[];cycle=0;rep_block=0
    def insert(n):
        ready.append(n);ready.sort(key=lambda q:(-q['priority'],q['ordinal']))
    def finish(n,when,delay=0):
        for e in n['edges']:
            d=nodes[e['dest']]
            d['earliest']=max(d['earliest'],when+delay+e['latency'])
            d['incoming']-=1
            if d['incoming']==0:insert(d)
    # Scheduler marks the synthetic start as scheduled, and only decrements its outgoing counts.
    for e in ns[0]['edges']:
        d=nodes[e['dest']];d['incoming']-=1
        if d['incoming']==0:insert(d)
    while ready and ready[0] is not ns[-1]:
        if cycle>100000:return {'excluded':'cycle limit'}
        chosen=[]
        for slot in range(w['issue_width']):
            winner=None;which=None
            for n in ready:
                if n is ns[-1]:break
                e=dict(node=n,slot=slot,first=chosen[0][0] if chosen else None,
                       resources=[dict(busy=a,capacity=b) for a,b in zip(busy,caps)],rep_block=rep_block)
                r=pipe(e)
                if r>=0 and cycle+max(0,busy[r])>=n['earliest']:
                    winner=n;which=r;break
            if winner is None:break
            history.append(dict(cycle=cycle,slot=slot,ready=[n['ordinal'] for n in ready],chosen=winner['ordinal']))
            ready.remove(winner);chosen.append((winner,which));choices.append((cycle,winner['ordinal']))
            finish(winner,cycle,max(0,busy[which]))
        for n,r in chosen:
            busy[r]=max(0,busy[r])+n.get('issue_cost',n.get('cost',0))
            if n['tuple']['op']==0x46:rep_block=2
        busy=[b-1 for b in busy];rep_block-=1;cycle+=1
    return {'choices':choices,'history':history}

def validate(path):
    d=json.loads(Path(path).read_text());counts={'height':0,'critical_path':0,'priority':0,'insertion':0,'pipe':0,'selected_first_eligible':0};bad=[]
    weights=None;trials=[]
    for i,e in enumerate(d['events']):
        stage=e['stage']
        if stage=='window':
            weights=e['weights'];trials=[]
            heights={};critical=set([e['nodes'][0]['ptr']])
            for n in reversed(e['nodes']):
                heights[n['ptr']]=max((edge['latency']+heights[edge['dest']]+1 for edge in n['edges']),default=0)
                counts['height']+=1
                if heights[n['ptr']]!=n['height']:bad.append([i,'height',n['ordinal'],heights[n['ptr']],n['height']])
            for n in e['nodes'][:-1]:
                if n['ptr'] in critical:
                    for edge in n['edges']:
                        if heights[n['ptr']]==edge['latency']+heights[edge['dest']]+1:critical.add(edge['dest'])
            for n in e['nodes']:
                counts['critical_path']+=1
                if bool(n['flags']&8)!=(n['ptr'] in critical):bad.append([i,'critical_path',n['ordinal']])
            for n in e['nodes']:
                pred=priority(n,weights);counts['priority']+=1
                if pred!=n['base_priority']:bad.append([i,'priority',n['ordinal'],pred,n['base_priority']])
        elif stage=='insert-result':
            counts['insertion']+=1;pred=insertion(e['before'],e['node']);got=[n['ptr'] for n in e['actual_queue']]
            if pred!=got:bad.append([i,'insertion',pred,got])
        elif stage=='pipe-try':
            counts['pipe']+=1;pred=pipe(e)
            if pred!=e['actual']:bad.append([i,'pipe',e['node']['ordinal'],pred,e['actual']])
            trials.append(e)
        elif stage=='select':
            # Pipe attempts are in ready-list order. Check the winner is the last tested node.
            # Earliest-time/soft-edge checks occur after the pipe check, so these are traced separately.
            counts['selected_first_eligible']+=1
            if not trials or trials[-1]['node']['ptr']!=e['node']['ptr'] or pipe(trials[-1])<0:
                bad.append([i,'selected',e['node']['ordinal']])
            trials=[]
    windows=[e for e in d['events'] if e['stage']=='window'];full=[]
    for w in windows:
        pred=schedule(w)
        if 'excluded' in pred:full.append(dict(window=w['window'],**pred));continue
        actual=[(e['cycle'],e['node']['ordinal']) for e in d['events'] if e['stage']=='select' and e['window']==w['window']]
        ok=pred['choices']==actual
        if 'prediction_before_schedule' in w:
            registered=w['prediction_before_schedule']
            if 'choices' in registered and [tuple(x) for x in registered['choices']]!=actual:
                bad.append(['registered-schedule',w['window']])
        full.append(dict(window=w['window'],match=ok,choices=len(actual),prediction=pred))
        if not ok:bad.append(['full-schedule',w['window'],pred['choices'],actual])
    result=dict(trace=str(path),checks=counts,full_schedules=full,failures=bad)
    print(json.dumps({k:v if k!='full_schedules' else [{a:b for a,b in x.items() if a!='prediction'} for x in v] for k,v in result.items()},indent=2));return result

if __name__=='__main__':
    r=validate(sys.argv[1]);Path(sys.argv[1]+'.validation.json').write_text(json.dumps(r,indent=2)+'\n')
    if r['failures']:sys.exit(1)
