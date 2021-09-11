#include <iomanip>
#include <iostream>
#include <string_view>
#include <utility>
#include <chrono>

#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"

using namespace transport_guide;

void Test(){
	std::vector<Query> input_queries = input::GetQueries(false);
	TransportCatalogue catalogue;
	input::updateCatalogue(catalogue, input_queries);
	std::vector<Query> output_queries = input::GetQueries(true);
	output::PrintQueriesResult(catalogue, output_queries);
}

int main(){
	std::cout << std::setprecision(6);
	Test();
}
