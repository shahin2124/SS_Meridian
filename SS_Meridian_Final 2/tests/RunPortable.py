"""Portable regression driver. Optional --render uses macOS offline OpenGL.
No tests or fixture shortcuts are compiled into the production application.
"""
from pathlib import Path
import argparse, os, shutil, subprocess, sys
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('--render',action='store_true');args=parser.parse_args()
build=root/'.qa';build.mkdir(exist_ok=True)
cxx=os.environ.get('CXX') or shutil.which('clang++') or shutil.which('g++')
if not cxx:raise SystemExit('Install/use an existing C++11 compiler to run portable tests; VS2013 builds use the solution.')
base=[cxx,'-std=c++11','-Wno-deprecated-declarations','-I'+str(root/'vendor'),'-I'+str(root/'src/m3')]
m1=['src/m1/'+x for x in ['Game.cpp','Scenes.cpp','Assets.cpp','Navigation.cpp','Animation.cpp','GameplayUpgrade.cpp']]
m3=['src/m3/Mission3.cpp','src/m3/Mission3Config.cpp']
jobs=[('M1Smoke',['tests/m1/Smoke.cpp','src/Image.cpp']+m1),('M2Logic',['tests/m2/LogicTests.cpp']),('M2Combat',['tests/m2/CombatSimulation.cpp']),('M3Logic',['tests/m3/Mission3Tests.cpp']+m3)]
if sys.platform=='darwin':jobs.append(('M3Loading',['tests/m3/StartupLoadingTests.cpp','src/m3/Mission3Assets.cpp','src/Image.cpp']))
subprocess.run([sys.executable,str(root/'tests/ValidateIntegration.py')],check=True)
for name,files in jobs:
 exe=build/name;subprocess.run(base+[str(root/f) for f in files]+['-o',str(exe)],check=True)
 run=subprocess.run([str(exe)],cwd=root,capture_output=True,text=True);(build/(name+'.log')).write_text(run.stdout+run.stderr)
 print(name+': '+('PASS' if run.returncode==0 else 'FAIL'));print(run.stdout if run.returncode else run.stdout.splitlines()[-1])
 if run.returncode:print(run.stderr);raise SystemExit(run.returncode)
if args.render:
 if sys.platform!='darwin':raise SystemExit('--render requires macOS OpenGL/GLUT frameworks.')
 (root/'qa_rendered').mkdir(exist_ok=True)
 files=['tests/HostIntegrationQA.cpp','src/Image.cpp']+m1+['src/m1/Renderer.cpp']+m3+['src/m3/Mission3Assets.cpp','src/m3/Mission3Renderer.cpp']
 exe=build/'HostIntegrationQA';subprocess.run(base+[str(root/f) for f in files]+['-framework','OpenGL','-framework','GLUT','-o',str(exe)],check=True)
 run=subprocess.run([str(exe)],cwd=root,capture_output=True,text=True);(build/'HostIntegrationQA.log').write_text(run.stdout+run.stderr);print(run.stdout+run.stderr);run.check_returncode()
print('All requested portable checks passed. Native VS2013/Windows validation is separate.')
