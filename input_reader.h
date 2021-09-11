#pragma once
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <utility>
#include <unordered_map>
#include "geo.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

namespace transport_guide {

QueryType DefineQueryType(const std::string& query);

struct Query {
	std::string query;
	QueryType type;
};

using DistanceMap = std::unordered_map<std::string_view, int>;

namespace input {

struct ParsedStopQuery  {
	std::string_view name;
	double lattitude;
	double longtitude;
	DistanceMap distance_to_stops = {};
};

struct ParsedBusQuery {
	std::string_view name;
	bool is_cycled;
	std::vector<std::string_view> stops_on_route;
};

void updateCatalogue(TransportCatalogue& catalogue,const std::vector<Query>& input_queries);

std::vector<Query> GetQueries(std::istream& in);

namespace read {

int LineWithNumber(std::istream& in);

std::string Line(std::istream& in);

} // namespace read

namespace parse {

input::ParsedStopQuery parseStopQuery(std::string_view stop_query);

DistanceMap getStopDistances(std::string_view distance_to_stops_query);

input::ParsedBusQuery parseBusQuery(std::string_view bus_query);

void updateBusInfo(TransportCatalogue& catalogue, std::vector<std::string_view> stops_on_route_temp, bool is_cycled, info::Bus& bus_info);

namespace detail {

std::string_view GetStopNameSV(const std::string& stop_str);

char DefineSeparator(std::string_view bus_query);

void updateDistance(const std::vector<info::Stop*>& stops_vec, info::Bus& result);

void updateBackRoute(const std::vector<info::Stop*>& stops_vec, info::Bus& result);

} // namespace detail 
	
} //namespace parse

namespace detail {

DistanceMap InsertSvsAndGetNewMap(TransportCatalogue& catalogue, DistanceMap temp_map);

void updatePassingBus(info::Bus* bus_info, std::vector<info::Stop*>& stops_vec);

} // namespace detail

} // namespace input

} // namespace transport_guide