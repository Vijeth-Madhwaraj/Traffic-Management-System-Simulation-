#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
docker build -t traffic-sim:local .
docker run --rm traffic-sim:local --scenario config/acceptance_scenario.txt
