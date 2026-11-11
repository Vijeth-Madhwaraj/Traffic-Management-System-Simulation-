#!/usr/bin/env python3
"""Verify all FR rows refer to existing source and executed Linux test names."""
import argparse
from pathlib import Path
import xml.etree.ElementTree as ET

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--junit', type=Path, default=root / 'docs/reports/coverage/tests.xml')
args = parser.parse_args()
names = {case.attrib['name'] for case in ET.parse(args.junit).getroot().findall('.//testcase')}
seen = set()
for line in (root / 'docs/traceability.md').read_text().splitlines():
    if not line.startswith('| FR-'):
        continue
    ident, owner, sources, test, result = [value.strip() for value in line.strip('|').split('|')]
    if ident in seen:
        raise SystemExit('Duplicate requirement: ' + ident)
    seen.add(ident)
    for source in sources.split('; '):
        if not (root / source).is_file():
            raise SystemExit('Missing source: ' + source)
    if test.strip('`') not in names:
        raise SystemExit('Test was not executed: ' + test)
if seen != {f'FR-{i:02}' for i in range(1, 19)}:
    raise SystemExit('Missing or unexpected functional requirement IDs')
print('18/18 FR source and executed-test mappings verified')
