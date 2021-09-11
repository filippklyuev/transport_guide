#include "stat_reader.h"

using namespace transport_guide;

std::string_view output::detail::GetName(const std::string& query){
	return std::string_view(query.data() + query.find(' ') + 1); // finding first letter or digit of bus/stop query after "Bus " or "Stop "
}

void output::PrintQueriesResult(const TransportCatalogue& catalogue, const std::vector<Query>& output_queries){
	for (const auto& query : output_queries){
		if (query.type == QueryType::STOP){
			std::cout << "Stop " << query.name << ": ";
			if (catalogue.IsStopListed(query.name)){
				std::cout << catalogue.GetStopInfo(query.name)  << std::endl;
			} else {
				std::cout << "not found" << std::endl;
			}
		} else {
			std::cout << "Bus " << query.name << ": ";
			if (catalogue.IsBusListed(query.name)){
				std::cout << catalogue.GetRouteInfo(query.name) << std::endl;
			} else {
				std::cout << "not found" << std::endl;
			}
		}
	}

}

std::ostream& output::operator<<(std::ostream& out, const info::Bus& info){
	(info.is_cycled) ? (out << info.stops.size()) : (out << info.stops.size() * 2 - 1);
	out << " stops on route, ";
	(info.is_cycled) ? (out << info.unique_stops.size()) : (out << info.unique_stops.size());
	out << " unique stops, ";
	out << info.factial_route_length << " route length, ";
	out << (info.factial_route_length / info.geo_route_length) << " curvature";
	return out;
}

std::ostream& output::operator<<(std::ostream& out, const info::Stop& info){
	if (info.passing_buses.size() == 0){
		out << "no buses";
	} else {
		out << "buses ";
		// out << (info.passing_buses.size()) << " ";
		for (auto bus : info.passing_buses){
			out << bus->getName() << " ";
		}
	}
	return out;
}