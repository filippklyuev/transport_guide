#include "input_reader.h"

using namespace transport_guide::input;

std::string read::Line() {
    std::string s;
    std::getline(std::cin, s);
    return s;
}

int read::LineWithNumber() {
    int result;
    std::cin >> result;
    Line();
    return result;
}

void read::Queries(transport_guide::TransportCatalogue& catalogue, const int nbr_of_queries){
	for (int i = 0; i < nbr_of_queries; i++){
		std::getline(std::cin, catalogue.GetInputQueries()[i]);
	}
}
