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

struct Query;

namespace output {

void PrintQueriesResult(const transport_guide::TransportCatalogue& catalogue, const std::vector<transport_guide::Query>& output_queries, std::ostream& out);

std::ostream& operator<<(std::ostream& out, const transport_guide::info::Bus& info);

std::ostream& operator<<(std::ostream& out, const transport_guide::info::Stop& info);

namespace detail {

std::string_view GetName(const std::string& query);

} //namespace detail

} // namespace output

} // namespace transport_guide
