#include "transport_catalogue.h"

using namespace transport_guide;

// std::vector<info::Stop*> TransportCatalogue::updateRouteInfoAndGetStops(std::vector<std::string_view> stops_on_route_temp){
// 	std::vector<info::Stop*> stops_on_route;
// 	for (auto stop : stops_on_route){

// 	}
// }

std::string_view TransportCatalogue::GetSVFromInsertedName(std::string_view name, QueryType type){
	if (type == QueryType::STOP){
		return GetSVFromInsertedStopName(std::string(name));
	} else {
		return GetSVFromInsertedBusName(std::string(name));
	}
}

std::string_view TransportCatalogue::GetSVFromInsertedStopName(std::string stop_name){
	if (IsStopListed(stop_name)){
		return *stops_.find(stop_name);
	}
	return *stops_.insert(std::move(stop_name)).first;
}

std::string_view TransportCatalogue::GetSVFromInsertedBusName(std::string bus_name){
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

info::Bus& TransportCatalogue::GetRouteInfo(std::string_view bus_name){
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

// void TransportCatalogue::AddDistanceToStop(const std::string_view stop, info::Bus& bus_info){
// 	bus_info.stops.push_back(stop);
// 	bus_info.unique_stops.insert(stop);
// 	if (bus_info.stops.size() == 1){
// 		return ;
// 	}
// 	int size = bus_info.stops.size();
// 	bus_info.geo_route_length += geo::ComputeDistance(GetStopsMap().at(bus_info.stops[size - 2]).coordinates, 
// 													GetStopsMap().at(bus_info.stops.back()).coordinates);
// 	if (GetStopsMap().at(bus_info.stops[size - 2]).distance_to_stops.count(stop)){
// 		bus_info.factial_route_length += (GetStopsMap().at(bus_info.stops[size - 2]).distance_to_stops.at(stop));
// 	} else {
// 		bus_info.factial_route_length += (GetStopsMap().at(stop).distance_to_stops.at(bus_info.stops[size - 2]));
// 	}
// }

// int TransportCatalogue::GetBackRouteDistance(const info::Bus& bus_info){
// 	int back_route = 0;
// 	for (int i = bus_info.stops.size() - 2; i >= 0; i--){ 
// 		if (GetStopsMap().at(bus_info.stops[i + 1]).distance_to_stops.count(bus_info.stops[i])){
// 			back_route += (GetStopsMap().at(bus_info.stops[i + 1]).distance_to_stops.at(bus_info.stops[i]));
// 		} else {
// 			back_route += (GetStopsMap().at(bus_info.stops[i]).distance_to_stops.at(bus_info.stops[i + 1]));
// 		}
// 	}
// 	return back_route;
// }

// void TransportCatalogue::AddBusToStop(const std::string_view bus_name, const std::string_view stop_name){
// 	GetStopsMap().at(stop_name).passing_buses.insert(bus_name);
// }
