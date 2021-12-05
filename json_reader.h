#pragma once
#include <memory>
#include <iostream>
#include <variant>
#include <vector>
#include <optional>
#include <string>
#include <sstream>
#include <string_view>
#include <utility>
#include <unordered_map>

#include <transport_catalogue.pb.h>
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

RoutingSettings parseRoutingSettings(const json::Dict& routing_settings);

class StatParser {
public:
    StatParser(const TransportCatalogue* catalogue, map_renderer::RenderSettings&& settings, RoutingSettings routing_settings) :
        catalogue_(catalogue),
        settings_(std::move(settings)),
        routing_settings_(routing_settings)
    {}

    StatParser(const catalogue_proto::TransportCatalogue* proto_catalogue)
        : proto_catalogue_(proto_catalogue)
    {}

    json::Document parseStatArray(const json::Array& requests_vector);

private:
    std::unique_ptr<router::TransportRouter> router_manager_ = nullptr;
    const TransportCatalogue* catalogue_ = nullptr;
    const catalogue_proto::TransportCatalogue* proto_catalogue_ = nullptr;
    std::optional<std::unordered_map<std::string_view, int>> proto_stops_map_ = std::nullopt;
    std::optional<std::unordered_map<std::string_view, int>> proto_buses_map_ = std::nullopt;
    const map_renderer::RenderSettings settings_;
    const RoutingSettings routing_settings_;

    void parseSingleStatRequest(const json::Dict& request, json::Builder& builder);

    svg::Document getSvgDoc() const;

    QueryType defineRequestType(std::string_view type) const;

    bool isValidRequest(const json::Dict& request, QueryType type) const;

    void parseStopRequest(const json::Dict& request, json::Builder& builder) const;

    void parseStopRequestProto(const json::Dict& request, json::Builder& builder) const;

    void parseBusRequest(const json::Dict& request, json::Builder& builder) const;

    void parseBusRequestProto(const json::Dict& request, json::Builder& builder) const;

    void parseMapRequest(const json::Dict& request, json::Builder& builder) const;

    void parseRouteRequest(const json::Dict& request, json::Builder& builder);

    void initializeProtoMap(QueryType request_type);
};

map_renderer::RenderSettings parseRenderSettings(const json::Dict& render_settings);

} // namespace json_reader

} // namespace transport_guide