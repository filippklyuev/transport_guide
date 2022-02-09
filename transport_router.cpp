#include "transport_router.h"

namespace transport_guide {

namespace router {

TransportRouter::TransportRouter(const TransportCatalogue& catalogue, RoutingSettings routing_settings)
		: catalogue_(catalogue)
		, wait_weight_(routing_settings.bus_wait_time)
		, bus_velocity_(routing_settings.bus_velocity)
		{
			vertices_info_.reserve(catalogue.GetStopsSet().size());
			assignVertices();
			MapOfRoutes routes = getRoutes();
			graph_ = std::make_unique<Graph>(vertices_info_.size());
			fillGraphWithEdges(std::move(routes));
			router_ = std::make_unique<Router>(*graph_);
	}

void TransportRouter::assignVertices(){
	for (const auto& [bus_name, bus_info] : catalogue_.GetBusesMap()){
		for (const info::Stop* stop : bus_info.stops){
			if (stops_info_.count(stop->name) == 0){
				vertices_info_.push_back(VertexInfo({vertices_info_.size(), stop}));
				const VertexInfo* info_ptr = &(vertices_info_.at(vertices_info_.size() - 1));
				stops_info_.emplace(stop->name, info_ptr);
			}
		}
	}	
}

template<typename O>
void TransportRouter::processOneDirection(std::vector<const VertexInfo*> vertex_vector,const int from_pos, const int lim
												,std::string_view bus_name, O op){
	int distance = 0;
	double weight = wait_weight_;
	int span = 0;
	
	const VertexInfo* from = vertex_vector[from_pos];

	int i = from_pos;
	for (op(i); i != lim; op(i)){
		const VertexInfo* to = vertex_vector[i];
		int distance_between_adjacent_stops = getDistance(vertex_vector[i < from_pos ? i + 1 : i - 1]->stop_info, to->stop_info);
		distance += distance_between_adjacent_stops;
		weight += calculateWeight(distance_between_adjacent_stops);
		span += 1;
		if (from != to){
			edges_info_.emplace(graph_->AddEdge({from->id, to->id, weight})
					, EdgeInfo{bus_name, span, weight, from->stop_info->name, to->stop_info->name});
		}
	}
}

MapOfRoutes TransportRouter::getRoutes() const {
	MapOfRoutes routes;
	for (const auto& [bus_name, bus_info] : catalogue_.GetBusesMap()){
		std::vector<const VertexInfo*> route;
		for (const info::Stop* stop : bus_info.stops){
			route.push_back(stops_info_.at(stop->name));
		}
		routes.emplace(bus_name, Route{bus_info.is_cycled, std::move(route)});
	}
	return routes;
}

const Graph& TransportRouter::getGraph() const {
	return *graph_;
}

const Router& TransportRouter::getRouter() const {
	return *router_;
}


void TransportRouter::connectVertexToReachableNoTransfer(const Route& route_info, const int from_position, std::string_view bus_name){
	const auto& vertex_vector = route_info.route;

	processOneDirection(vertex_vector, from_position, vertex_vector.size(), bus_name, [](int& x){++x;});
	if (!route_info.is_cycled){
		processOneDirection(vertex_vector, from_position, -1, bus_name, [](int& x){--x;});
	}

}

void TransportRouter::fillGraphWithEdges(const MapOfRoutes& routes){
	for (const auto& [bus_name, route_info] : routes){
		for (int i = 0; i < route_info.route.size(); i++){
			connectVertexToReachableNoTransfer(route_info, i, bus_name);
		}
	}
}

double TransportRouter::calculateWeight(int distance) const {
	if (bus_velocity_ == 0.0){
		return 0.0;
	}
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

double TransportRouter::getBusVelocity() const {
	return bus_velocity_;
}

const std::vector<VertexInfo>& TransportRouter::getVerticesInfo() const {
	return vertices_info_;
}

const std::unordered_map<EdgeId, EdgeInfo> TransportRouter::getEdgesInfo() const {
	return edges_info_;
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