#include "transport_catalogue.h"

namespace transport_guide {

std::string_view TransportCatalogue::InsertNameSV(std::string_view name, QueryType type){
	if (type == QueryType::STOP){
		return InsertStopName(std::string(name));
	} else {
		return InsertBusName(std::string(name));
	}
}

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

void TransportCatalogue::AddStop(std::string_view name){
	GetStopsMap()[name];
}

void TransportCatalogue::AddRoute(std::string_view name){
	GetBusesMap()[name];
}

bool TransportCatalogue::IsBusListed(const std::string_view bus_name) const {
	return GetBusesMap().find(bus_name) != GetBusesMap().end();
}

bool TransportCatalogue::IsStopListed(const std::string_view stop_name) const {
	return GetStopsMap().find(stop_name) != GetStopsMap().end();
}

info::Bus& TransportCatalogue::GetRouteInfo(std::string_view bus_name){ // пока что оставил исключения метода at, чтобы не перегружать код проверками на != nullptr при разыменовывании 
	return GetBusesMap().at(bus_name);
}

const info::Bus& TransportCatalogue::GetRouteInfo(std::string_view bus_name) const {
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

namespace info {

std::string_view Bus::getName() const {
	return name;
}

std::string_view Stop::getName() const {
	return name;
}

Stop& Stop::setName(std::string_view stop_name){
	name = stop_name;
	return *this;
}	

Stop& Stop::setCoordinates(geo::Coordinates coords){
	coordinates = coords;
	return *this;
}

Stop& Stop::setDistanceToStops(DistanceMap map){
	distance_to_stops = map;
	return *this;
}

void Bus::updatePassingBus(){
    for (auto& stop : stops){
        stop->passing_buses.insert(this);
    }
}

Bus& Bus::setName(std::string_view bus_name){
	name = bus_name;
	return *this;
}

Bus& Bus::setIsCycled(bool is_cycled_){
	is_cycled = is_cycled_;
	return *this;
}

Bus& Bus::setStopsAndDistance(const TransportCatalogue& catalogue, std::vector<std::string_view> stops_on_route_temp){
	for (auto stop : stops_on_route_temp){
		stops.push_back(&(const_cast<TransportCatalogue&>(catalogue).GetStopInfo(stop)));
		unique_stops.insert(stops.back()->getName());
		updateDistance();
	}
	if (!is_cycled){
		updateBackRoute();
	}
	return *this;
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
