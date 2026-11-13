FROM ubuntu:22.04 AS build
RUN apt-get update && apt-get install -y --no-install-recommends \
    g++ cmake make ca-certificates && rm -rf /var/lib/apt/lists/*
WORKDIR /source
COPY CMakeLists.txt ./
COPY include ./include
COPY src ./src
RUN cmake -S . -B build -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --parallel 2

FROM ubuntu:22.04 AS runtime
RUN apt-get update && apt-get install -y --no-install-recommends libstdc++6 \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /opt/traffic
COPY --from=build /source/build/traffic_sim /usr/local/bin/traffic_sim
COPY config ./config
USER 10001:10001
ENTRYPOINT ["/usr/local/bin/traffic_sim"]
CMD ["--scenario", "config/demo_scenario.txt"]
