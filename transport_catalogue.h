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

	using DistanceMap = std::unordered_map<std::string_view, int>;

	struct Stop {

		Stop(std::string_view name_, geo::Coordinates coordinates_, DistanceMap distance_to_stops_)
			: name(name_)
			, coordinates(coordinates_)
			, distance_to_stops(distance_to_stops_)
			{}

		std::string_view name = {};
		geo::Coordinates coordinates = {};
		DistanceMap distance_to_stops = {};
		std::set<Bus*, InfoPtrComparator<Bus>> passing_buses = {};

		std::string_view getName() const;
	};

	struct Bus {

		Bus(std::string_view name_, bool is_cycled_)
		: name(name_)
		, is_cycled(is_cycled_)
		{}

		std::string_view name;
		std::unordered_set<std::string_view> unique_stops = {};
		std::vector<Stop*> stops = {};
		bool is_cycled = false;
		double geo_route_length = 0.0;
		int64_t factial_route_length = 0;

		void updateBackRoute();

		void updateDistance();

		void updatePassingBus();

		size_t getUniqueStopsCount() const;

		std::string_view getName() const;
	};

} //namespace info	

class TransportCatalogue {
public:
	TransportCatalogue() = default;
	
	using BusMap = std::map<std::string_view, info::Bus>; // поменял на map для избежания инвалидации итераторов
	using StopMap = std::map<std::string_view, info::Stop>;

	void AddStop(std::string_view temp_stop_name, geo::Coordinates coords, info::DistanceMap&& distance_map);

	void AddRoute(std::string_view bus_name_temp, bool is_cycled, std::vector<std::string_view>&& stops_on_route);

	bool IsBusListed(const std::string_view bus_name) const ;

	bool IsStopListed(const std::string_view stop_name) const ;

	const info::Bus& GetBusInfo(std::string_view bus_name) const ;

	const info::Stop& GetStopInfo(const std::string_view stop) const ;

	
	
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

	info::Bus& GetBusInfo(std::string_view bus_name);

	info::Stop& GetStopInfo(const std::string_view stop);

	void processStopsOnRoute(info::Bus& bus_info, std::vector<std::string_view> stops_on_route);

	info::DistanceMap InsertSvsAndGetNewMap(info::DistanceMap temp_map);
};
	
} // namespace transport_guide
