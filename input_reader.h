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

namespace read {

std::string Line();
	
int LineWithNumber();
	
void Queries(transport_guide::TransportCatalogue& catalogue, const int nbr_of_queries);

}

} // namespace input

} // namespace transport_guide