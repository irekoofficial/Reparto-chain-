import numpy as np, scipy.signal as ss, json, sys
sr=44100
def feat(path):
    x=np.fromfile(path,dtype=np.float32).reshape(-1,2).astype(np.float64)
    L,R=x[:,0],x[:,1];M=(L+R)/2;S=(L-R)/2
    g=ss.lfilter([1.53512485958697,-2.69169618940638,1.19839281085285],[1,-1.69065929318241,0.73248077421585],x,axis=0)
    g=ss.lfilter([1,-2,1],[1,-1.99004745483398,0.99007225036621],g,axis=0)
    N=int(.4*sr);H=int(.1*sr)
    bl=np.array([np.mean(g[i:i+N]**2,axis=0).sum() for i in range(0,len(g)-N,H)])
    lb=-0.691+10*np.log10(bl+1e-12);a=lb[lb>-70]
    rel=-0.691+10*np.log10(np.mean(10**(a/10)))-10;a2=lb[lb>rel]
    lufs=-0.691+10*np.log10(np.mean(10**(a2/10)))
    pk=20*np.log10(np.abs(x).max());rms=10*np.log10(np.mean(x**2))
    f,P=ss.welch(M,sr,nperseg=16384);fs,PS=ss.welch(S,sr,nperseg=16384)
    edges=[(35,100),(100,300),(300,900),(900,2500),(2500,6000),(6000,16000)]
    b=np.array([10*np.log10(P[(f>=lo)&(f<hi)].sum()) for lo,hi in edges]);b-=b.mean()
    lowside=10*np.log10(PS[(fs>=20)&(fs<150)].sum()/P[(f>=20)&(f<150)].sum())
    return dict(lufs=lufs,peak=pk,crest=pk-rms-0+0,clip=float(np.mean(np.abs(x)>0.999)),
        corr=float(np.corrcoef(L,R)[0,1]),sm=10*np.log10(np.mean(S**2)/np.mean(M**2)),lowside=lowside,bands=b.tolist(),lra=float(np.percentile(a2,95)-np.percentile(a2,10)))
if __name__=="__main__":
  out={f"r{i}":feat(f"refs/r{i}.raw") for i in range(8)}
  json.dump(out,open('feats.json','w'),indent=1)
  for k,v in out.items(): print(k,f"LUFS {v['lufs']:.1f} crest {v['crest']:.1f} clip {v['clip']*100:.2f}% corr {v['corr']:.2f} S/M {v['sm']:.1f} lowS {v['lowside']:.1f} LRA {v['lra']:.1f}",[round(z,1) for z in v['bands']])
