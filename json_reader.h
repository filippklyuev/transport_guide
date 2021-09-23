#pragma once
#include <iostream>
#include <memory>
#include <variant>
#include <vector>
#include <string>
#include <sstream>
#include <string_view>
#include <utility>
#include <unordered_map>

#include "domain.h"
#include "json.h"
#include "json_builder.h"
#include "svg.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

namespace json_reader {

using DistanceMap = std::unordered_map<std::string_view, int>;

namespace parser {

void updateCatalogue(const json::Array& requests_vector, transport_guide::TransportCatalogue& catalogue);

transport_guide::input::ParsedStopQuery parseStopRequest(const json::Dict& stop_request);

transport_guide::input::ParsedBusQuery parseBusRequest(const json::Dict& bus_request);

transport_guide::info::RoutingSettings parseRoutingSettings(const json::Dict& routing_settings);

class StatParser {
public:
    StatParser(const transport_guide::TransportCatalogue& catalogue, map_renderer::RenderSettings settings, transport_guide::info::RoutingSettings routing_settings) :
        catalogue_(catalogue),
        settings_(settings),
        routing_settings_(routing_settings)
    {}

    json::Document parseStatArray(const json::Array& requests_vector);

private:
    std::unique_ptr<request_handler::RouterManager> router_manager_ = nullptr;
    const transport_guide::TransportCatalogue& catalogue_;
    map_renderer::RenderSettings settings_;
    transport_guide::info::RoutingSettings routing_settings_;

    void parseSingleStatRequest(const json::Dict& request,json::Builder& builder);

    void updateResultWithBusInfo(json::Builder& builder, const transport_guide::info::Bus& bus_info);

    void updateResultWithStopInfo(json::Builder& builder, const transport_guide::info::Stop& stop_info);

    void updateResultWithMap(json::Builder& builder);

    void updateResultWithRoute(json::Builder& builder, const std::string& from, const std::string& to);

};

map_renderer::RenderSettings parseRenderSettings(const json::Dict& render_settings);

namespace detail {

std::vector<std::string_view> parseStopsArray(const json::Array& stops);

DistanceMap GetDistanceToStops(const json::Dict& distance_to_stops);

} // namespace detail

} // namespace parser

} // namespace json_reader