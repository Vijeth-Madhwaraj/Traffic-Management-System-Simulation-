#!/usr/bin/env python3
"""Build, test and report project line/branch coverage using matching GCC/gcov."""
import argparse
import gzip
import html
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]

def command(args, **kwargs):
    subprocess.run([str(a) for a in args], check=True, **kwargs)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-dir', type=Path, default=ROOT / 'build-coverage')
    parser.add_argument('--output-dir', type=Path, default=ROOT / 'docs/reports/coverage')
    parser.add_argument('--gtest-source', type=Path)
    args = parser.parse_args()
    build, output = args.build_dir.resolve(), args.output_dir.resolve()
    output.mkdir(parents=True, exist_ok=True)
    configure = ['cmake', '-S', ROOT, '-B', build, '-DBUILD_TESTING=ON',
                 '-DCMAKE_BUILD_TYPE=Debug', '-DTRAFFIC_ENABLE_COVERAGE=ON']
    if args.gtest_source:
        configure.append('-DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=' + str(args.gtest_source.resolve()))
    command(configure)
    command(['cmake', '--build', build, '--parallel', '2'])
    for path in build.rglob('*.gcda'):
        path.unlink()  # Never accumulate counters across report runs.
    command(['ctest', '--test-dir', build, '--output-on-failure',
             '--output-junit', output / 'tests.xml'])
    raw = build / 'gcov-json'
    raw.mkdir(exist_ok=True)
    records = {}
    for index, notes in enumerate(sorted(build.glob('CMakeFiles/traffic_*.dir/**/*.gcno'))):
        work = raw / str(index)
        work.mkdir(exist_ok=True)
        command(['gcov', '--json-format', '--branch-probabilities', '--branch-counts', notes],
                cwd=work, stdout=subprocess.DEVNULL)
        for path in work.glob('*.gcov.json.gz'):
            data = json.loads(gzip.decompress(path.read_bytes()))
            for source in data['files']:
                absolute = Path(source['file'])
                if not absolute.is_absolute():
                    absolute = Path(data['current_working_directory']) / absolute
                try:
                    name = absolute.resolve().relative_to(ROOT).as_posix()
                except ValueError:
                    continue
                if not name.startswith(('src/', 'include/traffic/')):
                    continue
                for line in source['lines']:
                    key = (name, line['line_number'])
                    # Header instantiations may appear in multiple translation units.
                    record = records.setdefault(key, {'count': 0, 'branches': {}})
                    record['count'] += line['count']
                    for branch_index, branch in enumerate(line.get('branches', [])):
                        bkey = (index, branch_index)
                        record['branches'][bkey] = branch['count']
    files = {}
    for (name, _), line in records.items():
        counts = files.setdefault(name, [0, 0, 0, 0])
        counts[0] += 1
        counts[1] += line['count'] > 0
        counts[2] += len(line['branches'])
        counts[3] += sum(count > 0 for count in line['branches'].values())
    if not files:
        raise RuntimeError('No project coverage data found')
    total = [sum(values[i] for values in files.values()) for i in range(4)]
    result = {'schema': 1, 'scope': 'src and include/traffic; tests and dependencies excluded',
              'branch_definition': 'All gcov-reported branches, including compiler exception paths; no exclusions',
              'totals': dict(zip(['lines', 'lines_hit', 'branches', 'branches_hit'], total)),
              'files': {name: dict(zip(['lines', 'lines_hit', 'branches', 'branches_hit'], values))
                        for name, values in sorted(files.items())}}
    (output / 'coverage.json').write_text(json.dumps(result, indent=2) + '\n')
    for title, offset, filename in [('Code coverage', 0, 'code-coverage.md'),
                                     ('Branch coverage', 2, 'branch-coverage.md')]:
        denominator, numerator = total[offset], total[offset + 1]
        percentage = 100 * numerator / denominator if denominator else 100.0
        rows = [f'# {title}', '', f'Measured on Linux with GCC/gcov; {result["scope"]}.', '',
                f'**{numerator}/{denominator} covered ({percentage:.2f}%).**', '',
                result['branch_definition'], '',
                '| Source | Covered | Total | Percent |', '|---|---:|---:|---:|']
        for name, values in sorted(files.items()):
            count, hit = values[offset:offset + 2]
            rows.append(f'| `{name}` | {hit} | {count} | {100 * hit / count if count else 100:.2f}% |')
        (output / filename).write_text('\n'.join(rows) + '\n')
    rows = ''.join(f'<tr><td>{html.escape(name)}</td><td>{v[1]}/{v[0]}</td><td>{v[3]}/{v[2]}</td></tr>'
                   for name, v in sorted(files.items()))
    (output / 'index.html').write_text('<!doctype html><meta charset="utf-8"><title>Traffic coverage</title>'
        '<style>body{font:16px system-ui;margin:2rem}td,th{padding:.4rem;text-align:left}tr:nth-child(even){background:#eee}</style>'
        '<h1>Traffic project coverage</h1><p>Actual gcov counts; dependencies and tests excluded. '
        'Branch counts include compiler exception paths.</p><table><tr><th>Source</th><th>Lines</th><th>Branches</th></tr>'
        + rows + '</table>')
    print(json.dumps(result['totals']))

if __name__ == '__main__':
    main()
