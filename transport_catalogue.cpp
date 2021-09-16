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

void TransportCatalogue::AddRoute(std::string_view bus_name_temp, bool is_cycled, std::vector<std::string_view>&& stops_on_route, info::RoutingSettings routing_settings){
	std::string_view bus_name = InsertBusName(std::string(bus_name_temp));
	GetBusesMap().emplace(bus_name, info::Bus(bus_name, is_cycled, routing_settings));
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

const std::set<std::string>& TransportCatalogue::GetBusesSet() const {
    return buses_;
}

const std::set<std::string>& TransportCatalogue::GetStopsSet() const {
    return stops_;
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
	bus_info.updateCurvature();
	bus_info.updatePassingBus();
}

DistanceMap TransportCatalogue::InsertSvsAndGetNewMap(DistanceMap temp_map){
    DistanceMap result;
    for (auto [stop_name_temp, distance] : temp_map){
        result.emplace(std::make_pair(InsertStopName(std::string(stop_name_temp)), distance));
    }
    return result;
}

} // namespace transport_guide
