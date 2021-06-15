#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include "geo.h"
#include "transport_catalogue.h"

namespace transport_guide {

namespace output {

struct OutputQuery {
	std::string_view query;
	bool is_stop_query = false;
};

namespace read {

std::vector<OutputQuery> Queries(TransportCatalogue& catalogue, const int nbr_of_queries);

} // namespace read

namespace print {
	
void Output(const TransportCatalogue& catalogue, const std::vector<OutputQuery>& queries_to_print);

template <typename Info>
void QueryInfo(const Info& info){
	std::cout << info << std::endl;
}

std::ostream& operator<<(std::ostream& out, const info::Bus& info);

std::ostream& operator<<(std::ostream& out, const info::Stop& info);

} //namespace print

namespace detail {

std::string_view TransformBusName(const std::string& bus_name);

} //namespace detail

} // namespace output

} // namespace transport_guide
