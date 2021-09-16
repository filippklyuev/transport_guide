#include "stat_reader.h"

namespace transport_guide {

namespace output {

std::string_view detail::GetName(const std::string& query){
	return std::string_view(query.data() + query.find(' ') + 1); // finding first letter or digit of bus/stop query after "Bus " or "Stop "
}

void PrintQueriesResult(const TransportCatalogue& catalogue, const std::vector<Query>& output_queries, std::ostream& out){
	for (const auto& query : output_queries){
		std::string_view name = detail::GetName(query.query);
		if (query.type == QueryType::STOP){
			out << "Stop " << name << ": ";
			if (catalogue.IsStopListed(name)){
				out << catalogue.GetStopInfo(name)  << std::endl;
			} else {
				out << "not found" << std::endl;
			}
		} else {
			out << "Bus " << name << ": ";
			if (catalogue.IsBusListed(name)){
				out << catalogue.GetBusInfo(name) << std::endl;
			} else {
				out << "not found" << std::endl;
			}
		}
	}
}

std::ostream& operator<<(std::ostream& out, const info::Bus& info){
	(info.is_cycled) ? (out << info.stops.size()) : (out << info.stops.size() * 2 - 1);
	out << " stops on route, ";
	out << info.getUniqueStopsCount();
	out << " unique stops, ";
	out << info.factial_route_length << " route length, ";
	out << (info.factial_route_length / info.geo_route_length) << " curvature";
	return out;
}

std::ostream& operator<<(std::ostream& out, const info::Stop& info){
	if (info.passing_buses.size() == 0){
		out << "no buses";
	} else {
		out << "buses ";
		for (auto bus : info.passing_buses){
			out << bus->getName() << " ";
		}
	}
	return out;
}

} //namespace output 

} // namespace transport_guide