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
lane <id> <road-id> <capacity>
phase <intersection> <green-ticks> <yellow-ticks> <all-red-ticks> <movement>...
conflict <intersection> <movement> <conflicting-movement>
incident <road-id> <start-tick> <end-tick>
adaptive <intersection> <minimum-green> <maximum-green> <queue-threshold> <adjustment>
arrival <tick> <lane> <destination> <standard|emergency> <route-road>...
stream <id> <lane> <destination> <standard|emergency> <first-tick> <end-tick> <probability> <route-road>...
batch <stream-id> <vehicles-per-successful-arrival>
```

Use a single `-` instead of route roads to request automatic routing. The default provider
fixes the configured entry road and selects the enabled suffix with the lowest total travel time; equal-cost routes are
resolved lexicographically by road ID. A caller may inject another provider. Source is derived from
the lane's entry road. An explicit route must start on that road, follow connected enabled
roads, and end at the destination. Stream intervals are `[first-tick, end-tick)`, with at
most one probability decision per stream per tick. Probability is in `[0,1]`.
An optional `batch <stream-id> <count>` record creates `count` vehicles when that decision
succeeds; the default is 1, preserving existing scenarios. Batch size is 1–1000, and a
stream may have only one batch setting. It may appear before or after the stream record;
unknown stream IDs are rejected. All vehicles in a batch have the stream's configured
attributes and separate unique IDs. One RNG draw per active stream is preserved regardless
of batch size. Scheduled arrivals at the same tick preserve file order; streams run
afterwards in stream-ID order, with each batch emitted in ID order. Emergency class
keeps FIFO entry order and requests signal priority when ready to cross. Requests select a
configured phase after full clearance and never bypass signal or capacity checks.

An optional `adaptive` record configures FR-07 for one controlled intersection. At the start
of each Green phase, the controller sums ready-to-cross vehicle queues for movements permitted
by that phase. If the sum is at least `queue-threshold`, that phase's green duration increases
by `adjustment`, up to `maximum-green`. If the sum is zero, the duration decreases by the same
step, down to `minimum-green`. Other queue sizes leave the duration unchanged. Every base phase
green duration must be within the configured bounds. Yellow and all-red clearance durations,
phase order, and permitted movements are unchanged. Without an `adaptive` record, timing stays
fixed as before.

Lane capacity limits waiting vehicles. When crossing into a downstream road with configured
lanes, each lane also limits in-transit occupancy; source-queue entry does not consume that
downstream occupancy. Road capacity remains an aggregate limit. Each lane capacity must not
exceed its road's capacity. Full queues defer arrivals with their
original IDs; pending vehicles enter before new arrivals. `--scenario <file> --lane-status`
shows per-lane queue capacity, queued IDs, and deferred IDs without changing queue order.
Queue insertion rejects ID zero and an ID already present in that same queue; rejected
insertions do not consume capacity. No route is a controlled
`Unroutable` result, not a full-lane condition. Missing provider routes are recorded as
`RouteFailure::NoRoute`; invalid provider results are recorded as
`RouteFailure::InvalidRoute` and never enter a lane. Other valid arrivals continue.
Exceptions thrown by a provider preserve the admission tick for retry. Invalid explicit
routes in the scenario file remain setup errors and reject the scenario.

Limits: 1 MiB per file, 4096 bytes per line, 2–1024 intersections, 1–4096 roads and lanes,
1–100000 ticks, positive capacities/travel times/phase durations up to 1000000, and at most
128 streams. Scheduled arrivals plus the sum of `(stream interval length * batch size)`
must not exceed 100000 potential vehicles (even when probability is below 1). All arrivals must be inside
the run duration. Invalid files are rejected as a whole. Seed zero is valid.

`demo_scenario.txt` demonstrates admission and movement over explicit routes; its entry
queue drains by the end of the run. `seeded_scenario.txt` demonstrates repeatable random
arrivals. The CLI lane-status test also uses `tests/fixtures/blocked_entry_scenario.txt`,
whose slow entry road leaves vehicle 2 queued and vehicle 3 deferred. Automatic minimum-travel-time routing and CSV trip reports are available; incident and congestion
responses reroute vehicles already travelling when an enabled alternative exists. The CLI summary includes
counts of vehicles that reached their destination and vehicles still in transit; `--trace`
shows the reached-destination count after each tick.

`performance_scenario.txt` provides 4 intersections and 500 guaranteed arrivals for later
NFR-03 validation. Explicit-route movement is now enabled, but completed-trip performance
and reporting still need separate validation; earlier input-only timings do not establish
full-system NFR-03 compliance.

`batch_scenario.txt` demonstrates FR-02 batch arrivals: 3 vehicles per tick for 4 ticks
(12 generated). Only 4 have entered the narrow entry queue by the end of this short run;
IDs 5–12 remain deferred in FIFO order. Run it with:

```sh
./build/traffic_sim --scenario config/batch_scenario.txt --lane-status
```

## Management policies and reporting

`incident ROAD START END` blocks a known road over `[START,END)`; intervals must be
inside the run duration. Existing vehicles replan the remaining suffix after their current
road if a remaining segment is blocked or congested. Congestion responses first avoid
flagged roads; if none are available they retain an enabled route and wait for capacity.
If no enabled route exists they keep their old route and wait safely for restoration.
The source road of a queued arrival remains fixed. Movement still requires permitted
signals and available road/lane capacity after rerouting.

A ready emergency vehicle submits its next movement to the signal controller. Priority
phase requests are deduplicated and FIFO. Full current green, yellow and all-red intervals
are retained; normal cycling resumes when the request queue drains. In-process callers
may also use `Simulation::requestPriority(intersection,movement)`; unknown IDs return false.

Waiting includes deferred arrivals and every queued vehicle at tick end, plus in-transit
waiting after road travel has elapsed. Average wait uses completed vehicles only. Peak
queue length is observed before and after movement and excludes the separately reported
deferred backlog. Throughput is completed trips per processed tick in `[0,currentTick)`.
Final congestion is sampled after movement. CSV includes seed and measurement ticks;
open/write/close failures return exit code 1.

`acceptance_scenario.txt` combines congestion, a late road incident, emergency priority,
adaptive timing and final reporting; all four configured vehicles complete.
