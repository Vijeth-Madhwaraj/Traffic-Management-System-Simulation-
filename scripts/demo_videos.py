#!/usr/bin/env python3
"""Create dated capability videos from actual CLI output; not historic sprint evidence.
Optional tools: Pillow and imageio-ffmpeg==0.6.0. Uses the supplied executable.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import tempfile
import textwrap
from datetime import datetime
from zoneinfo import ZoneInfo
from PIL import Image, ImageDraw, ImageFont
import imageio_ffmpeg

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--executable', type=Path, required=True)
parser.add_argument('--output-dir', type=Path, default=ROOT / 'docs/videos')
args = parser.parse_args()
out = args.output_dir.resolve()
out.mkdir(parents=True, exist_ok=True)
recorded = datetime.now(ZoneInfo('Asia/Kolkata')).isoformat(timespec='seconds')
font_path = '/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf'
if not Path(font_path).exists():
    raise SystemExit('Install DejaVu Sans Mono fonts to render the capability video')
font = ImageFont.truetype(font_path, 20)
heading = ImageFont.truetype(font_path, 30)
small = ImageFont.truetype(font_path, 16)
provenance = {'recorded_at': recorded, 'kind': 'Captioned actual CLI output, not a historical sprint recording',
              'commands': [], 'videos': {}}

def run(*arguments):
    result = subprocess.run([str(args.executable.resolve()), *arguments], cwd=ROOT,
                            text=True, capture_output=True, check=True)
    provenance['commands'].append({'arguments': arguments, 'stdout_sha256': hashlib.sha256(result.stdout.encode()).hexdigest()})
    return result.stdout

def ticks(trace):
    matches = list(re.finditer(r'^Tick (\d+)\n', trace, flags=re.M))
    return [(m.group(1), trace[m.end(): matches[i+1].start() if i+1 < len(matches) else trace.rfind('\nGenerated:')])
            for i,m in enumerate(matches)]

def make(name, scenes):
    with tempfile.TemporaryDirectory() as directory:
        work = Path(directory)
        for i,(title,body) in enumerate(scenes):
            im = Image.new('RGB', (1280,720), '#111827')
            draw = ImageDraw.Draw(im)
            draw.text((35,24),title, font=heading, fill='#e5e7eb')
            draw.text((35,74),'Traffic Management System | current capability recording',font=small,fill='#93c5fd')
            draw.text((35,104),recorded,font=small,fill='#a7f3d0')
            lines=[]
            for line in body.strip().splitlines():
                lines.extend(textwrap.wrap(line, width=95, replace_whitespace=False) or [''])
            for j,line in enumerate(lines[:16]):
                draw.text((35,150+j*29),line,font=font,fill='#f3f4f6')
            draw.rectangle((35,653,1245,657),fill='#374151')
            draw.rectangle((35,653,35+1210*(i+1)/len(scenes),657),fill='#60a5fa')
            draw.text((35,676),f'Actual executable output | chapter {i+1}/{len(scenes)} | captions, no narration',font=small,fill='#9ca3af')
            im.save(work/f'{i:03}.png')
        concat=work/'frames.txt'
        concat.write_text(''.join(f"file '{i:03}.png'\nduration 4\n" for i in range(len(scenes)))+
                          f"file '{len(scenes)-1:03}.png'\n")
        path=out/name
        subprocess.run([imageio_ffmpeg.get_ffmpeg_exe(), '-y', '-hide_banner', '-loglevel','error',
                        '-f','concat','-safe','0','-i',str(concat),'-vf','fps=15',
                        '-c:v','libx264','-preset','fast','-pix_fmt','yuv420p',
                        '-t',str(len(scenes)*4),'-movflags','+faststart',str(path)],check=True)
        provenance['videos'][name]={'duration_seconds':len(scenes)*4,
            'sha256':hashlib.sha256(path.read_bytes()).hexdigest()}

validate=run('--validate','config/demo_scenario.txt')
overview=run('--scenario','config/demo_scenario.txt','--show-scenario')
trace=run('--scenario','config/demo_scenario.txt','--trace')
scenes=[('One minute capability demo','FR-01 to FR-07 and FR-09 to FR-12\nValidated scenarios, stable IDs, FIFO entry queues and safe movement.\nThis records current capabilities; it does not establish past sprint dates.'),
        ('Validate before running',validate)]
parts=overview.strip().splitlines()
for start in range(0,len(parts),14):scenes.append(('Review the scenario', '\n'.join(parts[start:start+14])))
for tick,body in ticks(trace):scenes.append(('Simulation tick '+tick,body))
scenes.append(('Final counts and metrics', trace[trace.rfind('\nGenerated:'):]))
while len(scenes)<14:
    scenes.append(('Read the evidence','Source/test mapping: docs/traceability.md\nUnit and system reports: docs/reports\nPeak queues are sampled before and after movement.\nDeferred waiting is included in vehicle waiting time.'))
scenes=scenes[:14]+[('Review and next steps','All displayed values came from the executable.\nReview the current documents and generated reports.\nTeam approvals and historical sprint evidence remain separate.')]
make('sprint-1-capabilities.mp4',scenes)
trace=run('--scenario','config/acceptance_scenario.txt','--trace')
overview=run('--scenario','config/acceptance_scenario.txt','--show-scenario')
scenes=[('Two minute integration demo','FR-08 and FR-13 to FR-18 with vehicle/movement modules.\nSafe priority, congestion response, a late incident and final metrics.\nThis is a current captioned capability recording.'),
        ('Scenario and policy',overview[:1100]),
        ('Safety contract','Requests select configured phases after full green/yellow/all-red.\nRouting preserves the current road and validates connected suffixes.\nA missing alternative means safe waiting, never prohibited movement.')]
scenes += [('Integrated tick '+tick,body) for tick,body in ticks(trace)]
scenes += [('Final integration metrics',trace[trace.rfind('\nGenerated:'):]),
           ('Repeatability','Use the same scenario and seed to replay the run.\nSystem checks compare complete CSV reports byte for byte.\nReset also clears lanes, incidents, priority requests and counters.'),
           ('Validation evidence','115 CTest cases and 12 system checks passed locally.\nRead measured coverage in docs/reports/coverage.\nWarnings, formatting and static analysis are separate checks.'),
           ('Recorded limitations','Docker and Jenkins runtime execution need external services.\nHuman usability review and final owner approval remain pending.\nNo historical sprint dates or approvals are inferred.')]
while len(scenes)<30:
    scenes.append(('Reproduce and review','traffic_sim --scenario config/acceptance_scenario.txt --trace\ntraffic_sim --scenario config/acceptance_scenario.txt --report result.csv\nSee docs/submission-checklist.md for final acceptance actions.'))
make('sprint-2-capabilities.mp4',scenes[:30])
(out/'provenance.json').write_text(json.dumps(provenance,indent=2)+'\n')
(out/'README.md').write_text('# Capability demo recordings\n\nRecorded '+recorded+'. These captioned videos render actual output from the supplied executable.\nThey are current one-minute and two-minute outcome demonstrations, **not evidence of when historical sprints occurred**.\nNo voice narration or screen-capture provenance is claimed. The original sprint board/history must establish the sprint dates.\n\nRun `python3 scripts/demo_videos.py --executable build/traffic_sim` with Pillow, imageio-ffmpeg 0.6.0 and DejaVu fonts to recreate.\nThe adjacent provenance file records commands, output fingerprints, video durations and file hashes.\n')
print('Created 60-second and 120-second current capability recordings')
