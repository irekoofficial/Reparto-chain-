import numpy as np, scipy.signal as ss
x=np.fromfile(__import__('sys').argv[1],dtype=np.float32).reshape(-1,2).astype(np.float64); sr=44100
L,R=x[:,0],x[:,1]; M=(L+R)/2; S=(L-R)/2
def rms(a):return 20*np.log10(np.sqrt(np.mean(a**2))+1e-12)
print("peak dBFS",20*np.log10(np.abs(x).max()),"RMS dB",rms(x))
# K-weighted LUFS
b1,a1=ss.butter(2,1500/(sr/2),'high') # approx
sh=ss.lfilter(*ss.butter(2,38/(sr/2),'high'),x,axis=0)
g=sh.copy()
bh=[1.53512485958697,-2.69169618940638,1.19839281085285];ah=[1,-1.69065929318241,0.73248077421585]
g=ss.lfilter(bh,ah,x,axis=0)
g=ss.lfilter([1,-2,1],[1,-1.99004745483398,0.99007225036621],g,axis=0)
N=int(0.4*sr);H=int(0.1*sr)
bl=np.array([np.mean(g[i:i+N]**2,axis=0).sum() for i in range(0,len(g)-N,H)])
lb=-0.691+10*np.log10(bl+1e-12)
a=lb[lb>-70];rel=10*np.log10(np.mean(10**(a/10)))-10
a2=lb[lb>rel];print("LUFS integrated",-0.691+10*np.log10(np.mean(10**((a2)/10)) ) if False else 10*np.log10(np.mean(10**(a2/10))))
print("LRA approx",np.percentile(a2,95)-np.percentile(a2,10))
print("crest dB",20*np.log10(np.abs(x).max())-rms(x))
print("clipped>0.999 frac",np.mean(np.abs(x)>0.999))
print("stereo corr",np.corrcoef(L,R)[0,1],"side/mid dB",rms(S)-rms(M))
f,P=ss.welch(M,sr,nperseg=16384);Pd=10*np.log10(P+1e-20)
bands=[(20,40),(40,60),(60,100),(100,200),(200,400),(400,800),(800,1600),(1600,3150),(3150,6300),(6300,12500),(12500,20000)]
tot=None
for lo,hi in bands:
    e=P[(f>=lo)&(f<hi)].sum();print(f"{lo}-{hi}Hz: {10*np.log10(e):.1f}")
fs,PS=ss.welch(S,sr,nperseg=16384)
for lo,hi in [(20,150),(150,1000),(1000,6000),(6000,20000)]:
    print("side",lo,hi,10*np.log10(PS[(fs>=lo)&(fs<hi)].sum()/P[(f>=lo)&(f<hi)].sum()))
# centroid & rolloff
cum=np.cumsum(P)/P.sum();print("rolloff95",f[np.searchsorted(cum,.95)],"rolloff99",f[np.searchsorted(cum,.99)])
# short-term dynamics
w=int(0.05*sr);r=np.array([rms(M[i:i+w]) for i in range(0,len(M)-w,w)])
print("rms50ms p10/p50/p90",np.percentile(r,[10,50,90]))
# tempo
env=np.abs(M);env=ss.decimate(env,100);env-=env.mean()
ac=np.correlate(env,env,'full')[len(env)-1:];fs2=sr/100
lo,hi=int(fs2*60/200),int(fs2*60/60);k=lo+np.argmax(ac[lo:hi]);print("tempo~",60*fs2/k)
