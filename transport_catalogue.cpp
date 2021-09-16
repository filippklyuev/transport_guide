#include "transport_catalogue.h"

namespace transport_guide {

std::string_view TransportCatalogue::InsertStopName(std::string stop_name){
	if (IsStopListed(stop_name)){
		return *stops_.find(stop_name);
	}
	return *stops_.insert(std::move(stop_name)).first;
}

std::string_view TransportCatalogue::InsertBusName(std::string bus_name){
	if (IsBusListed(bus_name)){
		return *buses_.find(bus_name);
	}
	return *buses_.insert(std::move(bus_name)).first;
}

void TransportCatalogue::AddStop(std::string_view temp_stop_name, geo::Coordinates coords, info::DistanceMap&& distance_map){
	std::string_view stop_name = InsertStopName(std::string(temp_stop_name));
	GetStopsMap().emplace(stop_name, info::Stop(stop_name, coords, InsertSvsAndGetNewMap(std::move(distance_map))));

}

void TransportCatalogue::AddRoute(std::string_view bus_name_temp, bool is_cycled, std::vector<std::string_view>&& stops_on_route){
	std::string_view bus_name = InsertBusName(std::string(bus_name_temp));
	GetBusesMap().emplace(bus_name, info::Bus(bus_name, is_cycled));
	processStopsOnRoute(GetBusInfo(bus_name), std::move(stops_on_route));
}

bool TransportCatalogue::IsBusListed(const std::string_view bus_name) const {
	return GetBusesMap().find(bus_name) != GetBusesMap().end();
}

bool TransportCatalogue::IsStopListed(const std::string_view stop_name) const {
	return GetStopsMap().find(stop_name) != GetStopsMap().end();
}

info::Bus& TransportCatalogue::GetBusInfo(std::string_view bus_name){ // пока что оставил исключения метода at, чтобы не перегружать код проверками на != nullptr при разыменовывании 
	return GetBusesMap().at(bus_name);
}

const info::Bus& TransportCatalogue::GetBusInfo(std::string_view bus_name) const {
	return GetBusesMap().at(bus_name);
}

const info::Stop& TransportCatalogue::GetStopInfo(const std::string_view stop) const {
	return GetStopsMap().at(stop);
}

info::Stop& TransportCatalogue::GetStopInfo(const std::string_view stop){
	return GetStopsMap().at(stop);
}

TransportCatalogue::StopMap& TransportCatalogue::GetStopsMap(){
	return stops_map_;
}

const TransportCatalogue::StopMap& TransportCatalogue::GetStopsMap() const {
	return stops_map_;
}

TransportCatalogue::BusMap& TransportCatalogue::GetBusesMap(){
	return buses_map_;
}

const TransportCatalogue::BusMap& TransportCatalogue::GetBusesMap() const {
	return buses_map_;
}

void TransportCatalogue::processStopsOnRoute(info::Bus& bus_info, std::vector<std::string_view> stops_on_route){
	for (auto stop : stops_on_route){
		bus_info.stops.push_back(&GetStopInfo(stop));
		bus_info.unique_stops.insert(bus_info.stops.back()->getName());
		bus_info.updateDistance();
	}
	if (!bus_info.is_cycled){
		bus_info.updateBackRoute();
	}
	bus_info.updatePassingBus();
}

info::DistanceMap TransportCatalogue::InsertSvsAndGetNewMap(info::DistanceMap temp_map){
    info::DistanceMap result;
    for (auto [stop_name_temp, distance] : temp_map){
        result.emplace(std::make_pair(InsertStopName(std::string(stop_name_temp)), distance));
    }
    return result;
}


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

size_t Bus::getUniqueStopsCount() const {
		return unique_stops.size();
}

} // namespace info

} // namespace transport_guide
