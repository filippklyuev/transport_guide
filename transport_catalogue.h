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

#include "domain.h"
#include "geo.h"

namespace transport_guide {

class TransportCatalogue {
public:
	TransportCatalogue() = default;
	
	using BusMap = std::map<std::string_view, info::Bus>;
	using StopMap = std::map<std::string_view, info::Stop>;

	void AddStop(std::string_view temp_stop_name, geo::Coordinates coords, DistanceMap&& distance_map);

	void AddRoute(std::string_view bus_name_temp, bool is_cycled, std::vector<std::string_view>&& stops_on_route);

	bool IsBusListed(const std::string_view bus_name) const ;

	bool IsStopListed(const std::string_view stop_name) const ;

	const info::Bus& GetBusInfo(std::string_view bus_name) const ;

	const info::Stop& GetStopInfo(const std::string_view stop) const ;

	const std::set<std::string>& GetBusesSet() const;

	const std::set<std::string>& GetStopsSet() const;

	const BusMap& GetBusesMap() const;

	const StopMap& GetStopsMap() const;
	
private:

	std::set<std::string> buses_;
	std::set<std::string> stops_;

	StopMap stops_map_;
	BusMap buses_map_;

	std::string_view  InsertBusName(std::string bus_name);

	std::string_view  InsertStopName(std::string stop_name);

	StopMap& GetStopsMap();

	BusMap& GetBusesMap();

	info::Bus& GetBusInfo(std::string_view bus_name);

	info::Stop& GetStopInfo(const std::string_view stop);

	DistanceMap InsertSvsAndGetNewMap(DistanceMap temp_map);

	void updatePassingBusInStops(const info::Bus& bus_info);
};
	
} // namespace transport_guide
