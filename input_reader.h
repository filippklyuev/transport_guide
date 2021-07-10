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
#include "transport_catalogue.h"

namespace transport_guide {

namespace input {

namespace to_catalogue {

void PutStops(transport_guide::TransportCatalogue& catalogue, const std::vector<std::string>& stops_strings);

void PutBuses(transport_guide::TransportCatalogue& catalogue, const std::vector<std::string>& buses_strings);

} // namespace to_catalogue

struct Queries {

	enum class Type {
		STOP,
		BUS
	};

	std::vector<std::string> stops_strings;
	std::vector<std::string> buses_strings;
};

Queries::Type DefineQueryType(const std::string& query);

Queries GetQueriesByType();

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

std::unordered_map<std::string_view, int> GetStopDistances(transport_guide::TransportCatalogue& catalogue, std::string_view stop_name, const std::string& stop_query, uint64_t pos_last);

std::pair<std::string_view, transport_guide::info::Bus> GetBusNameAndRoute(transport_guide::TransportCatalogue& catalogue, const std::string& bus_query);
	
} //namespace parse

} // namespace input

} // namespace transport_guide