#include <iomanip>
#include <iostream>
#include <string_view>
#include <utility>

#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"

// using namespace transport_guide;

void Test(){
	int nbr_of_queries = transport_guide::input::read::LineWithNumber();
	transport_guide::TransportCatalogue catalogue(nbr_of_queries);
	transport_guide::input::read::Queries(catalogue, nbr_of_queries);
	std::cout << std::setprecision(6);
	catalogue.ProcessInputQueries();
	nbr_of_queries = transport_guide::input::read::LineWithNumber();
	std::vector<transport_guide::output::OutputQuery> output_queries = transport_guide::output::read::Queries(catalogue, nbr_of_queries);
	transport_guide::output::print::Output(catalogue, output_queries);		
}

int main(){
	Test();
	// std::cout << "Test Complete!" << std::endl;
}