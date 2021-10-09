#include "transport_catalogue.h"

namespace transport_guide {

std::string_view TransportCatalogue::InsertStopName(std::string_view stop_name){
	if (IsStopListed(stop_name)){
		return *stops_.find(stop_name);
	}
	return *stops_.insert(std::string(stop_name)).first;
}

std::string_view TransportCatalogue::InsertBusName(std::string_view bus_name){
	if (IsBusListed(bus_name)){
		return *buses_.find(bus_name);
	}
	return *buses_.insert(std::string(bus_name)).first;
}

void TransportCatalogue::AddStop(std::string_view temp_stop_name, geo::Coordinates coords, DistanceMap&& distance_map){
	std::string_view stop_name = InsertStopName(temp_stop_name);
	GetStopsMap().emplace(stop_name, info::Stop(stop_name, coords, InsertSvsAndGetNewMap(std::move(distance_map))));

}

void TransportCatalogue::updatePassingBusInStops(const info::Bus& bus_info){
    for (const info::Stop* stop : bus_info.getStopsOnRoute()){
    	GetStopsMap().at(stop->name).passing_buses.insert(&bus_info);
    }
}

void TransportCatalogue::AddRoute(std::string_view bus_name_temp, bool is_cycled, std::vector<std::string_view>&& stops_on_route){
	std::string_view bus_name = InsertBusName(bus_name_temp);
	GetBusesMap().emplace(bus_name, info::Bus(bus_name, is_cycled, stops_map_ ,std::move(stops_on_route)));
	updatePassingBusInStops(GetBusesMap().at(bus_name));
}

bool TransportCatalogue::IsBusListed(std::string_view bus_name) const {
	return GetBusesMap().find(bus_name) != GetBusesMap().end();
}

bool TransportCatalogue::IsStopListed(std::string_view stop_name) const {
	return GetStopsMap().find(stop_name) != GetStopsMap().end();
}

info::Bus& TransportCatalogue::GetBusInfo(std::string_view bus_name){
	return GetBusesMap().at(bus_name);
}

const info::Bus& TransportCatalogue::GetBusInfo(std::string_view bus_name) const {
	return GetBusesMap().at(bus_name);
}

const info::Stop& TransportCatalogue::GetStopInfo(std::string_view stop) const {
	return GetStopsMap().at(stop);
}

info::Stop& TransportCatalogue::GetStopInfo(std::string_view stop){
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

const std::set<std::string, std::less<>>& TransportCatalogue::GetBusesSet() const {
    return buses_;
}

const std::set<std::string, std::less<>>& TransportCatalogue::GetStopsSet() const {
    return stops_;
}

DistanceMap TransportCatalogue::InsertSvsAndGetNewMap(DistanceMap temp_map){
    DistanceMap result;
    for (auto [stop_name_temp, distance] : temp_map){
        result.emplace(std::make_pair(InsertStopName(stop_name_temp), distance));
    }
    return result;
}

} // namespace transport_guide
