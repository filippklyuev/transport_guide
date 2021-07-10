#include "transport_catalogue.h"

using namespace transport_guide;

std::string_view TransportCatalogue::GetSVFromInsertedStopName(const std::string stop_name){
	if (IsStopListed(stop_name)){
		return *stops_.find(stop_name);
	}
	return *stops_.insert(stop_name).first;
}

std::string_view TransportCatalogue::GetSVFromInsertedBusName(const std::string bus_name){
	if (IsBusListed(bus_name)){
		return *buses_.find(bus_name);
	}
	return *buses_.insert(bus_name).first;
}


void TransportCatalogue::AddStop(const std::pair<std::string_view, info::Stop> stop){
	// if (IsStopListed(stop.first)){
	// 	GetStopsMap().at(stop.first) = stop.second;
	// }
	GetStopsMap().emplace(stop);
}

void TransportCatalogue::AddRoute(const std::pair<std::string_view, info::Bus> bus_route){
	// if (IsBusListed(bus_route.first)){
	// 	GetBusesMap().at(bus_route.first) = bus_route.second;
	// }
	GetBusesMap().emplace(bus_route);
}

bool TransportCatalogue::IsBusListed(std::string_view bus_name) const {
	return (buses_.find(std::string(bus_name)) != buses_.end());
}

bool TransportCatalogue::IsStopListed(std::string_view stop_name) const {
	return (stops_.find(std::string(stop_name)) != stops_.end());
}

const info::Bus& TransportCatalogue::GetRouteInfo(const std::string_view bus_name) const {
	return GetBusesMap().at(bus_name);
}

const info::Stop& TransportCatalogue::GetStopInfo(const std::string_view stop) const {
	return GetStopsMap().at(stop);
}

std::vector<std::string>& TransportCatalogue::GetOutputQueries(){
	return output_queries_;
}

const std::vector<std::string>& TransportCatalogue::GetOutputQueries() const{
	return output_queries_;
}

std::unordered_map<std::string_view, info::Stop>& TransportCatalogue::GetStopsMap(){
	return stops_map_;
}

const std::unordered_map<std::string_view, info::Stop>& TransportCatalogue::GetStopsMap() const {
	return stops_map_;
}

std::unordered_map<std::string_view, info::Bus>& TransportCatalogue::GetBusesMap(){
	return buses_map_;
}

const std::unordered_map<std::string_view, info::Bus>& TransportCatalogue::GetBusesMap() const {
	return buses_map_;
}

void TransportCatalogue::AddDistanceToStop(std::string_view stop, info::Bus& bus_info){
	bus_info.stops.push_back(stop);
	bus_info.unique_stops.insert(stop);
	if (bus_info.stops.size() == 1){
		return ;
	}
	int size = bus_info.stops.size();
	bus_info.geo_route_length += geo::ComputeDistance(GetStopsMap().at(bus_info.stops[size - 2]).coordinates, 
													GetStopsMap().at(bus_info.stops.back()).coordinates);
	if (GetStopsMap().at(bus_info.stops[size - 2]).distance_to_stops.count(stop)){
		bus_info.factial_route_length += (GetStopsMap().at(bus_info.stops[size - 2]).distance_to_stops.at(stop));
	} else {
		bus_info.factial_route_length += (GetStopsMap().at(stop).distance_to_stops.at(bus_info.stops[size - 2]));
	}
}

int TransportCatalogue::CalculateBackRoute(const info::Bus& bus_info){
	int back_route = 0;
	for (int i = bus_info.stops.size() - 1; i >= 0; i--){
		if (i == bus_info.stops.size() - 1){
			i -= 1 ;
		}
		if (GetStopsMap().at(bus_info.stops[i + 1]).distance_to_stops.count(bus_info.stops[i])){
			back_route += (GetStopsMap().at(bus_info.stops[i + 1]).distance_to_stops.at(bus_info.stops[i]));
		} else {
			back_route += (GetStopsMap().at(bus_info.stops[i]).distance_to_stops.at(bus_info.stops[i + 1]));
		}
	}
	return back_route;
}

void TransportCatalogue::AddBusToStop(const std::string_view bus_name, const std::string_view stop_name){
	GetStopsMap().at(stop_name).passing_buses.insert(bus_name);
}
