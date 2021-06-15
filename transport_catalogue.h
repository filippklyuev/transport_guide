#pragma once

#include <deque>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "geo.h"

namespace transport_guide {

namespace info {

	struct Bus {
		std::unordered_set<std::string_view> unique_stops;
		std::deque<std::string_view> stops;
		bool is_cycled = false;
		double geo_route_length = 0.0;
		int64_t factial_route_length = 0;

	};

	struct Stop {
		transport_guide::detail::Coordinates coordinates;
		std::unordered_map<std::string_view, int> distance_to_stops = {};
		std::set<std::string_view> passing_buses;
	};

} //namespace info	

class TransportCatalogue {
public:
	TransportCatalogue() = default;

	TransportCatalogue(int qry_nbr){
		input_queries_.resize(qry_nbr);
	}

	using BusMap = std::unordered_map<std::string_view, info::Bus>;
	using StopMap = std::unordered_map<std::string_view, info::Stop>;

	void ProcessInputQueries();

	void AddStop(const std::pair<std::string_view, info::Stop> stop);

	void AddRoute(const std::pair<std::string_view, info::Bus> bus_route);

	bool IsBusListed(std::string_view bus_nbr) const ;

	bool IsStopListed(std::string_view stop_name) const ;


	const info::Bus& GetRouteInfo(std::string_view bus_nbr) const ;

	const info::Stop& GetStopInfo(const std::string_view stop) const ;

	std::vector<std::string>& GetInputQueries();

	const std::vector<std::string>& GetInputQueries() const;

	std::vector<std::string>& GetOutputQueries();

	const std::vector<std::string>& GetOutputQueries() const;

	
private:

	std::vector<std::string> input_queries_;
	std::vector<std::string> output_queries_;
	StopMap stops_map_;
	BusMap buses_map_;

	StopMap& GetStopsMap();

	const StopMap& GetStopsMap() const;

	BusMap& GetBusesMap();

	const BusMap& GetBusesMap() const;

	void GetStopDistances(const std::string& stop_qry);

	std::string_view GetStopName(const std::string& stop_qry, int64_t& pos_first, int64_t& pos_last);	

	std::pair<std::string_view, info::Stop> ParseStopQuery(const std::string& stop_qry);

	std::pair<std::string_view, info::Bus> ParseBusQuery(const std::string& bus_qry);

	char DefineBreaker(const std::string& bus_qry);

	void CalculateRoute(std::string_view stop, info::Bus& bus_info);

	void AddBusToStop(const std::string_view bus_name, const std::string_view stop_name);

	int CalculateBackRoute(const info::Bus& bus_info);
};
	
template <typename S>
bool IsStopQuery(const S& query){
	return query[0] == 'S';
}
	
} // namespace transport_guide
