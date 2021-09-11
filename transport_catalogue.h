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
// #include "input_reader.h"

namespace transport_guide {

enum class QueryType {
	STOP,
	BUS
};

namespace info {

	template<typename InfoPtr>
	struct InfoPtrComparator {
		bool operator()(const InfoPtr* lhs,const InfoPtr* rhs) const {
			return lhs->name < rhs->name;
		}
	};

	struct Nameable {

		std::string_view name;

		std::string_view getName(){
			return name;
		}

	};

	struct Bus;

	struct Stop : Nameable {
		Stop(Stop&& other) = default;
		Stop() = default;

		geo::Coordinates coordinates = {};
		std::unordered_map<std::string_view, int> distance_to_stops = {};
		std::set<Bus*, InfoPtrComparator<Bus>> passing_buses = {};

	};

	struct Bus : Nameable {
		Bus(Bus&& other) = default;
		Bus() = default;

		bool operator<(const Bus& other) const {
			return this->name < other.name;
		}

		std::unordered_set<std::string_view> unique_stops = {};
		std::vector<Stop*> stops = {};
		bool is_cycled = false;
		double geo_route_length = 0.0;
		int64_t factial_route_length = 0;

		size_t getUniqueStopsCount(){
			return unique_stops.size();
		}
	};

} //namespace info	



class TransportCatalogue {
public:
	TransportCatalogue() = default;
	
	using BusMap = std::map<std::string_view, info::Bus>; // поменял на map для избежания инвалидации итераторов
	using StopMap = std::map<std::string_view, info::Stop>;

	void ProcessInputQueries();

	std::string_view GetSVFromInsertedName(std::string_view name, QueryType type);

	void AddStop(std::string_view name);

	void AddRoute(std::string_view name);

	bool IsBusListed(const std::string_view bus_name) const ;

	bool IsStopListed(const std::string_view stop_name) const ;

	const info::Bus& GetRouteInfo(std::string_view bus_name) const ;

	info::Bus& GetRouteInfo(std::string_view bus_name);

	const info::Stop& GetStopInfo(const std::string_view stop) const ;

	info::Stop& GetStopInfo(const std::string_view stop);

	// void AddDistanceToStop(const std::string_view stop, info::Bus& bus_info);

	// void AddBusToStop(const std::string_view bus_name, const std::string_view stop_name);

	// int GetBackRouteDistance(const info::Bus& bus_info);
	
private:

	std::set<std::string> buses_; // Поменял с unordered_set, из-за инвалидации ссылок (и string_view) при возможном рехешировании
	std::set<std::string> stops_;

	StopMap stops_map_;
	BusMap buses_map_;

	std::string_view  GetSVFromInsertedBusName(std::string bus_name);

	std::string_view  GetSVFromInsertedStopName(std::string stop_name);

	StopMap& GetStopsMap();

	const StopMap& GetStopsMap() const;

	BusMap& GetBusesMap();

	const BusMap& GetBusesMap() const;
};
	
} // namespace transport_guide
