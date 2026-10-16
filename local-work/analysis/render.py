import math,subprocess,sys
def load(p):
    V=[];F=[]
    for ln in open(p):
        if ln.startswith('v '):
            q=ln.split();V.append(tuple(float(x) for x in q[1:4]))
        elif ln.startswith('f '):
            idx=[int(t.split('/')[0])-1 for t in ln.split()[1:]]
            for i in range(1,len(idx)-1):F.append((idx[0],idx[i],idx[i+1]))
    return V,F
def render(p,out,yaw,pitch,w=360,h=360):
    V,F=load(p)
    if not V:subprocess.run(["ffmpeg","-y","-loglevel","error","-f","lavfi","-i",f"color=c=0x12141a:s={w}x{h}","-frames:v","1",out.replace('.ppm','.png')]);return
    cx=sum(v[0] for v in V)/len(V);cy=sum(v[1] for v in V)/len(V);cz=sum(v[2] for v in V)/len(V)
    ya=math.radians(yaw);pa=math.radians(pitch)
    def pr(v):
        x,y,z=v[0]-cx,v[1]-cy,v[2]-cz
        xr=x*math.cos(ya)+z*math.sin(ya);zr=-x*math.sin(ya)+z*math.cos(ya)
        return xr,y*math.cos(pa)-zr*math.sin(pa),y*math.sin(pa)+zr*math.cos(pa)
    P=[pr(v) for v in V];xs=[p[0] for p in P];ys=[p[1] for p in P]
    s=min((w-40)/((max(xs)-min(xs))or 1),(h-40)/((max(ys)-min(ys))or 1))
    P2=[((p[0]-min(xs))*s+20,(max(ys)-p[1])*s+20) for p in P]
    L=(0.4,0.8,-0.45);ln=math.sqrt(sum(c*c for c in L));L=tuple(c/ln for c in L)
    fs=[]
    for a,b,c in F:
        A,B,C=P[a],P[b],P[c]
        ux,uy,uz=B[0]-A[0],B[1]-A[1],B[2]-A[2];vx,vy,vz=C[0]-A[0],C[1]-A[1],C[2]-A[2]
        n=(uy*vz-uz*vy,uz*vx-ux*vz,ux*vy-uy*vx);nl=math.sqrt(sum(t*t for t in n))or 1;n=tuple(t/nl for t in n)
        d=sum(n[i]*L[i] for i in range(3))
        if d<0:continue
        fs.append(((A[2]+B[2]+C[2])/3,0.25+0.75*max(0,d),(P2[a],P2[b],P2[c])))
    fs.sort(key=lambda f:f[0])
    img=[[(18,20,26) for _ in range(w)] for _ in range(h)]
    for zc,sh,(p0,p1,p2) in fs:
        col=(int(210*sh),int(160*sh),int(100*sh));pp=[(p0,p1),(p1,p2),(p2,p0)]
        for y in range(h):
            x2=[]
            for (ax,ay),(bx,by) in pp:
                if (ay<=y<by)or(by<=y<ay):
                    t=(y-ay)/(by-ay);x2.append(ax+t*(bx-ax))
            if len(x2)>=2:
                for x in range(max(0,int(min(x2))),min(w-1,int(max(x2)))+1):img[y][x]=col
    with open(out,'wb') as f:
        f.write(("P6\n%d %d\n255\n"%(w,h)).encode())
        for y in range(h):
            for x in range(w):
                r,g,b=img[y][x];f.write(bytes((r,g,b)))
    subprocess.run(["ffmpeg","-y","-loglevel","error","-i",out,out.replace('.ppm','.png')])
if __name__=="__main__":
    render(sys.argv[1],sys.argv[2],float(sys.argv[3]),float(sys.argv[4]))
