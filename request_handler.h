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

namespace request_handler {

using Graph = graph::DirectedWeightedGraph<double>;
using Router = graph::Router<double>;
using VertexId = size_t;
using EdgeId = size_t;    

json::Array getPassingBuses(const transport_guide::info::Stop& stop_info);

void printSvgDoc(std::ostream& out, const json::Document& doc_to_print);
    
class RouterManager {
public:

    enum class EDGE_TYPE {
        WAIT,
        GO
    };

    struct RouteElemInfo {
        EDGE_TYPE type;
        double time;
        std::string_view name = {};
        std::optional<int> span = std::nullopt;
    };

    struct RouteInfo {
        bool is_successful = false;
        double overall_time = 0.0;
        std::vector<RouteElemInfo> route_elems;  
    };

    RouterManager(const transport_guide::TransportCatalogue& catalogue, transport_guide::info::RoutingSettings routing_settings) 
        : catalogue_(catalogue){
            wait_weight = routing_settings.bus_wait_time;
            bus_velocity = routing_settings.bus_velocity;
            graph_ = std::make_unique<Graph>(catalogue_.GetStopsMap().size() * 2);
            createEdges();
            // std::cout << "HERE " << '\n';
            router_ = std::make_unique<Router>(*graph_);
        }

    RouteInfo GetRouteInfo(const std::string& stop_from, const std::string& stop_to);

using BusInfo = transport_guide::info::Bus;
using StopInfo = transport_guide::info::Stop;

private:
    const transport_guide::TransportCatalogue& catalogue_;
    std::unique_ptr<Router> router_ = nullptr;
    std::unique_ptr<Graph> graph_ = nullptr;
    std::unordered_map<std::string_view, VertexId> stop_vertex;
    std::unordered_map<VertexId, std::string_view> vertex_stop;
    std::map<EdgeId, EDGE_TYPE> edges_info;
    double wait_weight = 0.0;
    double bus_velocity = 0.0;

    void createEdges();

    void addGoEdge(const StopInfo& from, const StopInfo& to, VertexId id_from, VertexId id_to);

    double findWeightForGoEdge(int distance);

    void addWaitEdge(VertexId id_from);

    void processSingleRoute(const BusInfo& bus_info);

    std::vector<std::string_view> getPassingBusNames(const StopInfo& stop_info);

    void addStopsAndWaitEdges(const std::vector<transport_guide::info::Stop*>& stops);
};

} // namespace request handler

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
/*
class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};
*/