#include <chrono>
#include <iostream>
#include "json.h"
#include "json_reader.h"
#include "request_handler.h"

// #include "log_duration.h"

void Test(){
    // LOG_DURATION("ALL_TEST");
    json::Document input_json = json::Load(std::cin);
    transport_guide::TransportCatalogue catalogue;
    const json::Dict& all_requests = input_json.GetRoot().AsDict();
    transport_guide::json_reader::parser::updateCatalogue(all_requests.at("base_requests").AsArray(),  catalogue);
    // std::cout << "Base" << '\n';
    transport_guide::info::RoutingSettings routing_settings = transport_guide::json_reader::parser::parseRoutingSettings(all_requests.at("routing_settings").AsDict());
    // std::cout << "RoutingSettings" << '\n';
    transport_guide::json_reader::parser::StatParser stat_parser(catalogue, transport_guide::json_reader::parser::parseRenderSettings(all_requests.at("render_settings").AsDict()), routing_settings);
    // std::cout << "StatParser" << '\n';
    json::Document result_to_print(stat_parser.parseStatArray(all_requests.at("stat_requests").AsArray()));
    // request_handler::printSvgDoc(std::cout, result_to_print); // Printing SVG doc only
    json::Print(result_to_print, std::cout);    
}

int main(){
    // std::ios_base::sync_with_stdio(0);
    Test();
}

