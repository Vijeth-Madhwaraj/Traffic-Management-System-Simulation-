#pragma once

#include "traffic/network/road_network.hpp"

#include <vector>

namespace traffic {

struct Incident {
    RoadId roadId;
    Tick startsAt{};
    Tick endsAt{};
};

class IncidentManager {
public:
    void schedule(Incident incident);
    void advance(Tick now, RoadNetwork& network);

private:
    std::vector<Incident> incidents_;
};

}  // namespace traffic
