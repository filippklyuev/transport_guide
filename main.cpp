#include <iomanip>
#include <iostream>
#include <string_view>
#include <utility>

#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"

// using namespace transport_guide;

void Test(){
	transport_guide::input::Queries input_queries = transport_guide::input::GetQueriesByType();
	transport_guide::TransportCatalogue catalogue;
	transport_guide::input::to_catalogue::PutStops(catalogue, input_queries.stops_strings);
	transport_guide::input::to_catalogue::PutBuses(catalogue, input_queries.buses_strings);
}


int main(){
	std::cout << std::setprecision(6);
	Test();
}
