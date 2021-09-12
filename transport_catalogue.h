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

enum class QueryType {
	STOP,
	BUS
};

class TransportCatalogue;

namespace info {

	template<typename InfoPtr>
	struct InfoPtrComparator {
		bool operator()(const InfoPtr* lhs,const InfoPtr* rhs) const {
			return lhs->name < rhs->name;
		}
	};

	struct Bus;

	struct Nameable {

		std::string_view name;

		std::string_view getName() const;

	};

	using DistanceMap = std::unordered_map<std::string_view, int>;

	struct Stop : Nameable {

		geo::Coordinates coordinates = {};
		DistanceMap distance_to_stops = {};
		std::set<Bus*, InfoPtrComparator<Bus>> passing_buses = {};

		Stop& setName(std::string_view stop_name);

		Stop& setCoordinates(geo::Coordinates coords);

		Stop& setDistanceToStops(DistanceMap map);

	};

	struct Bus : Nameable {

		std::unordered_set<std::string_view> unique_stops = {};
		std::vector<Stop*> stops = {};
		bool is_cycled = false;
		double geo_route_length = 0.0;
		int64_t factial_route_length = 0;

		Bus& setName(std::string_view stop_name);

		Bus& setIsCycled(bool is_cycled);

		Bus& setStopsAndDistance(const TransportCatalogue& catalogue, std::vector<std::string_view> stops_on_route_temp);

		void updateBackRoute();

		void updateDistance();

		void updatePassingBus();

		size_t getUniqueStopsCount() const;

	};

} //namespace info	

class TransportCatalogue {
public:
	TransportCatalogue() = default;
	
	using BusMap = std::map<std::string_view, info::Bus>; // поменял на map для избежания инвалидации итераторов
	using StopMap = std::map<std::string_view, info::Stop>;

	std::string_view InsertNameSV(std::string_view name, QueryType type);

	void AddStop(std::string_view name);

	void AddRoute(std::string_view name);

	bool IsBusListed(const std::string_view bus_name) const ;

	bool IsStopListed(const std::string_view stop_name) const ;

	const info::Bus& GetRouteInfo(std::string_view bus_name) const ;

	info::Bus& GetRouteInfo(std::string_view bus_name);

	const info::Stop& GetStopInfo(const std::string_view stop) const ;

	info::Stop& GetStopInfo(const std::string_view stop);
	
private:

	std::set<std::string> buses_; // Поменял с unordered_set, из-за инвалидации ссылок (и string_view) при возможном рехешировании
	std::set<std::string> stops_;

	StopMap stops_map_;
	BusMap buses_map_;

	std::string_view  InsertBusName(std::string bus_name);

	std::string_view  InsertStopName(std::string stop_name);

	StopMap& GetStopsMap();

	const StopMap& GetStopsMap() const;

	BusMap& GetBusesMap();

	const BusMap& GetBusesMap() const;
};
	
} // namespace transport_guide
