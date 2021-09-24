#pragma once
#include <map>
#include <memory>
#include <optional>
#include <string_view>
#include <sstream>
#include <unordered_map>
#include <utility>

#include "graph.h"
#include "router.h"
#include "json.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace transport_guide {

namespace request_handler {

using Graph = graph::DirectedWeightedGraph<double>;
using Router = graph::Router<double>;
using VertexId = size_t;
using EdgeId = size_t;

using BusInfo = transport_guide::info::Bus;
using StopInfo = transport_guide::info::Stop; 

json::Array getPassingBuses(const StopInfo& stop_info);

void printSvgDoc(std::ostream& out, const json::Document& doc_to_print);

enum class EDGE_TYPE {
    WAIT,
    RIDE,
    TRANSFER
};

enum class VERTEX_TYPE {
    WAIT_START,
    RIDE_START
};

struct VertexInfo {
    VertexInfo(VertexId id)
        : wait_vertex(id)
    {}

    VertexId wait_vertex;
    std::unordered_map<std::string_view, VertexId> buses_vertex;
};

struct RouteElemInfo {
    EDGE_TYPE type;
    double time;
    std::string_view name;
    std::optional<int> span = std::nullopt;
};

struct RouteInfo {
    double overall_time = 0.0;
    std::vector<RouteElemInfo> route_elems;  
};

struct StopNameAndInfo {
    StopNameAndInfo(std::string_view stop_name_, VERTEX_TYPE type_)
        : stop_name(stop_name_)
        , type(type_)
    {}

    StopNameAndInfo(std::string_view stop_name_, VERTEX_TYPE type_, std::string_view bus_name_)
        : stop_name(stop_name_)
        , type(type_)
        , bus_name(bus_name_)
    {}    
    std::string_view stop_name;
    VERTEX_TYPE type;
    std::optional<std::string_view> bus_name = std::nullopt;
};

class RouterManager {
public:

    RouterManager(const transport_guide::TransportCatalogue& catalogue, transport_guide::info::RoutingSettings routing_settings) 
        : catalogue_(catalogue){
            wait_weight = routing_settings.bus_wait_time;
            bus_velocity = routing_settings.bus_velocity;
            graph_ = std::make_unique<Graph>(calculateNbrOfVertexes());
            createEdges();
            // std::cout << "HERE " << '\n';
            router_ = std::make_unique<Router>(*graph_);
        }

    std::optional<RouteInfo>  GetRouteInfo(const std::string& stop_from, const std::string& stop_to);

private:
    const transport_guide::TransportCatalogue& catalogue_;
    std::unique_ptr<Router> router_ = nullptr;
    std::unique_ptr<Graph> graph_ = nullptr;
    std::unordered_map<std::string_view, VertexInfo> stop_vertexInfo;
    std::unordered_map<VertexId, StopNameAndInfo> vertex_stopInfo;
    std::map<EdgeId, EDGE_TYPE> edge_type;
    double wait_weight = 0.0;
    double bus_velocity = 0.0;

    void createEdges();

    void addGoEdge(const StopInfo& from, const StopInfo& to, VertexId id_from, VertexId id_to);

    double calculateWeight(const StopInfo& from, const StopInfo& to) const ;

    // void addWaitEdge(VertexId id_from);

    void processSingleRoute(const BusInfo& bus_info);

    // std::vector<std::string_view> getPassingBusNames(const StopInfo& stop_info);

    void addStopsAndWaitTransferEdges(const std::vector<StopInfo*>& stops);

    size_t calculateNbrOfVertexes() const ;

    VertexId getVertexIdForStop(std::string_view stop_name, std::string_view bus_name) const ;
};

} // namespace request handler

} // namespace transport_guide