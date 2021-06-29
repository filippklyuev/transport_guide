#include "transport_catalogue.h"

using namespace transport_guide;

void TransportCatalogue::ProcessInputQueries(){
	std::vector<int> buses_pos = {};
	for (int i = 0; i < GetInputQueries().size(); i++){
		if (IsStopQuery(GetInputQueries()[i])){
			AddStop(ParseStopQuery(GetInputQueries()[i]));
		} else {
			buses_pos.push_back(i);
		}
	}
	for (int i = 0; i < GetInputQueries().size(); i++){
		if (IsStopQuery(GetInputQueries()[i])){
			GetStopDistances(GetInputQueries()[i]);
		}
	}
	for (int bus_pos : buses_pos){
		AddRoute(ParseBusQuery(GetInputQueries()[bus_pos]));
	}
}

void TransportCatalogue::AddStop(const std::pair<std::string_view, info::Stop> stop){
	GetStopsMap().emplace(stop);
}

void TransportCatalogue::AddRoute(const std::pair<std::string_view, info::Bus> bus_route){
	GetBusesMap().emplace(bus_route);
}

bool TransportCatalogue::IsBusListed(std::string_view bus_nbr) const {
	return (GetBusesMap().count(bus_nbr) == 1);
}

bool TransportCatalogue::IsStopListed(std::string_view stop_name) const {
	return (GetStopsMap().count(stop_name) == 1);
}

const info::Bus& TransportCatalogue::GetRouteInfo(std::string_view bus_nbr) const {
	return GetBusesMap().at(bus_nbr);
}

const info::Stop& TransportCatalogue::GetStopInfo(const std::string_view stop) const {
	return GetStopsMap().at(stop);
}

std::vector<std::string>& TransportCatalogue::GetInputQueries(){
	return input_queries_;
}

const std::vector<std::string>& TransportCatalogue::GetInputQueries() const {
	return input_queries_;
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

std::string_view TransportCatalogue::GetStopName(const std::string& stop_qry, int64_t& pos_first, int64_t& pos_last){
	pos_first = stop_qry.find(' ') + 1;
	pos_last = stop_qry.find(':');
	return std::string_view(stop_qry.data() + pos_first, pos_last - pos_first);
}

std::pair<std::string_view, info::Stop> TransportCatalogue::ParseStopQuery(const std::string& stop_qry){
	std::pair<std::string_view, info::Stop> result;
	int64_t pos_first, pos_last;
	result.first = GetStopName(stop_qry, pos_first, pos_last);
	pos_first = pos_last + 2;
	pos_last = stop_qry.find(',');
	result.second.coordinates.lat = std::stod(stop_qry.substr(pos_first, pos_last - pos_first));
	pos_first = stop_qry.find(' ', pos_last);
	pos_last = stop_qry.find(',', pos_first);
	if (pos_last == stop_qry.npos){
		result.second.coordinates.lng = std::stod(stop_qry.substr(pos_first));
		return result;
	}
	result.second.coordinates.lng = std::stod(stop_qry.substr(pos_first, pos_last - pos_first));
	return result;
}

void TransportCatalogue::GetStopDistances(const std::string& stop_qry){
	int64_t pos_first = 0, pos_last = 0;
	std::string_view stop_name = GetStopName(stop_qry, pos_first, pos_last);
	std::unordered_map<std::string_view, int> result;
	pos_first = pos_last;
	for (int i = 0; i < 2; i++){
		pos_first = stop_qry.find(',', pos_first + 1);
	}
	if (pos_first == stop_qry.npos){
		return ;
	}
	pos_first += 2;
	std::string_view to_stop;
	int meters;
	while (true){
		pos_last = stop_qry.find('m', pos_first);
		meters = std::stoi(stop_qry.substr(pos_first, pos_last - pos_first));
		pos_first = pos_last + 5;
		pos_last = stop_qry.find(',', pos_first);
		if (pos_last == stop_qry.npos){
			to_stop = std::string_view(stop_qry.data() + pos_first);
			break ;
 		} else {
 			to_stop = std::string_view(stop_qry.data() + pos_first, pos_last - pos_first);
 			pos_first = pos_last + 2;
 			result.emplace(std::make_pair(to_stop, meters));
 		}
	}
	result.emplace(std::make_pair(to_stop, meters));
	GetStopsMap().at(stop_name).distance_to_stops = result;	
}

char TransportCatalogue::DefineBreaker(const std::string& bus_qry){
	char breaker;
	if (bus_qry.rfind('>') != bus_qry.npos){
		breaker = '>';
	} else {
		breaker = '-';
	}
	return breaker;
}

void TransportCatalogue::CalculateRoute(std::string_view stop, info::Bus& bus_info){
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

std::pair<std::string_view, info::Bus> TransportCatalogue::ParseBusQuery(const std::string& bus_qry){
	const char breaker = DefineBreaker(bus_qry);
    std::pair<std::string_view, info::Bus> result;
    int64_t pos = bus_qry.find(':', 4);
    result.first = std::string_view(bus_qry.data() + 4, pos - 4);
    pos = pos + 2;
    while (true) {
    	int64_t pos_last = bus_qry.find(breaker, pos);
    	if (pos_last == bus_qry.npos){
    		std::string_view last_stop = std::string_view(bus_qry.data() + pos);
    		CalculateRoute(last_stop, result.second);
    		AddBusToStop(result.first, last_stop);
    		break ;
    	}
    	std::string_view stop = std::string_view(bus_qry.data() + pos, pos_last - (pos + 1));
    	CalculateRoute(stop, result.second);
    	AddBusToStop(result.first, stop);
    	pos = pos_last + 2;
    }
    (breaker == '>') ? (result.second.is_cycled = true) : (result.second.is_cycled = false);
    if (result.second.is_cycled == false){
    	result.second.factial_route_length += CalculateBackRoute(result.second);
    	result.second.geo_route_length *= 2;
    }
    return result;
}

void TransportCatalogue::AddBusToStop(const std::string_view bus_name, const std::string_view stop_name){
	GetStopsMap().at(stop_name).passing_buses.insert(bus_name);
}
