#include "transport_router.h"

namespace transport_guide {

namespace router {

MapOfRoutes TransportRouter::assignVerticesGetRoutes(){
	size_t vertices_count = 0;
	MapOfRoutes routes;
	for (const auto& [bus_name, bus_info] : catalogue_.GetBusesMap()){
		std::vector<const VertexInfo*> route;
		for (const info::Stop* stop : bus_info.stops){
			const VertexInfo* info_ptr;
			if (stops_info_.count(stop->name) == 0){
				vertices_info_.emplace(vertices_count, VertexInfo({vertices_count, stop}));
				info_ptr = &(vertices_info_.at(vertices_count));
				stops_info_.emplace(stop->name, info_ptr);
				vertices_count += 1;
			} else {
				info_ptr = stops_info_.at(stop->name);
			}
			route.push_back(info_ptr);
		}
		routes.emplace(bus_name, Route{bus_info.is_cycled, std::move(route)});
	}
	return routes;
}

void TransportRouter::connectVertexToReachableNoTransfer(const Route& route_info, const int from_position, std::string_view bus_name){
	const auto& vertex_info_vector = route_info.route;
	const VertexInfo* from = vertex_info_vector[from_position];
	TransportRouter::RouteDetails route_details(wait_weight_);
	if (!route_info.is_cycled){
		if (from_position > 0){
			for (int i = from_position - 1; i >= 0; i--){
				const VertexInfo* to = vertex_info_vector[i];
				int distance_between_adjacent_stops =  getDistance(vertex_info_vector[i + 1]->stop_info, to->stop_info);
				updateRouteDetails(distance_between_adjacent_stops, route_details);
				if (from != to){
					edges_info_.emplace(graph_->AddEdge({from->id, to->id, route_details.weight})
							, EdgeInfo{bus_name, route_details.span, route_details.weight, from->stop_info->name, to->stop_info->name});
				}
			}
		}
	}
	route_details.Reset();
	if (from_position < vertex_info_vector.size() - 1){
		for (int i = from_position + 1; i < vertex_info_vector.size(); i++){
			const VertexInfo* to = vertex_info_vector[i];
			int distance_between_adjacent_stops = getDistance(vertex_info_vector[i - 1]->stop_info, to->stop_info);
			updateRouteDetails(distance_between_adjacent_stops, route_details);
			if (from != to){
				edges_info_.emplace(graph_->AddEdge({from->id, to->id, route_details.weight})
						, EdgeInfo{bus_name, route_details.span, route_details.weight, from->stop_info->name, to->stop_info->name});
			}
		}
	}
}

void TransportRouter::updateRouteDetails(int distance_between_adjacent_stops, TransportRouter::RouteDetails& route_details) const {
	route_details.distance += distance_between_adjacent_stops;
	double weight_between_adjacent_stops = calculateWeight(distance_between_adjacent_stops);
	route_details.weight += weight_between_adjacent_stops;
	route_details.span += 1;
}

void TransportRouter::fillGraphWithEdges(const MapOfRoutes& routes){
	for (const auto& [bus_name, route_info] : routes){
		for (int i = 0; i < route_info.route.size(); i++){
			connectVertexToReachableNoTransfer(route_info, i, bus_name);
		}
	}
}

double TransportRouter::calculateWeight(int distance) const {
	return (static_cast<double>(distance) / (bus_velocity_ / 6.0 * 100.0));
}

int TransportRouter::getDistance(const info::Stop* from, const info::Stop* to) const {
    if (from->distance_to_stops.count(to->name)){
    	return from->distance_to_stops.at(to->name);
    } else if (to->distance_to_stops.count(from->name)){
    	return to->distance_to_stops.at(from->name);
    } else {
    	return 0;
    }
}

std::optional<RouteInfo> TransportRouter::GetRouteInfo(std::string_view stop_from, std::string_view stop_to) const {
	if (stops_info_.count(stop_from) && stops_info_.count(stop_to)){
		VertexId from = stops_info_.at(stop_from)->id;
		VertexId to = stops_info_.at(stop_to)->id;

		std::optional<Router::RouteInfo> result = router_->BuildRoute(from, to);
		if (result){
			std::vector<const EdgeInfo*> result_vector;
			for (EdgeId edge : result->edges){
				result_vector.push_back(&(edges_info_.at(edge)));
			}
			return RouteInfo{result->weight, std::move(result_vector)};
		}
	}
	return std::nullopt;
}

double TransportRouter::getWaitWeight() const {
	return wait_weight_;
}

} //namespace router

} // namespace trasnport_guide