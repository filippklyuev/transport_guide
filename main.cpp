#include <chrono>
#include <iostream>
#include <fstream>
#include <string_view>
#include <transport_catalogue.pb.h>
#include "json.h"
#include "json_reader.h"
#include "transport_router.h"
#include "request_handler.h"
#include "serialization.h"

void TestOld(){
    json::Document input_json = json::Load(std::cin);
    transport_guide::TransportCatalogue catalogue;
    const json::Dict& all_requests = input_json.GetRoot().AsDict();
    transport_guide::json_reader::updateCatalogue(all_requests.at("base_requests").AsArray(),  catalogue);
    auto render_settings = transport_guide::json_reader::parseRenderSettings(all_requests.at("render_settings").AsDict());
    auto routing_settings = transport_guide::json_reader::parseRoutingSettings(all_requests.at("routing_settings").AsDict());
    transport_guide::json_reader::StatParser stat_parser(&catalogue, std::move(render_settings), std::move(routing_settings));
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
        std::string filename = all_requests.at("serialization_settings").AsDict().at("file").AsString();
        transport_guide::Serializer serializer(filename, catalogue, render_settings, routing_settings);
        serializer.SerializeTransportCatalogue();      
    } else if (mode == "process_requests"sv) {
        json::Document output_json = json::Load(std::cin);
        const json::Dict& output_requests = output_json.GetRoot().AsDict();
        std::string filename = output_requests.at("serialization_settings").AsDict().at("file").AsString();
        catalogue_proto::TransportCatalogue proto_catalogue = transport_guide::DeserializeCatalogue(filename);
        transport_guide::json_reader::StatParser stat_parser(&proto_catalogue);
        json::Document result_to_print(stat_parser.parseStatArray(output_requests.at("stat_requests").AsArray()));
        json::Print(result_to_print, std::cout);
        // transport_guide::request_handler::printSvgDoc(std::cout, result_to_print);
    // } else if (mode == "test"sv){
    //     json::Document answer = json::Load(std::cin);
    //     transport_guide::request_handler::printSvgDoc(std::cout, answer);
        // TestOld();
    } else {
        PrintUsage();
        return 1;        
    }
}
