#include "stat_reader.h"

using namespace transport_guide::output;

std::vector<Query> transport_guide::output::GetQueries(){
	int number_of_queries = transport_guide::input::read::LineWithNumber();
	std::vector<Query> output_queries(number_of_queries);
	for (int i = 0; i < number_of_queries; i++){
		output_queries[i].query = transport_guide::input::read::Line();
		output_queries[i].type = transport_guide::DefineQueryType(output_queries[i].query);
		output_queries[i].short_query = transport_guide::output::detail::GetShortQuery(output_queries[i].query);
	}
	return output_queries;
}

std::string_view detail::GetShortQuery(const std::string& query){
	return std::string_view(query.data() + query.find(' ') + 1); // finding first letter or digit of bus/stop query after "Bus " or "Stop "
}

void transport_guide::output::PrintQueriesResult(const transport_guide::TransportCatalogue& catalogue, const std::vector<Query>& output_queries){
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