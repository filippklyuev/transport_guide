#include <chrono>
#include <iostream>
#include <fstream>
#include <string_view>
#include "json.h"
#include "json_reader.h"
#include "transport_router.h"
#include "request_handler.h"
#include "serialization.h"

#include <transport_catalogue.pb.h>

void TestOld(){
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

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        json::Document input_json = json::Load(std::cin);
        transport_guide::TransportCatalogue catalogue;
        const json::Dict& all_requests = input_json.GetRoot().AsDict();
        transport_guide::json_reader::updateCatalogue(all_requests.at("base_requests").AsArray(),  catalogue);
        auto render_settings = transport_guide::json_reader::parseRenderSettings(all_requests.at("render_settings").AsDict());
        auto routing_settings = transport_guide::json_reader::parseRoutingSettings(all_requests.at("routing_settings").AsDict());
        string filename = all_requests.at("serialization_settings").AsDict().at("file");
        transport_guide::SerializeTransportCatalogue(filename, catalogue);      
    } else if (mode == "process_requests"sv) {

        // process requests here

    } else {
        PrintUsage();
        return 1;
    }
}