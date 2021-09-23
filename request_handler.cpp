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

void RouterManager::addWaitEdge(VertexId id_from){
    VertexId id_to = id_from + 1;
    // std::cout << "vertx.from = " << id_from << "; vertx.to = " << id_to << "\n\n";
    EdgeId edge_id = graph_->AddEdge({id_from, id_to, wait_weight});
    edges_info.emplace(edge_id, EDGE_TYPE::WAIT);
}

double RouterManager::findWeightForGoEdge(int distance){
    return (static_cast<double>(distance) / (bus_velocity / 60.0 * 1000.0));
}

void RouterManager::addGoEdge(const RouterManager::StopInfo& from, const RouterManager::StopInfo& to, VertexId id_from, VertexId id_to){
    double weight;
    if (from.distance_to_stops.count(to.name)){
        weight = findWeightForGoEdge(from.distance_to_stops.at(to.name));
    } else {
        weight = findWeightForGoEdge(to.distance_to_stops.at(from.name));
    }

    // std::cout << "StopFrom = " << from.name << "; vertx.from = " << id_from << "; vertx.to = " << id_to << "\n\n";
    EdgeId edge_id = graph_->AddEdge({id_from, id_to, weight});
    edges_info.emplace(edge_id, EDGE_TYPE::GO);
}

void RouterManager::addStopsAndWaitEdges(const std::vector<transport_guide::info::Stop*>& stops){
    // DUPLICATES REMOVE !!!!!!
    static VertexId vertex_count = 0;
    for (const auto stop : stops){
        if (!stop_vertex.count(stop->name)){
            stop_vertex.emplace(stop->name, vertex_count + 1);
            vertex_stop.emplace(vertex_count + 1, stop->name);
            addWaitEdge(vertex_count);
            vertex_count += 2;
        }
    }
}

void RouterManager::processSingleRoute(const RouterManager::BusInfo& bus_info){
    addStopsAndWaitEdges(bus_info.stops);
    const auto& stops = bus_info.stops;
    for (int i = 0; i < stops.size() - 1; i++){
        bool next_is_last = (i == stops.size() - 2);
        if (!next_is_last){
            // std::cout << "Adding go from " << stops[i]->name << " to " << stops[i + 1]->name << '\n';
            addGoEdge(*stops[i], *stops[i + 1], stop_vertex.at(stops[i]->name), stop_vertex.at(stops[i + 1]->name));  
        }
        bool next_is_transferable = (stops[i + 1]->passing_buses.size()) > 1;
        if (next_is_transferable || next_is_last){
            // std::cout << "Adding transferable from " << stops[i]->name << " to " << stops[i + 1]->name << '\n';
            addGoEdge(*stops[i], *stops[i + 1], stop_vertex.at(stops[i]->name), stop_vertex.at(stops[i + 1]->name) - 1);
        }
    }
    // DUPLICATES REMOVE !!!!!!
    if (!bus_info.is_cycled){
        for (int i = stops.size() - 1; i > 0; i--){
            addGoEdge(*stops[i], *stops[i - 1], stop_vertex.at(stops[i]->name), stop_vertex.at(stops[i - 1]->name));
            bool next_is_transferable = (stops[i - 1]->passing_buses.size()) > 1;
            if (next_is_transferable){
                addGoEdge(*stops[i], *stops[i - 1], stop_vertex.at(stops[i]->name), stop_vertex.at(stops[i - 1]->name) - 1);
            }            
        }
    }
}

std::vector<std::string_view> RouterManager::getPassingBusNames(const RouterManager::StopInfo& stop_info){
    std::vector<std::string_view> result;
    for (BusInfo* bus_info : stop_info.passing_buses){
        result.push_back(bus_info->getName());
    }
    return result;
}

RouterManager::RouteInfo RouterManager::GetRouteInfo(const std::string& stop_from, const std::string& stop_to){
    RouterManager::RouteInfo result;
    std::cout << "Finding route from " << stop_vertex.at(stop_from) << " to " << stop_vertex.at(stop_to) << '\n';
    std::optional<graph::Router<double>::RouteInfo> res = router_->BuildRoute(stop_vertex.at(stop_from) - 1, stop_vertex.at(stop_to) - 1);
    if (!res.has_value()){
        return result;
    } else {
        result.is_successful = true;
    }
    result.overall_time = res->weight;
    const auto& edges_vector = res->edges;
    // std::cout << edges_vector.size() << '\n';
    for (size_t i = 0; i < edges_vector.size(); i++){
        // std::cout << "i = " << i << '\n';
        auto edge = graph_->GetEdge(edges_vector[i]);
        if (edges_info.at(edges_vector[i]) == EDGE_TYPE::WAIT){
            result.route_elems.push_back({EDGE_TYPE::WAIT, edge.weight, vertex_stop.at(edge.to), std::nullopt});
        } else {
            int span = 0;
            double span_weight = 0;
            const StopInfo& start_stopinfo = catalogue_.GetStopInfo(vertex_stop.at(edge.from));
            std::string_view bus_name = getPassingBusNames(start_stopinfo)[0];
            std::cout << "Starting span:\n";
            for (size_t j = i; j < edges_vector.size(); j++){
                edge = graph_->GetEdge(edges_vector[j]);
                std::cout << "Edge #" << edges_vector[j];
                if (edges_info.at(edges_vector[j]) == EDGE_TYPE::GO){
                    std::cout << " type = GO from id " << edge.from << " to " << edge.to << '\n'; 
                } else {
                    std::cout << " type = WAIT from id " << edge.from << " to " << edge.to << '\n'; 
                }
                // std::cout << "from " << vertex_stop.at(edge.from) << '\n';
                span += 1;
                if (edges_info.at(edges_vector[j]) == EDGE_TYPE::WAIT || j + 1 == edges_vector.size()){
                    result.route_elems.push_back({EDGE_TYPE::GO, span_weight, bus_name, span});
                    if (j + 1 == edges_vector.size()){
                        return result;
                    }
                    i = j - 1; // !!!!!!!!!!!!!!!!!!!!!!!! j - 1 
                    std::cout << "Breaking i = " << i << '\n';
                    break ;
                }
                span_weight += edge.weight;
            }
        }
    }
    return result;
}

void RouterManager::createEdges(){
    for (const auto& [bus_name, bus_info] : catalogue_.GetBusesMap()){
            processSingleRoute(bus_info);
    }   
}

} // namespace request handler
