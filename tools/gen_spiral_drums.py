#!/usr/bin/env python3
"""Original procedural SPIRALFM1 drum assets. No external recordings or sample inputs.
Reproducible oscillator/noise synthesis; fixed seeds; mono PCM16 at 44.1 kHz.
Retuned for SPIRALFM1: swept FM kicks, inharmonic metal partials, band-passed noise bodies.
"""
import math, random, wave, struct, json, hashlib
from pathlib import Path
BASE=Path(__file__).resolve().parents[1]/'assets/spiralfm1/samples'
SR=44100
SEED=4711
def sound(role,kit):
    i=['PSY','FOREST','HITECH'].index(kit); rng=random.Random(SEED+i*313+7*sum(map(ord,role)))
    dur={'KICK':.30,'HAT':.115,'CLAP':.16,'PERC':.14,'SNARE':.15}[role]
    phase=0.; fmp=0.; lp=0.; lp2=0.; b1=0.; b2=0.; a=[]
    for n in range(int(SR*dur)):
        t=n/SR; noise=rng.uniform(-1,1)
        lp+=.16*(noise-lp); hp=noise-lp                       # 1-pole tonal split
        b1+=.24*(hp-b1); b2+=.24*(b1-b2)                      # 2-pole band-pass, for the metal voices
        if role=='KICK':
            # fundamental sweeps down a spiral, an FM sideband adds the click without a noise transient
            f=[58,52,66][i]+[188,232,300][i]*math.exp(-t/[.011,.016,.006][i])
            phase+=2*math.pi*f/SR
            fmp+=2*math.pi*f*[3.2,2.7,3.9][i]/SR
            body=math.sin(phase+math.sin(fmp)*[.9,.6,1.3][i]*math.exp(-t/.006))
            body*=math.exp(-t/[.075,.095,.05][i])
            click=hp*math.exp(-t/.0013)*[.14,.08,.22][i]
            v=math.tanh([1.7,1.9,1.5][i]*(body+click))
        elif role=='HAT':
            metal=sum(math.sin(2*math.pi*f*t+f*.0002) for f in [2947+163*i,4391+257*i,6607-113*i,9199+71*i])/4
            v=((hp*.62+metal*.38)+b2*.18)*math.exp(-t/[.021,.036,.014][i])
        elif role=='CLAP':
            burst=sum(math.exp(-(t-b)/.0035) for b in [0,.0085+i*.0012,.0175+i*.0022,.0265+i*.0018] if t>=b)
            v=(b1*1.5)*(burst*.5+math.exp(-t/[.029,.041,.021][i])*.4)
        elif role=='SNARE':
            tone=(math.sin(2*math.pi*(184+i*27)*t)+.45*math.sin(2*math.pi*(184+i*27)*1.48*t))
            v=tone*.3*math.exp(-t/.026)+(hp*.8+b2*.25)*math.exp(-t/[.038,.048,.027][i])
        else:
            # PERC: a pitched hit that curls down through its own first partial
            phase+=2*math.pi*(268+i*124+520*math.exp(-t/.008))/SR
            v=math.sin(phase+.9*math.sin(phase*2.41+t*.0007))*math.exp(-t/.028)+b2*.12*math.exp(-t/.005)
        v*=min(1,t/.0002)*min(1,(dur-t)/.008)
        a.append(v)
    pk=max(map(abs,a)); tgt={'KICK':.78,'HAT':.80,'CLAP':.86,'PERC':.78,'SNARE':.80}[role]
    return [round(v/pk*tgt*32767) for v in a]
def write(file,role,kit):
    samples=sound(role,kit)
    with wave.open(str(BASE/file),'wb') as w:
        w.setparams((1,2,SR,len(samples),'NONE','not compressed')); w.writeframes(struct.pack('<'+'h'*len(samples),*samples))
    return {'file':file,'role':role,'kit_name':kit,'sr':SR,'duration':len(samples)/SR,'sha256':hashlib.sha256((BASE/file).read_bytes()).hexdigest(),'source':'Original procedural synthesis: tools/gen_spiral_drums.py; no external audio input'}
def main():
    BASE.mkdir(parents=True,exist_ok=True)
    old=json.loads((BASE/'DRUM-SOURCES.json').read_text())['entries']
    entries=[]; assets=[]
    for kit in ['PSY','FOREST','HITECH']:
        for role in ['KICK','HAT','CLAP','PERC','SNARE']:
            file=f'{kit}-{role}.wav'; e=write(file,role,kit); assets.append(e)
            match=next((x for x in old if x.get('file')==file),None)
            if match: entries.append(dict(e,kit=match['kit'],note=match['note']))
    (BASE/'DRUM-SOURCES.json').write_text(json.dumps({'entries':entries},indent=2),encoding='utf-8')
    (BASE/'SOURCE.json').write_text(json.dumps(next(x for x in assets if x['file']=='FOREST-KICK.wav'),indent=2),encoding='utf-8')
    (BASE/'ORIGINAL-SOUNDS.json').write_text(json.dumps({'design':'SPIRALFM1 procedural drum collection (retuned from the Mycelium base set)','audio_inputs':[],'assets':assets},indent=2),encoding='utf-8')
    legacy=BASE/'LTWB-KICK.wav'
    if legacy.exists(): legacy.unlink()
    print('15 original drum assets generated; manifest hashes recorded.')
if __name__=='__main__':main()
