import json,subprocess,numpy as np,feat
F=json.load(open('feats.json'))
names={0:("Dj Honda","Honda"),1:("Befocus - Un Tin","Befocus Un Tin"),2:("Befocus - Cosita","Befocus Cosita"),3:("Dichavate","Dichavate"),
 4:("El Bandolero","El Bandolero"),5:("Mauro El Codigo Secreto","Mauro Codigo Secreto"),6:("Lobo Malo","Lobo Malo"),7:("Dave Produce","Dave Produce")}
keys=sorted(F);avg=np.mean([F[k]['bands'] for k in keys],axis=0);avgsm=np.mean([F[k]['sm'] for k in keys])
def params(k):
    f=F[k];eq=np.clip(0.8*(np.array(f['bands'])-avg),-4,4)
    p={f"mEq{i+1}":round(float(eq[i]),1) for i in range(6)}
    p['mWidth']=round(float(np.clip(10**((f['sm']-avgsm)/20),0.75,1.3)),2)
    p['mBassMono']=int(np.clip(100+(-f['lowside']-15)*8,40,160))
    p['mClip']=round(min(0.6,0.25+f['clip']*100*0.25),2)
    p['mGlueRat']=round(float(np.clip(1.6+(12.6-f['crest'])*0.4,1.5,2.4)),1)
    p['mGlueThr']=-14;p['mCeiling']=-0.3;p['vocalOn']=0;p['masterOn']=1
    return p
def run(k,p,drive):
    pre=-14-F[k]['lufs']
    args=['./test_dsp',f'refs/{k}.raw','o.raw','0',str(pre)]+[f'{a}={b}' for a,b in p.items() if not a.startswith('mEq')]+[f'mDrive={drive}']
    subprocess.run(args,check=True);return feat.feat('o.raw')
res={}
for i,k in enumerate(keys):
    p=params(k);tgt=F[k]['lufs'];lo,hi=0.,20.
    for _ in range(9):
        mid=(lo+hi)/2;l=run(k,p,mid)['lufs']
        if l<tgt:lo=mid
        else:hi=mid
    p['mDrive']=round(mid,1);o=run(k,p,mid)
    print(k,names[i][0],'drive',p['mDrive'],f"LUFS out {o['lufs']:.1f} (ref {tgt:.1f}) crest {o['crest']:.1f} (ref {F[k]['crest']:.1f})",{a:b for a,b in p.items() if a not in('vocalOn','masterOn','mCeiling','mGlueThr')})
    res[k]=p
json.dump(res,open('presets_master.json','w'),indent=1)
