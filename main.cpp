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
    transport_guide::json_reader::updateCatalogue(all_requests.at("base_requests").AsArray(),  catalogue);
    auto render_settings = transport_guide::json_reader::parseRenderSettings(all_requests.at("render_settings").AsDict());
    transport_guide::json_reader::StatParser stat_parser(catalogue, std::move(render_settings));
    json::Document result_to_print(stat_parser.parseStatArray(all_requests.at("stat_requests").AsArray()));
    // request_handler::printSvgDoc(std::cout, result_to_print); // Printing SVG doc only
    json::Print(result_to_print, std::cout);    
}

int main(){
    // std::ios_base::sync_with_stdio(0);
    Test();
}

