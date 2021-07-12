#include <iomanip>
#include <iostream>
#include <string_view>
#include <utility>
#include <chrono>

#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"

// using namespace transport_guide;

void Test(){
	LOG_DURATION("2.0");
	std::vector<transport_guide::input::Query> input_queries = transport_guide::input::GetQueries();
	transport_guide::TransportCatalogue catalogue;
	transport_guide::input::ParseInput(catalogue, input_queries);
	// transport_guide::input::to_catalogue::PutStops(catalogue, input_queries.stops_strings);
	// transport_guide::input::to_catalogue::PutBuses(catalogue, input_queries.buses_strings);
	std::vector<transport_guide::output::Query> output_queries = transport_guide::output::GetQueries();
	// LOG_DURATION("PRINT");
	transport_guide::output::PrintQueriesResult(catalogue, output_queries);
}

int main(){
	std::cout << std::setprecision(6);
	Test();
}
