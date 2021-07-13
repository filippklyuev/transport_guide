#include <iomanip>
#include <iostream>
#include <string_view>
#include <utility>
#include <chrono>

#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"

void Test(){
	std::vector<transport_guide::Query> input_queries = transport_guide::input::GetQueries(bool{false});
	transport_guide::TransportCatalogue catalogue;
	transport_guide::input::ParseInput(catalogue, input_queries);
	std::vector<transport_guide::Query> output_queries = transport_guide::input::GetQueries(bool{true});
	transport_guide::output::PrintQueriesResult(catalogue, output_queries);
}

int main(){
	std::cout << std::setprecision(6);
	Test();
}
