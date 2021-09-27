#include "transport_router.h"

namespace transport_guide {

namespace router {

std::set<VertexId> TransportRouter::getPossibleToIds(std::string_view stop_to) const {
	// std::optional<std::string_view> bus_intersection = findBusIntersection(catalogue_.GetStopInfo(stop_from), catalogue_.GetStopInfo(stop_to));
	std::set<VertexId> result;
	const auto& to_stop_info = stops_routerinfo_.at(stop_to);
	if (to_stop_info.is_transferable){
		result.insert(to_stop_info.wait_vertex);
	}
	for (const auto& [bus_name, vertex] : to_stop_info.routes_vertices){
		result.insert(vertex->id);
	}
	for (const auto& [bus_name, vertex] : to_stop_info.routes_vertices){
		result.insert(vertex->id);
	}
	// if (bus_intersection){
	// 	result.push_back(to_stop_info.routes_vertices.at(*bus_intersection)->id);
	// 	if (to_stop_info.routes_doubles.count(*bus_intersection)){
	// 		result.push_back(to_stop_info.routes_doubles.at(*bus_intersection)->id);
	// 	}
	// } else {
	// 	if (to_stop_info.is_transferable){
	// 		// std::cout << "is_transferable\n";
	// 		result.push_back(to_stop_info.wait_vertex);
	// 	} 
	// 	for (const auto& [bus_name, vertex] : to_stop_info.routes_vertices){
	// 		result.push_back(vertex->id);
	// 	}
	// 	for (const auto& [bus_name, vertex] : to_stop_info.routes_doubles){
	// 		result.push_back(vertex->id);
	// 	}
	// }
	// for (const auto vtx : result){
	// 	std::cout << "possible vertex - " << vtx << " to stop " << stop_to << '\n';
	// }
	return result;	
}

std::optional<TransportRouter::RouteInfo> TransportRouter::GetRouteInfo(std::string_view stop_from, std::string_view stop_to){
	VertexId from_id = stops_routerinfo_.at(stop_from).wait_vertex;
	std::optional<Router::RouteInfo> fastest_route, buffer;
	bool begin = true;
	for (const VertexId to_id : getPossibleToIds(stop_to)){
		buffer = router_->BuildRoute(from_id, to_id);
		if (begin){
			if (!buffer){
				return std::nullopt;
			} else {
				fastest_route = buffer;
				begin = false;
			}
		}
		if (fastest_route->weight > buffer->weight){
			fastest_route = buffer;
		}
	}
	return TransportRouter::RouteInfo(translateRouteInfo(*fastest_route));
}

using RouteVerticesInfo = std::map<std::string_view, std::vector<VertexInfo*>>;

RouteVerticesInfo TransportRouter::assignVertices(){
	size_t vertices_count = 0;
	RouteVerticesInfo route_vertices_map;
	for (const auto& [bus_name, bus_info] : catalogue_.GetBusesMap()){
		std::unordered_set<std::string_view> used_stop_names_this_route;
		std::vector<VertexInfo*> vertices_on_route;
		for (const info::Stop* stop : bus_info.stops){
			if (used_stop_names_this_route.find(stop->name) != used_stop_names_this_route.end()){
				if (stop == bus_info.stops[bus_info.stops.size() - 1] && bus_info.is_cycled){
					stops_routerinfo_.at(stop->name).is_transferable = true;
					// std::cout << "Pushing da same\n";
					vertices_on_route.push_back(vertices_on_route[0]);
				} else {
					// std::cout << "Adding DOUBLER vertex with id " << vertices_count << " for stop_name " << stop->name << " and bus " << bus_name << '\n';
					vertices_info_.emplace(vertices_count, VertexInfo({/*vertex_id*/vertices_count, VERTEX_TYPE::DOUBLER, stop->name, bus_name
															,/*wait_vertex*/ getStopWaitVertex(stop->name), /*is_transferable*/ true		
											, /*is_end_point*/ (stop == bus_info.stops[bus_info.stops.size() - 1] || stop == bus_info.stops[0])}));
					// std::cout << "Is endpoint " << (stop == bus_info.stops[bus_info.stops.size() - 1] || stop == bus_info.stops[0]) << '\n';

					VertexInfo* info_ptr = &vertices_info_.at(vertices_count);
					// std::cout << "Check for isendpoint " << info_ptr->is_endpoint_of_route << '\n';
					vertices_on_route.push_back(info_ptr);
					auto& stop_info = stops_routerinfo_.at(stop->name);
					stop_info.is_transferable = true;
					stop_info.routes_vertices.at(bus_name)->is_transferable = true;
					stop_info.routes_doubles.emplace(bus_name, info_ptr);
					vertices_count += 1;
				}
			} else {
				if (stops_routerinfo_.find(stop->name) == stops_routerinfo_.end()){
					// std::cout << "Adding WAIT vertex with id " << vertices_count << " for stop_name " << stop->name << '\n';
					stops_routerinfo_.emplace(stop->name, StopRouterInfo(/*wait_vertex*/vertices_count, stop->passing_buses.size() > 1));
				
					vertices_count += 1;
				}
				// std::cout << "Adding RIDE_START vertex with id " << vertices_count << " for stop_name " << stop->name << " and bus " << bus_name << '\n';
				bool is_endpoint_of_route = (stop == bus_info.stops[bus_info.stops.size() - 1] || stop == bus_info.stops[0]);
				vertices_info_.emplace(vertices_count, VertexInfo({/*vertex_id*/vertices_count, VERTEX_TYPE::RIDE_START, stop->name, bus_name
														,/*wait_vertex*/ getStopWaitVertex(stop->name), /*is_transferable*/ stop->passing_buses.size() > 1
											, /*is_end_point*/ is_endpoint_of_route}));
				// std::cout << "Is endpoint " << (stop == bus_info.stops[bus_info.stops.size() - 1] || stop == bus_info.stops[0]) << '\n';

				VertexInfo* info_ptr = &vertices_info_.at(vertices_count);
				// std::cout << "Check for isendpoint " << info_ptr->is_endpoint_of_route << '\n';
				vertices_on_route.push_back(info_ptr);
				auto& stop_info = stops_routerinfo_.at(stop->name);
				stop_info.routes_vertices.emplace(bus_name, info_ptr);
				stop_info.is_transferable = is_endpoint_of_route;
				vertices_count += 1;
				used_stop_names_this_route.insert(stop->name);			
			}	
		}
		route_vertices_map.emplace(bus_name, std::move(vertices_on_route));
	}
	return route_vertices_map;
}

void TransportRouter::fillGraphWithEdges(const RouteVerticesInfo route_vertices){
	addWaitEdges();
	for (const auto& [bus_name, vertex_vector] : route_vertices){
		processSingleRoute(vertex_vector, catalogue_.GetBusInfo(bus_name).is_cycled);
	}
}

void TransportRouter::processPairOfStops(const VertexInfo* from, const VertexInfo* to){
	// std::cout << '\n';
	double weight = calculateWeight(catalogue_.GetStopInfo(from->stop_name), catalogue_.GetStopInfo(to->stop_name));
	if (to->is_transferable || to->is_endpoint_of_route){
		// std::cout << "Adding RIde Edge (transferable or endpoint) from " << from->id << " to " << to->wait_vertex << " with weight " << weight << '\n';
		edges_type_.emplace(graph_->AddEdge({from->id, to->wait_vertex, weight}), EDGE_TYPE::RIDE);
	}
	if (!to->is_endpoint_of_route){
		// std::cout << "Adding RIde Edge to next from " << from->id << " to " << to->id << " with weight " << weight << '\n';
		edges_type_.emplace(graph_->AddEdge({from->id, to->id, weight}), EDGE_TYPE::RIDE);
	}

}

void TransportRouter::processSingleRoute(const std::vector<VertexInfo*>& vertex_vector, bool is_cycled){
	for (int i = 0; i < vertex_vector.size() - 1; i++){
		processPairOfStops(vertex_vector[i], vertex_vector[i + 1]);
	}
	if (!is_cycled){
		for (int i = vertex_vector.size() - 1; i > 0; i--){
			processPairOfStops(vertex_vector[i], vertex_vector[i - 1]);
		}
	}
}

size_t TransportRouter::getVerticesCount() const {
	size_t result = 0;
	for (const auto& [bus_name, bus_info] : catalogue_.GetBusesMap()){
		result += bus_info.stops.size();
		//Убираю одну повторяющуюся остановку
		if (bus_info.is_cycled){
			result -= 1;
		}
	}
	//каждая уникальная остановка будет иметь одну vertex "ожидания" 
	return result + catalogue_.GetStopsSet().size();
}

void TransportRouter::addWaitEdges() {
	// std::cout << '\n';
	for (const auto& [stop , router_info] : stops_routerinfo_){
		for (const auto& [bus, route_vertex_info] : router_info.routes_vertices){
			// std::cout << "Adding WAIT_EDGE (to on dif routes) from " << router_info.wait_vertex << " to " << route_vertex_info->id << '\n'; 
			edges_type_.emplace(graph_->AddEdge({router_info.wait_vertex, route_vertex_info->id, wait_weight_}), EDGE_TYPE::WAIT);
		}
		for (const auto& [bus, double_vertex_info] : router_info.routes_doubles){
			// std::cout << "Adding WAIT_EDGE (to doubles) from " << router_info.wait_vertex << " to " << double_vertex_info->id << '\n'; 
			edges_type_.emplace(graph_->AddEdge({router_info.wait_vertex, double_vertex_info->id, wait_weight_}), EDGE_TYPE::WAIT);
		}
	}
}

double TransportRouter::calculateWeight(const info::Stop& from, const info::Stop& to) const {
    int distance;
    from.distance_to_stops.count(to.name) ? distance = from.distance_to_stops.at(to.name) : distance = to.distance_to_stops.at(from.name);
    // std::cout << "Distance from " << from.name << " to " << to.name << " equals " << distance << '\n';
    return (static_cast<double>(distance) / (bus_velocity_ / 60.0 * 1000.0));
}

TransportRouter::RouteInfo TransportRouter::translateRouteInfo(const Router::RouteInfo route_info) const {
	TransportRouter::RouteInfo result;
	result.overall_time = route_info.weight;
	const auto& edges = route_info.edges;
	for (int i = 0; i < edges.size(); i++){
		EdgeId edge_id = edges[i];
		if (edges_type_.at(edge_id) == EDGE_TYPE::WAIT){
			result.route_elems.push_back(TransportRouter::RouteInfo::ElemInfo({EDGE_TYPE::WAIT, wait_weight_
				, vertices_info_.at(graph_->GetEdge(edge_id).to).stop_name, 0}));
		} else {
			std::string_view bus_name = vertices_info_.at(graph_->GetEdge(edge_id).from).bus_name;
			int span = 0;
			double time = 0.0;
			for (int j = i; j < edges.size(); j++){
				edge_id = edges[j];
				if (edges_type_.at(edge_id) == EDGE_TYPE::RIDE){
					time += graph_->GetEdge(edge_id).weight;
					span += 1;
					if (j == edges.size() - 1){
						result.route_elems.push_back(TransportRouter::RouteInfo::ElemInfo({EDGE_TYPE::RIDE, time, bus_name, span}));
						i = j;
						break ;	
					}
				} else {
					result.route_elems.push_back(TransportRouter::RouteInfo::ElemInfo({EDGE_TYPE::RIDE, time, bus_name, span}));
					i = j - 1;
					break;
				}
			}
			// result.route_elems.push_back(TransportRouter::RouteInfo::ElemInfo({EDGE_TYPE::RIDE, time, bus_name, span}));
		}
	}
	return result;
}
std::optional<std::string_view> TransportRouter::findBusIntersection(const info::Stop& from, const info::Stop& to) const {
	std::set<std::string_view> from_buses, to_buses;

	for (const info::Bus* bus : from.passing_buses){
		from_buses.insert(bus->name);
	}
	for (const info::Bus* bus : to.passing_buses){
		to_buses.insert(bus->name);
	}
	std::vector<std::string_view> intersection;
	std::set_intersection(from_buses.begin(), from_buses.end()
						, to_buses.begin(), to_buses.end()
						, std::back_inserter(intersection));
	if (intersection.empty()){
		return std::nullopt;
	} else {
		return std::string_view(intersection[0]);
	}
}

VertexId TransportRouter::getStopWaitVertex(std::string_view stop_name) const {
	return stops_routerinfo_.at(stop_name).wait_vertex;
}

} //namespace router

} // namespace trasnport_guide