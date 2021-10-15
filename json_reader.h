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
#include "json_builder.h"
#include "svg.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

namespace transport_guide {

enum class QueryType {
    STOP,
    BUS,
    MAP
};

namespace json_reader {

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

void updateCatalogue(const json::Array& requests_vector, TransportCatalogue& catalogue);

ParsedStopQuery parseStopRequest(const json::Dict& stop_request);

ParsedBusQuery parseBusRequest(const json::Dict& bus_request);

class StatParser {
public:
    StatParser(const TransportCatalogue& catalogue,map_renderer::RenderSettings&& settings) :
        catalogue_(catalogue),
        settings_(std::move(settings))
    {}

    json::Document parseStatArray(const json::Array& requests_vector) const;

private:
    const TransportCatalogue& catalogue_;
    const map_renderer::RenderSettings settings_;

    void parseSingleStatRequest(const json::Dict& request, json::Builder& builder) const;

    svg::Document getSvgDoc() const;

    QueryType defineRequestType(std::string_view type) const;

    bool isValidRequest(const json::Dict& request, QueryType type) const;

    void parseStopRequest(const json::Dict& request, json::Builder& builder) const;

    void parseBusRequest(const json::Dict& request, json::Builder& builder) const;

    void parseMapRequest(const json::Dict& request, json::Builder& builder) const;

};

map_renderer::RenderSettings parseRenderSettings(const json::Dict& render_settings);

} // namespace json_reader

} // namespace transport_guide