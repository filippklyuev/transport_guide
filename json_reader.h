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

namespace transport_guide {

enum class QueryType {
    STOP,
    BUS
};

struct ParsedStopQuery  {
    std::string_view name;
    geo::Coordinates coordinates = {};
    DistanceMap distance_to_stops = {};
};

struct ParsedBusQuery {
    std::string_view name;
    bool is_cycled;
    std::vector<std::string_view> stops_on_route;
};          

namespace json_reader {

using DistanceMap = std::unordered_map<std::string_view, int>;

namespace parser {

void updateCatalogue(const json::Array& requests_vector, TransportCatalogue& catalogue);

ParsedStopQuery parseStopRequest(const json::Dict& stop_request);

ParsedBusQuery parseBusRequest(const json::Dict& bus_request);

class StatParser {
public:
    StatParser(const TransportCatalogue& catalogue, map_renderer::RenderSettings settings) :
        catalogue_(catalogue),
        settings_(settings)
    {}

    json::Array parseStatArray(const json::Array& requests_vector);

private:
    const TransportCatalogue& catalogue_;
    map_renderer::RenderSettings settings_;

    json::Dict parseSingleStatRequest(const json::Dict& request);

    void updateResultWithBusInfo(json::Dict& result, const info::Bus& bus_info);

    void updateResultWithStopInfo(json::Dict& result, const info::Stop& stop_info);

    void updateResultWithMap(json::Dict& result);
};

map_renderer::RenderSettings parseRenderSettings(const json::Dict& render_settings);

namespace detail {

void ParseAndInsertColor(svg::Color& empty_color_place, const json::Node& color_node);  

std::vector<std::string_view> parseStopsArray(const json::Array& stops);

DistanceMap GetDistanceToStops(const json::Dict& distance_to_stops);

} // namespace detail

} // namespace parser

} // namespace json_reader

} // namespace transport_guide