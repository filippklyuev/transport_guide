#include "transport_catalogue.h"

using namespace transport_guide;

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
