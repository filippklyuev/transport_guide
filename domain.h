#pragma once
#include <set>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"

namespace transport_guide {

using DistanceMap = std::unordered_map<std::string_view, int>;

namespace info {

    struct RoutingSettings {
        int bus_wait_time;
        double bus_velocity;
    };

    template<typename InfoPtr>
    struct InfoPtrComparator {
        bool operator()(const InfoPtr* lhs,const InfoPtr* rhs) const {
            return lhs->name < rhs->name;
        }
    };

    struct Bus;

    using DistanceMap = std::unordered_map<std::string_view, int>;

    struct Stop {

        Stop(std::string_view name_, geo::Coordinates coordinates_, DistanceMap distance_to_stops_)
            : name(name_)
            , coordinates(coordinates_)
            , distance_to_stops(distance_to_stops_)
            {}

        std::string_view name = {};
        geo::Coordinates coordinates = {};
        DistanceMap distance_to_stops = {};
        std::set<Bus*, InfoPtrComparator<Bus>> passing_buses = {};

        std::string_view getName() const;
    };

    struct Bus {

        Bus(std::string_view name_, bool is_cycled_, info::RoutingSettings routing_settings_)
        : name(name_)
        , is_cycled(is_cycled_)
        , routing_settings(routing_settings_)
        {}

        std::string_view name;
        std::unordered_set<std::string_view> unique_stops = {};
        std::vector<Stop*> stops = {};
        info::RoutingSettings routing_settings;
        bool is_cycled;
        double geo_route_length = 0.0;
        double curvature = 0.0;
        int64_t factial_route_length = 0;

        void updateBackRoute();

        void updateDistance();

        void updatePassingBus();

        void updateCurvature();

        int getBusWaitTime() const;

        double getBusVelocity() const;

        size_t getUniqueStopsCount() const;

        size_t getStopsCount() const;

        std::string_view getName() const;
    };

} //namespace info

enum class QueryType {
    STOP,
    BUS
};

namespace input {

struct ParsedStopQuery  {
    std::string_view name;
    geo::Coordinates coordinates = {};
    DistanceMap distance_to_stops = {};
};

struct ParsedBusQuery {
    std::string_view name;
    bool is_cycled;
    std::vector<std::string_view> stops_on_route;
};

} // namespace input 

} // namespace transport_guide
