#pragma once
#include <iomanip>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include "geo.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

namespace transport_guide {

enum class QueryType {
	STOP,
	BUS
};

QueryType DefineQueryType(const std::string& query);

struct Query {
	std::string query;
	std::string_view short_query;
	transport_guide::QueryType type;
};

namespace input {

void ParseInput(transport_guide::TransportCatalogue& catalogue,const std::vector<transport_guide::Query>& input_queries);

std::vector<transport_guide::Query> GetQueries(bool is_for_output);

namespace read {

int LineWithNumber();

std::string Line();

} // namespace read

namespace parse {

namespace detail {

std::string_view GetStopNameSV(const std::string& stop_str);

char DefineSeparator(const std::string& bus_query);

} // namespace detail

std::pair<std::string_view, transport_guide::info::Stop> GetStopNameAndInfo (transport_guide::TransportCatalogue& catalogue, const std::string& stop_query);

std::unordered_map<std::string_view, int> GetStopDistances(transport_guide::TransportCatalogue& catalogue, const std::string& stop_query, uint64_t pos_last);

std::pair<std::string_view, transport_guide::info::Bus> GetBusNameAndRoute(transport_guide::TransportCatalogue& catalogue, const std::string& bus_query);
	
} //namespace parse

} // namespace input

} // namespace transport_guide