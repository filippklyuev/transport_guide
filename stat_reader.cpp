#include "stat_reader.h"

using namespace transport_guide::output;

std::string_view detail::TransformBusName(const std::string& bus_name){
	return std::string_view(bus_name.data() + bus_name.find(' ') + 1);
}

std::vector<OutputQuery> read::Queries(TransportCatalogue& catalogue, const int nbr_of_queries){
	std::vector<OutputQuery> get_queries;
	catalogue.GetOutputQueries().resize(nbr_of_queries);
	get_queries.resize(nbr_of_queries);
	for (int i = 0; i < nbr_of_queries; i++){
		std::getline(std::cin, catalogue.GetOutputQueries()[i]);
		if (IsStopQuery(catalogue.GetOutputQueries()[i])){
			get_queries[i].is_stop_query = true;
		}
		get_queries[i].query = (detail::TransformBusName(catalogue.GetOutputQueries()[i]));
	}
	return get_queries;
}

void print::Output(const TransportCatalogue& catalogue, const std::vector<OutputQuery>& queries_to_print){
	for (const auto& query : queries_to_print) {
		if (query.is_stop_query){
			std::cout << "Stop " << query.query << ": ";
			if (catalogue.IsStopListed(query.query)){
				print::QueryInfo(catalogue.GetStopInfo(query.query));
			} else {
				std::cout << "not found" << std::endl;
			}
		} else {
			std::cout << "Bus " << query.query << ": ";
			if (catalogue.IsBusListed(query.query)){
				print::QueryInfo(catalogue.GetRouteInfo(query.query));
			} else {
				std::cout << "not found" << std::endl;
			}
		}
	}
}

std::ostream& print::operator<<(std::ostream& out, const info::Bus& info){
	(info.is_cycled) ? (out << info.stops.size()) : (out << info.stops.size() * 2 - 1);
	out << " stops on route, ";
	(info.is_cycled) ? (out << info.unique_stops.size()) : (out << info.unique_stops.size());
	out << " unique stops, ";
	out << info.factial_route_length << " route length, ";
	out << (info.factial_route_length / info.geo_route_length) << " curvature";
	return out;
}

std::ostream& print::operator<<(std::ostream& out, const info::Stop& info){
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