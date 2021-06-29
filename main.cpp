#include <iomanip>
#include <iostream>
#include <string_view>
#include <utility>

#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"

// using namespace transport_guide;

void Test(){
	int queries_count = transport_guide::input::read::LineWithNumber();
	transport_guide::TransportCatalogue catalogue(queries_count);
	transport_guide::input::read::Queries(catalogue, queries_count);
	std::cout << std::setprecision(6);
	catalogue.ProcessInputQueries();
	queries_count = transport_guide::input::read::LineWithNumber();
	std::vector<transport_guide::output::OutputQuery> output_queries = transport_guide::output::read::Queries(catalogue, nbr_of_queries);
	transport_guide::output::print::Output(catalogue, output_queries);		
}

int main(){
	Test();
}