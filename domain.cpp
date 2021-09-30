#include "domain.h"

namespace transport_guide{

namespace info {

std::string_view Bus::getName() const {
	return name;
}

std::string_view Stop::getName() const {
	return name;
}

const std::vector<const Stop*>& Bus::getStopsOnRoute() const{
    return stops;
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

void Bus::calculateDistance(){
    for (int i = 0; i < stops.size() - 1; i++){
        const info::Stop* from_stop = stops[i];
        const info::Stop* to_stop = stops[i + 1];
        geo_route_length += geo::ComputeDistance(from_stop->coordinates, to_stop->coordinates);
        if (from_stop->distance_to_stops.count(to_stop->name)){
            factial_route_length += from_stop->distance_to_stops.at(to_stop->name);
        } else {
            factial_route_length += to_stop->distance_to_stops.at(from_stop->name);
        }
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

} // namespace info

} // namespace transport_guide