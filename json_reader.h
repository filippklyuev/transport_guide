#pragma once
#include <memory>
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
#include "transport_router.h"
#include "request_handler.h"
#include "transport_catalogue.h"

namespace transport_guide {

enum class QueryType {
    STOP,
    BUS,
    MAP,
    ROUTE
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

RoutingSettings parseRoutingSettings(const json::Dict& routing_settings);

class StatParser {
public:
    StatParser(const TransportCatalogue& catalogue, map_renderer::RenderSettings settings, RoutingSettings routing_settings) :
        catalogue_(catalogue),
        settings_(settings),
        routing_settings_(routing_settings)
    {}

    json::Document parseStatArray(const json::Array& requests_vector);

private:
    std::unique_ptr<router::TransportRouter> router_manager_ = nullptr;
    const TransportCatalogue& catalogue_;
    map_renderer::RenderSettings settings_;
    RoutingSettings routing_settings_;

    void parseSingleStatRequest(const json::Dict& request,json::Builder& builder);

    void updateResultWithBusInfo(json::Builder& builder, const info::Bus& bus_info);

    void updateResultWithStopInfo(json::Builder& builder, const info::Stop& stop_info);

    void updateResultWithMap(json::Builder& builder);

    json::Node getMapAsNode();

    QueryType defineRequestType(std::string_view type);

    bool isValidRequest(const json::Dict& request, QueryType type);

    void updateResultWithRoute(json::Builder& builder, const std::string& from, const std::string& to);
};

map_renderer::RenderSettings parseRenderSettings(const json::Dict& render_settings);

} // namespace parser

} // namespace json_reader

} // namespace transport_guide