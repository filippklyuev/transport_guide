#include "request_handler.h"

namespace transport_guide {

namespace request_handler {

json::Array getPassingBuses(const StopInfo& stop_info){
    json::Array result;
    for (const transport_guide::info::Bus* passing_bus : stop_info.passing_buses){
        result.push_back(json::Node(std::string(passing_bus->getName())));
    }
    return result;
}

void printSvgDoc(std::ostream& out, const json::Document& doc_to_print){
    out << doc_to_print.GetRoot().AsArray()[0].AsDict().at("map").AsString();
}

// double RouterManager::calculateWeight(const StopInfo& from, const StopInfo& to) const {
//     int distance;
//     from.distance_to_stops.count(to.name) ? distance = from.distance_to_stops.at(to.name) : distance = to.distance_to_stops.at(from.name);
//     // std::cout << "Distance from " << from.name << " to " << to.name << " equals " << distance << '\n';
//     return (static_cast<double>(distance) / (bus_velocity / 60.0 * 1000.0));
// }

// void RouterManager::addStopsAndWaitTransferEdges(const std::vector<StopInfo*>& stops){
//     static VertexId vertex_count = 0;
//     std::cout << "\n\n\n";
//     std::string_view prev_stop_name = "";
//     std::unordered_set<std::string_view> stops_set;
//     int counter = 0;
//     for (const auto stop : stops){
//         counter += 1;

//         if (stops_set.count(stop->name) && (counter != stops.size())){
//             // stop_vertexInfo.at(stop->name).doubles_on_route = true;
//             vertex_stopInfo.emplace(vertex_count, StopNameAndInfo(stop->name, VERTEX_TYPE::DOUBLER));
//             auto& [wait_vertex, doubler, to_same, buses_vertex] = stop_vertexInfo.at(stop->name);
//             doubler = vertex_count;
//             // stop_vertexInfo.at(stop->name).wait_vertex;
//             edge_type.emplace(graph_->AddEdge({wait_vertex, *doubler, wait_weight}), EDGE_TYPE::WAIT);
//             std::cout << "Adding doubler with id " << *doubler << " for stop " << stop->name << '\n';
//             std::cout << "Adding WaitEdge from " << wait_vertex << " to " << *doubler << '\n';
//             if (prev_stop_name == stop->name){
//                 to_same = true;
//             }
//             vertex_count += 1;
//         }
//         else if (!stop_vertexInfo.count(stop->name)){
//             //Adding StartWait vertex for a new stop
//             VertexId wait_vertex = vertex_count;
//             vertex_stopInfo.emplace(wait_vertex, StopNameAndInfo(stop->name, VERTEX_TYPE::WAIT_START));
//             std::cout << "Added wait_vertex " << stop->name << " with id " << wait_vertex << '\n';
//             stop_vertexInfo.emplace(stop->name, VertexInfo(wait_vertex));
//             for (const auto bus : stop->passing_buses){
//                 //Adding StartRide vertex to a route
//                 vertex_stopInfo.emplace(++vertex_count, StopNameAndInfo(stop->name, VERTEX_TYPE::RIDE_START, bus->name));
//                 std::cout << "Added Stop " << stop->name << " of bus " << bus->name << " with id " << vertex_count << '\n';
//                 auto& buses_vertex = stop_vertexInfo.at(stop->name).buses_vertex;
//                 buses_vertex.emplace(bus->name, vertex_count);
//                 //Adding Wait edge to a new stop
//                 std::cout << "Adding Wait Edge from " << wait_vertex << " to " << vertex_count << '\n';
//                 edge_type.emplace(graph_->AddEdge({wait_vertex, vertex_count, wait_weight}), EDGE_TYPE::WAIT);
//             }
//             vertex_count += 1;
//         }
//         prev_stop_name = stop->name;
//         stops_set.insert(stop->name);
//     }
    
// }

// VertexId RouterManager::getVertexIdForStop(std::string_view stop_name, std::string_view bus_name) const {
//     return stop_vertexInfo.at(stop_name).buses_vertex.at(bus_name);
// }

// void RouterManager::processSingleRoute(const BusInfo& bus_info){
//     addStopsAndWaitTransferEdges(bus_info.stops);
//     const auto& stops = bus_info.stops;
//     VertexId from, to;
//     double weight;
//     std::string_view bus_name = bus_info.name;
//     std::cout << "\n\n\n";
//     // EdgeId prev_edge;
//     std::unordered_set<std::string_view> used_doubled_stops;
//     for (int i = 0; i < stops.size() - 1; i++){

//         weight = calculateWeight(*stops[i], *stops[i + 1]);
//         from = getVertexIdForStop(stops[i]->name, bus_name);
//         const auto& [wait_vertex, doubler, to_same, buses_vertex] = stop_vertexInfo.at(stops[i]->name);
//         const auto& [next_wait_vertex, next_doubler, next_to_same, next_buses_vertex] = stop_vertexInfo.at(stops[i + 1]->name);
//         if (i < stops.size() - 2){
//             if (doubler){
//                 if (used_doubled_stops.count(stops[i]->name)){
//                     from = *doubler;
//                 } else if (to_same){
//                     //Handle route to itself
//                     // to = *doubler;
//                     std::cout << "Adding To same ride Edge from " << from << " to " << *doubler << " with weight " << weight << '\n';
//                     edge_type.emplace(graph_->AddEdge({from, *doubler, weight}), EDGE_TYPE::RIDE);
//                     used_doubled_stops.insert(stops[i]->name);  
//                     continue ;
//                     // edge_type.emplace(graph_->AddEdge({to, from, 0.0}), EDGE_TYPE::RIDE);                
//                 } else {
//                     // Adding Transfer Edge to doubled stop
//                     std::cout << "Adding Transfer Doubler Edge from " << from << " to " << *doubler << '\n';
//                     used_doubled_stops.insert(stops[i]->name);
//                     edge_type.emplace(graph_->AddEdge({from, *doubler, wait_weight}), EDGE_TYPE::WAIT);                       
//                 }
//             }
//             // Adding RideEdge from i stop to i + 1 stop
//             if (next_doubler){
//                 if (used_doubled_stops.count(stops[i + 1]->name)){
//                     to = *next_doubler;
//                 } else {
//                     used_doubled_stops.insert(stops[i + 1]->name);
//                 }
//             } else {
//                to = getVertexIdForStop(stops[i + 1]->name, bus_name); 
//             }
//             std::cout << "Adding Ride Edge from " << from << " to " << to << " with weight " << weight << '\n';
//             edge_type.emplace(graph_->AddEdge({from, to, weight}), EDGE_TYPE::RIDE);
//         }
//         bool next_is_transferable = stops[i + 1]->passing_buses.size() > 1;
//         //Handling last stop (going straight to StartWait vertex) or TransferStops
//         if (i == stops.size() - 2 || next_is_transferable){
//             // weight = calculateWeight(*stops[i], *stops[i + 1]);
//             // from = getVertexIdForStop(stops[i]->name, bus_name);
//             to = stop_vertexInfo.at(stops[i + 1]->name).wait_vertex;
//             std::cout << "Adding Edge To Wait from " << from << " to " << to << " with weight " << weight << '\n';
//             edge_type.emplace(graph_->AddEdge({from, to, weight}), EDGE_TYPE::RIDE);           
//         }
//         // prev_edge = 
//     }

//     if (!bus_info.is_cycled){
//         used_doubled_stops.clear();
//         //Handling backroute
//         for (int i = stops.size() - 1; i > 0; i--){
//             weight = calculateWeight(*stops[i], *stops[i - 1]);
//             from = getVertexIdForStop(stops[i]->name, bus_name);
//             const auto& [wait_vertex, doubler, to_same, buses_vertex] = stop_vertexInfo.at(stops[i]->name);
//             const auto& [next_wait_vertex, next_doubler, next_to_same, next_buses_vertex] = stop_vertexInfo.at(stops[i - 1]->name);            
//             if (i > 1){
//                 if (doubler){
//                     if (used_doubled_stops.count(stops[i]->name)){
//                         from = *doubler;
//                     } else if (to_same){
//                         //Handle route to itself
//                         // to = *doubler;
//                         std::cout << "Adding To same ride Edge from " << from << " to " << *doubler << " with weight " << weight << '\n';
//                         edge_type.emplace(graph_->AddEdge({from, *doubler, weight}), EDGE_TYPE::RIDE);
//                         used_doubled_stops.insert(stops[i]->name);  
//                         continue ;
//                         // edge_type.emplace(graph_->AddEdge({to, from, 0.0}), EDGE_TYPE::RIDE);                
//                     } else {
//                         // Adding Transfer Edge to doubled stop
//                         std::cout << "Adding Transfer Doubler Edge from " << from << " to " << *doubler << '\n';
//                         used_doubled_stops.insert(stops[i]->name);
//                         edge_type.emplace(graph_->AddEdge({from, *doubler, wait_weight}), EDGE_TYPE::WAIT);                       
//                     }
//                 }
//                 if (next_doubler){
//                     if (used_doubled_stops.count(stops[i - 1]->name)){
//                         to = *next_doubler;
//                     } else {
//                         used_doubled_stops.insert(stops[i - 1]->name);
//                     }
//                 } else {
//                    to = getVertexIdForStop(stops[i - 1]->name, bus_name); 
//                 }                         
//                 // to = getVertexIdForStop(stops[i - 1]->name, bus_name);
//                 std::cout << "Adding Ride Edge from " << from << " to " << to << " with weight " << weight << '\n';
//                 edge_type.emplace(graph_->AddEdge({from, to, weight}), EDGE_TYPE::RIDE);
//             }
//             // Handling first stop in cycled route (going straight to StartWait vertex) or nexttransferable stop
//             bool next_is_transferable = stops[i - 1]->passing_buses.size() > 1;
//             if (i == 1 || next_is_transferable){
//                 // weight = calculateWeight(*stops[i], *stops[i - 1]);
//                 // from = getVertexIdForStop(stops[i]->name, bus_name);
//                 to = stop_vertexInfo.at(stops[i - 1]->name).wait_vertex;
//                 std::cout << "Adding Ride Edge to Wait from " << from << " to " << to << " with weight " << weight << '\n';
//                 edge_type.emplace(graph_->AddEdge({from, to, weight}), EDGE_TYPE::RIDE);                
//             }
//         }
//     }
// }

// std::optional<RouteInfo> RouterManager::GetRouteInfo(const std::string& stop_from, const std::string& stop_to){
//     // RouterManager::RouteInfo result;
//     // std::cout << "Finding route from " << stop_vertex.at(stop_from) << " to " << stop_vertex.at(stop_to) << '\n';
//     VertexId from = stop_vertexInfo.at(stop_from).wait_vertex;
//     VertexId to_wait = stop_vertexInfo.at(stop_to).wait_vertex;
//     graph::Router<double>::RouteInfo shortest_route;
//     // Finding all the variants
//     bool begin = true;
//     std::optional<graph::Router<double>::RouteInfo> route_to_wait_vertex = router_->BuildRoute(from, to_wait);
//     for (const auto& [bus_name, vertex_to] : stop_vertexInfo.at(stop_to).buses_vertex){
//         std::cout << "Trying to find route from stop " << stop_from << " with wait_id " << from << " to stop " << stop_to << " with vertex id " << vertex_to << '\n';
//         std::optional<graph::Router<double>::RouteInfo> built_route = router_->BuildRoute(from, vertex_to);
//         // std::cout << "built_route length = " <<
//         if (!built_route){
//             // std::cout << "Nullopt\n";
//             return std::nullopt;
//         }
//         if (begin){
//             //initializing shortest_route            
//             if (route_to_wait_vertex){
//                 shortest_route = *route_to_wait_vertex;
//             } else {
//                 shortest_route = *built_route;
//             }
//         } 
//         begin = false;
//         if (built_route->weight < shortest_route.weight) { shortest_route = *built_route; }
//     }
//     double overall_time = shortest_route.weight;
//     std::vector<RouteElemInfo> route_elems;
//     std::cout << "shortest_route = " << overall_time << '\n';
//     const auto& edges_vector = shortest_route.edges;
//     // std::cout << edges_vector.size() << '\n';
//     for (size_t i = 0; i < edges_vector.size(); i++){
//         // std::cout << "i = " << i << '\n';
//         auto edge = graph_->GetEdge(edges_vector[i]);
//         EDGE_TYPE type = edge_type.at(edges_vector[i]);
//         if (type == EDGE_TYPE::WAIT){
//             route_elems.push_back({EDGE_TYPE::WAIT, edge.weight, vertex_stopInfo.at(edge.from).stop_name});
//         } else if (type == EDGE_TYPE::RIDE){
//             int span = 0;
//             double span_weight = 0;
//             // const StopInfo& start_stopinfo = catalogue_.GetStopInfo(vertex_stopInfo.at(edge.from).stop_name);
//             std::string_view bus_name = *(vertex_stopInfo.at(edge.from).bus_name);
//             std::cout << "Starting span: at vertex " << edge.from << " and weight " << edge.weight << '\n';
//             for (size_t j = i; j < edges_vector.size(); j++){
//                 edge = graph_->GetEdge(edges_vector[j]);
//                 type = edge_type.at(edges_vector[j]);
//                 if (j != i){
//                     // std::cout << "Continue span: at vertex " << edge.from << " and weight " << edge.weight << "; Span = " << span << "; And span weight " << overall_time << '\n';
//                 }
//                 std::cout << "Edge #" << edges_vector[j];
//                 if (edge_type.at(edges_vector[j]) == EDGE_TYPE::RIDE){
//                     std::cout << " type = GO from id " << edge.from << " to " << edge.to << '\n'; 
//                 } else {
//                     std::cout << " type = WAIT from id " << edge.from << " to " << edge.to << '\n'; 
//                 }
//                 // std::cout << "from " << vertex_stopInfo.at(edge.from) << '\n';
                
//                 if (type == EDGE_TYPE::WAIT){
//                     route_elems.push_back({EDGE_TYPE::RIDE, span_weight, bus_name, span});
//                     i = j - 1;
//                     break ;
//                 } else if (j + 1 == edges_vector.size()){
//                     span_weight += edge.weight;
//                     span += 1;
//                     route_elems.push_back({EDGE_TYPE::RIDE, span_weight, bus_name, span});
//                     return RouteInfo({overall_time, std::move(route_elems)});
//                 }
//                 span += 1;
//                 span_weight += edge.weight;
//             }
//         }
//     }
//     // std::optional<RouteInfo> result = {overall_time, std::move(route_elems)};
//     return RouteInfo({overall_time, std::move(route_elems)});
// }

// void RouterManager::createEdges(){
//     for (const auto& [bus_name, bus_info] : catalogue_.GetBusesMap()){
//         processSingleRoute(bus_info);
//     }   
// }

// size_t RouterManager::calculateNbrOfVertexes() const {
//     size_t result = 0;
//     // size_t cycled_routes = 0; // Not needed?
//     for (const auto& [bus_name, bus_info] : catalogue_.GetBusesMap()){
//         // (bus_info.is_cycled) ? result += bus_info.stops.size() - 1 : result += bus_info.stops.size();
//         // result += bus_info.stops.size();
//     }
//     return result + (catalogue_.GetStopsSet().size()); // All the unique stops need 1 wait vertex
// }

} // namespace request handler

} // namespace transport_guide
