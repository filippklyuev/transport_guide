#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include "geo.h"
#include "input_reader.h"
#include "transport_catalogue.h"

namespace transport_guide {

namespace output {

struct Query {
	std::string query;
	std::string_view short_query;
	transport_guide::QueryType type;
};

std::vector<Query> GetQueries();

void PrintQueriesResult(const transport_guide::TransportCatalogue& catalogue, const std::vector<Query>& output_queries);

std::ostream& operator<<(std::ostream& out, const transport_guide::info::Bus& info);

std::ostream& operator<<(std::ostream& out, const transport_guide::info::Stop& info);

namespace detail {

std::string_view GetShortQuery(const std::string& query);

} //namespace detail

} // namespace output

} // namespace transport_guide
