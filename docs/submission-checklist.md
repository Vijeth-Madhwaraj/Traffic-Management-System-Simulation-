# Final submission checklist

Prepared 11 November 2026 against the lecturer checklist and FAQs supplied by Aarya.

| Deliverable | State | Evidence or action |
|---|---|---|
| SRS introduction, FR/NFR, use-case diagram | Updated | Traffic_Management_SRS.docx and Traffic_Management_SRS-1.pdf |
| Security objectives and requirements | Updated | SRS Section 6 defines SO-1..3 and SEC-01..04 |
| Test Plan Sections 3/4/5 and security 5.1 | Updated | Original 28 cases retained with current outcomes |
| SAD component and two sequence diagrams, API/errors | Updated | Existing diagrams retained; concrete contracts corrected |
| Source/test RTM | Verified | All 18 FR test names resolve to executed tests |
| Unit report | Generated | reports/unit-tests.md; 115/115 cases |
| System report | Generated | reports/system-validation.md; 12/12 checks |
| Code and branch coverage reports | Generated | reports/coverage; actual gcov counts and machine-readable data |
| Compiler warnings, formatting, analysis | Passed locally | clang-format 18.1.8; warnings as errors; 12 analysis units |
| Four-intersection / 500-vehicle benchmark | Passed recorded workload | Environment, elapsed time and CSV in reports |
| Seeded whole-run replay | Passed | Identical CSV system checks and reset regressions |
| Docker implementation | Prepared; runtime blocked locally | Docker daemon unavailable; container CI job added |
| Jenkins pipeline/archive | Prepared; execution pending | Requires a configured Jenkins server/agent |
| GitHub Project link, points, assignments, two sprints | Unverified | Token lacks read:project; team must inspect board |
| Assigned-member PRs and owner approval | Pending final review | Commit messages record module coordination, not invented teammate authorship |
| One-minute capability demo | Recorded current implementation | videos/sprint-1-capabilities.mp4; recorded now, not historical sprint evidence |
| Two-minute capability demo | Recorded current implementation | videos/sprint-2-capabilities.mp4; recorded now, not historical sprint evidence |
| Sprint 2 official freeze | Pending team action | Record accepted revision/tag only after review/merge |
| Design deviations | Documented; approval pending | deviations.md |
| NFR-06 unfamiliar-user review | Pending | Ask a reviewer to validate a file, explain an error, find completed/unfinished counts and interpret throughput; record actual results and reviewer/date |
| Test Plan/team/evaluator approvals and viva | Pending | Obtain genuine sign-off and prepare each member's contribution explanation |

Do not close outstanding acceptance items solely because automated tests pass. Issues #7,
#32, #33, #34 and #26 have implementation evidence on this branch, but must be reconciled
with their closed status and accepted PR. FR-10/11/12 issues can be reviewed against the RTM.
Never manufacture story points, historic sprint dates, reviewer signatures or individual authorship.
