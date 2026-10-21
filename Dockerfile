# Starter container recipe; pin a base image and add build/run stages when the
# supported compiler/CMake versions are agreed by the team.
FROM ubuntu:22.04
WORKDIR /workspace
COPY . .
# TODO: install build tools, configure/build with CMake, and define the runtime command.
