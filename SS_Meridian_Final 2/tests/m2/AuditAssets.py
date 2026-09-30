"""Optional audit tool: Python 3 + Pillow. Not required to build or run the game."""
from pathlib import Path
import re, json, hashlib, collections, xml.etree.ElementTree as ET
from PIL import Image
root=Path(__file__).resolve().parents[2]
header=(root/'src/m2/Assets.h').read_text()
paths=re.findall(r'\{"(assets/[^\"]+\.png)"',header)
assert len(paths)==89 and len(set(paths))==89
seq=collections.defaultdict(list);buttons=collections.defaultdict(list);audit=[]
for rel in paths:
 p=root/rel;assert p.is_file(),rel
 with Image.open(p) as im:
  im.load();assert im.format=='PNG'
  if '/background/' not in rel and not rel.endswith('mission2_dragon_battle_background.png'):
   assert im.mode=='RGBA' and im.getchannel('A').getextrema()[0]==0,rel
  m=re.search(r'_(0[1-6])\.png$',rel)
  if m:seq[rel[:-6]].append((int(m[1]),im.size))
  m=re.search(r'_(normal|hover|pressed)\.png$',rel)
  if m:buttons[rel[:m.start()]].append((m[1],im.size))
  audit.append(dict(path=rel,size=list(im.size),mode=im.mode,sha256=hashlib.sha256(p.read_bytes()).hexdigest()))
for group,entries in seq.items():assert [n for n,s in entries]==list(range(1,7)) and len({s for n,s in entries})==1,group
for group,entries in buttons.items():assert {n for n,s in entries}=={'normal','hover','pressed'} and len({s for n,s in entries})==1,group
assert len(seq)==10 and len(buttons)==5
for p in root.glob('*.vcxproj*'):ET.parse(p)
project=(root/'SS_Meridian.vcxproj').read_text();assert '<PlatformToolset>v120</PlatformToolset>' in project and '<CharacterSet>MultiByte</CharacterSet>' in project and 'MachineX86' in project and 'x64' not in project

print('PASS: 89 readable PNG paths; expected transparency; 10 complete 01-06 sequences; 5 complete button groups; VS project XML/v120/Win32/MultiByte configuration.')
