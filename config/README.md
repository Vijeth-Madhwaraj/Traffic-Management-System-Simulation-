# Scenario configuration, version 1

Run a file with `traffic_sim --scenario config/demo_scenario.txt`.
Preview a scenario before running it with `traffic_sim --validate <file>`. This validates
the same format and references, prints configuration counts, and exits without generating
vehicles or consuming seeded arrivals. Invalid files return a nonzero exit code.

Records are whitespace-separated; `#` begins a comment. IDs have no whitespace or `#`.
The first non-comment record must be `scenario 1`; `seed` and `duration` are required once.
Other records may reference definitions later in the file. Repeated `phase` records define
an intersection's ordered phase cycle. Movement IDs must match the movement module's IDs;
the intersection owner must supply the complete conflict table.

```text
scenario 1
seed <unsigned-64-bit-seed>
duration <ticks>
intersection <id>
road <id> <from> <to> <capacity> <travel-ticks>
lane <id> <entry-road-id> <capacity>
phase <intersection> <green-ticks> <yellow-ticks> <all-red-ticks> <movement>...
conflict <intersection> <movement> <conflicting-movement>
arrival <tick> <lane> <destination> <standard|emergency> <route-road>...
stream <id> <lane> <destination> <standard|emergency> <first-tick> <end-tick> <probability> <route-road>...
```

Use a single `-` instead of route roads to ask the route provider. Source is derived from
the lane's entry road. An explicit route must start on that road, follow connected enabled
roads, and end at the destination. Stream intervals are `[first-tick, end-tick)`, with at
most one arrival per stream per tick. Probability is in `[0,1]`. Scheduled arrivals at the
same tick preserve file order; streams run afterwards in stream-ID order. Emergency class
does not change FIFO order or bypass signal safety.

Lane capacity limits waiting vehicles, separately from road occupancy in transit. Each lane
capacity must not exceed its entry road's capacity. Full queues defer arrivals with their
original IDs; pending vehicles enter before new arrivals. `--scenario <file> --lane-status`
shows per-lane queue capacity, queued IDs, and deferred IDs without changing queue order.
Queue insertion rejects ID zero and an ID already present in that same queue; rejected
insertions do not consume capacity. No route is a controlled
`Unroutable` result, not a full-lane condition.

Limits: 1 MiB per file, 4096 bytes per line, 2–1024 intersections, 1–4096 roads and lanes,
1–100000 ticks, positive capacities/travel times/phase durations up to 1000000, and at most
128 streams. Scheduled arrivals plus the sum of stream interval lengths must not exceed
100000 potential vehicles (even when probability is below 1). All arrivals must be inside
the run duration. Invalid files are rejected as a whole. Seed zero is valid.

`demo_scenario.txt` demonstrates admission and movement over explicit routes; its entry
queue drains by the end of the run. `seeded_scenario.txt` demonstrates repeatable random
arrivals. The CLI lane-status test also uses `tests/fixtures/blocked_entry_scenario.txt`,
whose slow entry road leaves vehicle 2 queued and vehicle 3 deferred. Automatic routing
and completed-trip performance/reporting remain incomplete.

`performance_scenario.txt` provides 4 intersections and 500 guaranteed arrivals for later
NFR-03 validation. Explicit-route movement is now enabled, but completed-trip performance
and reporting still need separate validation; earlier input-only timings do not establish
full-system NFR-03 compliance.
