# System validation report

Environment: Linux-7.0.0-38-generic-x86_64-with-glibc2.43.
Compiler: c++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0.
CPU: Intel(R) Core(TM) Ultra 9 185H; memory: 31743348 kB.

Source fingerprint: `33a9f36aaee0111f517a08e28aa33c58a4f17aef07aae7e86fbf231e664bf4e2`.

Each CLI process is bounded to 60 seconds. See the adjacent JSON for commands, outcomes and output.

| Case | Result | Seconds |
|---|---|---:|
| CLI help | PASS | 0.003332 |
| Validate scenario without advancing | PASS | 0.002855 |
| Reject invalid capacity | PASS | 0.002756 |
| Reject unknown option | PASS | 0.003449 |
| Integrated emergency incident and congestion demo | PASS | 0.005484 |
| CSV records seed throughput and final metrics | PASS | - |
| Seeded replay 1 | PASS | 0.021758 |
| Seeded replay 2 | PASS | 0.016416 |
| Identical seeded CSV results | PASS | - |
| Missing report directory returns failure | PASS | 0.004907 |
| CSV write or close failure returns failure | PASS | 0.0034 |
| NFR03 four intersection 500 vehicle benchmark | PASS | 0.425032 |

The benchmark runs 500 configured ticks: 497 completed and 3 still travelling at the duration boundary.
This measures one workload on this machine, not all possible scenarios. Docker and Jenkins execution
require external runtimes and are recorded separately in the submission checklist.
