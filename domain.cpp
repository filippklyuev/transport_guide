#include "domain.h"

namespace transport_guide{

namespace info {

std::string_view Bus::getName() const {
	return name;
}

std::string_view Stop::getName() const {
	return name;
}

void Bus::updatePassingBus(){
    for (auto& stop : stops){
        stop->passing_buses.insert(this);
    }
}

void Bus::updateBackRoute(){
    for (int i = stops.size() - 2; i >= 0; i--){
        if (stops[i + 1]->distance_to_stops.count(stops[i]->getName())){
            factial_route_length += stops[i + 1]->distance_to_stops.at(stops[i]->getName());
        } else {
            factial_route_length += stops[i]->distance_to_stops.at(stops[i + 1]->getName());
        }
    }
    geo_route_length *= 2;
}

void Bus::updateDistance(){
    if (stops.size() == 1){
        return ;
    }
    int size = stops.size();
    std::string_view last_stop_name = stops.back()->getName();
    geo_route_length += geo::ComputeDistance(stops[size - 2]->coordinates, stops.back()->coordinates);

    if (stops[size - 2]->distance_to_stops.count(last_stop_name)){
        factial_route_length += stops[size - 2]->distance_to_stops.at(last_stop_name);
    } else {
        factial_route_length += stops.back()->distance_to_stops.at(stops[size-2]->getName());
    }
}

void Bus::updateCurvature(){
    curvature = factial_route_length / geo_route_length;
}


size_t Bus::getUniqueStopsCount() const {
		return unique_stops.size();
}

size_t Bus::getStopsCount() const {
    if (is_cycled){
        return (stops.size());
    } else {
        return (stops.size() * 2 - 1);
    }
}

int Bus::getBusWaitTime() const {
    return routing_settings.bus_wait_time;
}

double Bus::getBusVelocity() const {
    return routing_settings.bus_velocity;
}

} // namespace info

} // namespace transport_guide