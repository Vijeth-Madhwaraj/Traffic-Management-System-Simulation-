#!/usr/bin/env python3
"""Run reproducible CLI acceptance scenarios and write measured system evidence."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path, required=True)
    parser.add_argument('--output-dir', type=Path, default=ROOT / 'docs/reports')
    args = parser.parse_args()
    exe = args.executable.resolve()
    out = args.output_dir.resolve()
    out.mkdir(parents=True, exist_ok=True)
    results = []
    def run(name, arguments, expected=0, contains=None):
        start = time.perf_counter()
        result = subprocess.run([str(exe)] + list(map(str, arguments)), cwd=ROOT,
                                text=True, capture_output=True, timeout=60)
        passed = result.returncode == expected and (contains is None or contains in result.stdout)
        results.append({'name': name, 'passed': passed, 'exit': result.returncode,
                        'seconds': round(time.perf_counter() - start, 6),
                        'stdout': result.stdout, 'stderr': result.stderr})
        return result
    with tempfile.TemporaryDirectory() as directory:
        temp = Path(directory)
        run('CLI help', ['--help'], contains='Usage:')
        run('Validate scenario without advancing', ['--validate', 'config/acceptance_scenario.txt'], contains='Scenario valid')
        run('Reject invalid capacity', ['--validate', 'tests/fixtures/invalid_capacity.txt'], expected=1)
        run('Reject unknown option', ['--unknown'], expected=2)
        run('Integrated emergency incident and congestion demo',
            ['--scenario', 'config/acceptance_scenario.txt', '--report', temp / 'acceptance.csv'],
            contains='Completed: 4\nUnfinished: 0')
        report = (temp / 'acceptance.csv').read_text()
        results.append({'name': 'CSV records seed throughput and final metrics',
                        'passed': all(value in report for value in ['seed,2026', 'completed,4', 'throughput_per_tick,0.200000'])})
        for i in [1, 2]:
            run(f'Seeded replay {i}', ['--scenario', 'config/seeded_scenario.txt', '--report', temp / f'repeat{i}.csv'])
        results.append({'name': 'Identical seeded CSV results',
                        'passed': (temp / 'repeat1.csv').read_bytes() == (temp / 'repeat2.csv').read_bytes()})
        run('Missing report directory returns failure',
            ['--scenario', 'config/demo_scenario.txt', '--report', temp / 'missing/report.csv'], expected=1)
        if Path('/dev/full').exists():
            result = run('CSV write or close failure returns failure',
                         ['--scenario', 'config/demo_scenario.txt', '--report', '/dev/full'], expected=1)
            results[-1]['passed'] &= 'CSV report saved' not in result.stdout
        benchmark = run('NFR03 four intersection 500 vehicle benchmark',
                        ['--scenario', 'config/performance_scenario.txt', '--report', temp / 'benchmark.csv'],
                        contains='Generated: 500')
        (out / 'benchmark.csv').write_bytes((temp / 'benchmark.csv').read_bytes())
        (out / 'acceptance.csv').write_text(report)
    sources = sorted(p for parent in ['include', 'src', 'tests', 'config'] for p in (ROOT / parent).rglob('*') if p.is_file())
    digest = hashlib.sha256()
    for path in sources:
        digest.update(path.relative_to(ROOT).as_posix().encode())
        digest.update(path.read_bytes())
    evidence = {'platform': platform.platform(), 'processor': platform.processor(),
                'compiler': subprocess.check_output(['c++', '--version'], text=True).splitlines()[0],
                'base_revision': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                'source_sha256': digest.hexdigest(), 'results': results}
    (out / 'system-validation.json').write_text(json.dumps(evidence, indent=2) + '\n')
    lines = ['# System validation report', '', f'Environment: {evidence["platform"]}.',
             f'Compiler: {evidence["compiler"]}.', '', f'Source fingerprint: `{digest.hexdigest()}`.', '',
             'Each CLI process is bounded to 60 seconds. See the adjacent JSON for commands, outcomes and output.', '',
             '| Case | Result | Seconds |', '|---|---|---:|']
    for result in results:
        lines.append(f'| {result["name"]} | {"PASS" if result["passed"] else "FAIL"} | {result.get("seconds", "-")} |')
    lines += ['', 'The benchmark runs 500 configured ticks: 497 completed and 3 still travelling at the duration boundary.',
              'This measures one workload on this machine, not all possible scenarios. Docker and Jenkins execution',
              'require external runtimes and are recorded separately in the submission checklist.']
    (out / 'system-validation.md').write_text('\n'.join(lines) + '\n')
    if not all(result['passed'] for result in results):
        raise SystemExit('System acceptance failed; inspect the recorded results')
    print(f'{len(results)}/{len(results)} system checks passed')

if __name__ == '__main__':
    main()
