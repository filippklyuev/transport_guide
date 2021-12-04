#pragma once
#include <map>
#include <set>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"

namespace transport_guide {

using DistanceMap = std::unordered_map<std::string_view, int>;

namespace info {

    template<typename InfoPtr>
    struct InfoPtrComparator {
        bool operator()(const InfoPtr* lhs,const InfoPtr* rhs) const {
            return lhs->name < rhs->name;
        }
    };

    struct Bus;

    struct Stop {

        Stop()
        {}

        Stop(std::string_view name_, geo::Coordinates coordinates_, DistanceMap distance_to_stops_, int id)
            : name(name_)
            , coordinates(coordinates_)
            , distance_to_stops(distance_to_stops_)
            , id_(id)
            {}

        std::string_view name = {};
        geo::Coordinates coordinates = {};
        DistanceMap distance_to_stops = {};
        std::set<const Bus*, InfoPtrComparator<Bus>> passing_buses = {};
        int id_ = 0;

        std::string_view getName() const;
    };

    using StopMap = std::map<std::string_view, info::Stop>;

    struct Bus {
        Bus()
        {}

        Bus(std::string_view name_, bool is_cycled_, const StopMap& stops_map, std::vector<std::string_view> stops_on_route, int id)
            : name(name_)
            , is_cycled(is_cycled_)
            , id_(id)
        {
            for (auto stop : stops_on_route){
                stops.push_back(&(stops_map.at(stop)));
                unique_stops.insert(stops.back()->getName());
                
            }
            calculateDistance();
            if (!is_cycled){
                updateBackRoute();
            }
            if (stops.size() > 1){
                updateCurvature();
            }
        }

        std::string_view name;
        std::unordered_set<std::string_view> unique_stops = {};
        std::vector<const Stop*> stops = {};
        bool is_cycled = false;
        double geo_route_length = 0.0;
        double curvature = 0.0;
        int64_t factial_route_length = 0;
        int id_ = 0;

        const std::vector<const Stop*>& getStopsOnRoute() const;

        size_t getUniqueStopsCount() const;

        size_t getStopsCount() const;

        std::string_view getName() const;

    private:

        void updateBackRoute();

        void calculateDistance();

        void updateCurvature();

    };

} //namespace info

} // namespace transport_guide
