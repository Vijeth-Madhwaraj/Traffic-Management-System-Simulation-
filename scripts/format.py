#!/usr/bin/env python3
"""Check project C++ formatting with clang-format 18.1.8; use --fix to apply."""
import argparse
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--formatter', default='clang-format')
parser.add_argument('--fix', action='store_true')
args = parser.parse_args()
version = subprocess.check_output([args.formatter, '--version'], text=True)
if '18.1.8' not in version:
    raise SystemExit('Install the pinned formatter: python3 -m pip install clang-format==18.1.8')
files = [str(path) for folder in ['src', 'include', 'tests']
         for path in sorted((root / folder).rglob('*')) if path.suffix in ['.cpp', '.hpp']]
subprocess.run([args.formatter] + (['-i'] if args.fix else ['--dry-run', '--Werror']) + files,
               cwd=root, check=True)
