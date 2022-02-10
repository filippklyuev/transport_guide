#include "request_handler.h"

namespace transport_guide {

namespace request_handler {

json::Array getPassingBuses(const StopInfo& stop_info){
    json::Array result;
    for (const info::Bus* passing_bus : stop_info.passing_buses){
        result.push_back(json::Node(std::string(passing_bus->getName())));
    }
    return result;
}

void printSvgDoc(std::ostream& out, const json::Document& doc_to_print){
    for (const auto& node : doc_to_print.GetRoot().AsArray()) {
        if (node.AsDict().find("map") != node.AsDict().end()) {
            out << node.AsDict().at("map").AsString();
        }
    }
    // out << doc_to_print.GetRoot().AsArray()[0].AsDict().at("map").AsString();
}

} // namespace request handler

} // namespace transport_guide
