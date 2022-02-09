#include <chrono>
#include <iostream>
#include "json.h"
#include "json_reader.h"
#include "transport_router.h"
#include "request_handler.h"

void Test(){
    json::Document input_json = json::Load(std::cin);
    transport_guide::TransportCatalogue catalogue;
    const json::Dict& all_requests = input_json.GetRoot().AsDict();
    transport_guide::json_reader::updateCatalogue(all_requests.at("base_requests").AsArray(),  catalogue);
    auto render_settings = transport_guide::json_reader::parseRenderSettings(all_requests.at("render_settings").AsDict());
    auto routing_settings = transport_guide::json_reader::parseRoutingSettings(all_requests.at("routing_settings").AsDict());
    transport_guide::json_reader::StatParser stat_parser(catalogue, std::move(render_settings), std::move(routing_settings));
    json::Document result_to_print(stat_parser.parseStatArray(all_requests.at("stat_requests").AsArray()));
    json::Print(result_to_print, std::cout);    
}

int main(){
    Test();
}

