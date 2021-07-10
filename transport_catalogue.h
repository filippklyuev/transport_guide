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
		geo::Coordinates coordinates;
		std::unordered_map<std::string_view, int> distance_to_stops = {};
		std::set<std::string_view> passing_buses = {};
	};

} //namespace info	

class TransportCatalogue {
public:
	TransportCatalogue() = default;
	
	using BusMap = std::unordered_map<std::string_view, info::Bus>;
	using StopMap = std::unordered_map<std::string_view, info::Stop>;

	void ProcessInputQueries();

	std::string_view  GetSVFromInsertedBusName(const std::string bus_name);

	std::string_view  GetSVFromInsertedStopName(const std::string stop_name);

	void AddStop(const std::pair<std::string_view, info::Stop> stop);

	void AddRoute(const std::pair<std::string_view, info::Bus> bus_route);

	bool IsBusListed(std::string_view bus_name) const ;

	bool IsStopListed(std::string_view stop_name) const ;

	const info::Bus& GetRouteInfo(const std::string_view bus_name) const ;

	const info::Stop& GetStopInfo(const std::string_view stop) const ;

	std::vector<std::string>& GetOutputQueries();

	const std::vector<std::string>& GetOutputQueries() const;

	void AddDistanceToStop(std::string_view stop, info::Bus& bus_info);

	void AddBusToStop(const std::string_view bus_name, const std::string_view stop_name);

	int CalculateBackRoute(const info::Bus& bus_info);

	
private:

	std::vector<std::string> output_queries_;
	std::unordered_set<std::string> buses_;
	std::unordered_set<std::string> stops_;

	StopMap stops_map_;
	BusMap buses_map_;

	StopMap& GetStopsMap();

	const StopMap& GetStopsMap() const;

	BusMap& GetBusesMap();

	const BusMap& GetBusesMap() const;
};
	
template <typename S>
bool IsStopQuery(const S& query){
	return query[0] == 'S';
}
	
} // namespace transport_guide
