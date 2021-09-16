#include "request_handler.h"

namespace request_handler {

json::Array getPassingBuses(const transport_guide::info::Stop& stop_info){
    json::Array result;
    for (const transport_guide::info::Bus* passing_bus : stop_info.passing_buses){
        result.push_back(json::Node(std::string(passing_bus->getName())));
    }
    return result;
}

void printSvgDoc(std::ostream& out, const json::Document& doc_to_print){
    out << doc_to_print.GetRoot().AsArray()[0].AsDict().at("map").AsString();
}

} // namespace request handler
