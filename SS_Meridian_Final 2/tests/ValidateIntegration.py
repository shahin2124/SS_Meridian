"""Dependency-free structural, asset-preservation, and VS2013 project audit."""
from pathlib import Path
import hashlib, json, re, xml.etree.ElementTree as ET
root=Path(__file__).resolve().parents[1]
ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'}
assert len(list(root.glob('*.sln')))==len(list(root.glob('*.vcxproj')))==1
p=ET.parse(root/'SS_Meridian.vcxproj')
configs=[x.attrib['Include'] for x in p.findall('.//m:ProjectConfiguration',ns)]
assert sorted(configs)==['Debug|Win32','Release|Win32']
for field,value in [('PlatformToolset','v120'),('CharacterSet','MultiByte'),('TargetMachine','MachineX86'),('ConfigurationType','Application'),('TargetName','SS_Meridian')]:
 assert p.find('.//m:'+field,ns).text==value
assert sorted(x.text for x in p.findall('.//m:RuntimeLibrary',ns))==['MultiThreaded','MultiThreadedDebug']
sources=[x.attrib['Include'].replace('\\','/') for x in p.findall('.//m:ClCompile[@Include]',ns)]
assert sorted(sources)==sorted(str(x.relative_to(root)) for x in (root/'src').rglob('*.cpp'))
text='\n'.join((root/x).read_text() for x in sources)
assert len(re.findall(r'\bint\s+main\s*\(',text))==1
assert text.count('#define STB_IMAGE_IMPLEMENTATION')==1
assert text.count('glutCreateWindow(')==text.count('glutMainLoop(')==1
assert not re.search(r'\b(WinMain|wglCreateContext|CreateProcess\w*|ShellExecute\w*|system)\s*\(',text)
for f in (root/'src').rglob('*'):
 if f.suffix in ('.h','.cpp','.inl'):
  for path in re.findall(r'"(assets/[^"\n]+\.png)"',f.read_text()):assert (root/path).is_file(),path
manifest=json.loads((root/'ASSET_PROVENANCE.json').read_text())
for entry in manifest:assert hashlib.sha256((root/entry['output']).read_bytes()).hexdigest()==entry['sha256'],entry['output']
menu=(root/'src/Menu.h').read_text()
assert menu.count('i < 3')==3 and 'i < 4' not in menu
assert 'Coming Soon' not in menu
assert 'drawTextureAlpha(texMissionPanel,' not in menu
m3=(root/'src/m3/Mission3.cpp').read_text()
assert 'case FinalBeachFade:' in m3 and 'go(FinalJourney)' in m3
# Bundled Windows runtime must be the supplied x86 DLL.
b=(root/'vendor/GLUT32.DLL').read_bytes();pe=int.from_bytes(b[60:64],'little')
assert b[pe:pe+4]==b'PE\0\0' and int.from_bytes(b[pe+4:pe+6],'little')==0x14c
print('PASS: one solution/target/main/window/loop/STB; all source files included; Win32/v120/MultiByte and consistent CRTs; three menu choices; ending retained; x86 GLUT DLL; all PNG literals; %d unchanged image/audio hashes.' % len(manifest))
