#pragma once
#include <iostream>
#include <variant>
#include <vector>
#include <string>
#include <sstream>
#include <string_view>
#include <utility>
#include <unordered_map>

#include "domain.h"
#include "json.h"
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

class StatParser {
public:
    StatParser(const transport_guide::TransportCatalogue& catalogue, map_renderer::RenderSettings settings) :
        catalogue_(catalogue),
        settings_(settings)
    {}

    json::Array parseStatArray(const json::Array& requests_vector);

private:
    const transport_guide::TransportCatalogue& catalogue_;
    map_renderer::RenderSettings settings_;

    json::Dict parseSingleStatRequest(const json::Dict& request);

    void updateResultWithBusInfo(json::Dict& result, const transport_guide::info::Bus& bus_info);

    void updateResultWithStopInfo(json::Dict& result, const transport_guide::info::Stop& stop_info);

    void updateResultWithMap(json::Dict& result);
};

map_renderer::RenderSettings parseRenderSettings(const json::Dict& render_settings);

namespace detail {

std::vector<std::string_view> parseStopsArray(const json::Array& stops);

DistanceMap GetDistanceToStops(const json::Dict& distance_to_stops);

} // namespace detail

} // namespace parser

} // namespace json_reader