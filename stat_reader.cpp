#include "stat_reader.h"

std::string_view transport_guide::output::detail::GetShortQuery(const std::string& query){
	return std::string_view(query.data() + query.find(' ') + 1); // finding first letter or digit of bus/stop query after "Bus " or "Stop "
}

void transport_guide::output::PrintQueriesResult(const transport_guide::TransportCatalogue& catalogue, const std::vector<transport_guide::Query>& output_queries){
	for (const auto& query : output_queries){
		if (query.type == transport_guide::QueryType::STOP){
			std::cout << "Stop " << query.short_query << ": ";
			if (catalogue.IsStopListed(query.short_query)){
				std::cout << catalogue.GetStopInfo(query.short_query)  << std::endl;
			} else {
				std::cout << "not found" << std::endl;
			}
		} else {
			std::cout << "Bus " << query.short_query << ": ";
			if (catalogue.IsBusListed(query.short_query)){
				std::cout << catalogue.GetRouteInfo(query.short_query) << std::endl;
			} else {
				std::cout << "not found" << std::endl;
			}
		}
	}

}

std::ostream& transport_guide::output::operator<<(std::ostream& out, const transport_guide::info::Bus& info){
	(info.is_cycled) ? (out << info.stops.size()) : (out << info.stops.size() * 2 - 1);
	out << " stops on route, ";
	(info.is_cycled) ? (out << info.unique_stops.size()) : (out << info.unique_stops.size());
	out << " unique stops, ";
	out << info.factial_route_length << " route length, ";
	out << (info.factial_route_length / info.geo_route_length) << " curvature";
	return out;
}

std::ostream& transport_guide::output::operator<<(std::ostream& out, const transport_guide::info::Stop& info){
	if (info.passing_buses.size() == 0){
		out << "no buses";
	} else {
		out << "buses ";
		for (const auto& bus : info.passing_buses){
			out << bus << " ";
		}
	}
	return out;
}