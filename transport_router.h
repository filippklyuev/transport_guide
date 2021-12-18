#pragma once

#include <algorithm>
#include <functional>
#include <optional>
#include <map>
#include <memory>
#include <set>
#include <iostream>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "transport_catalogue.h"
#include "graph.h"
#include "ranges.h"
#include "router.h"

namespace transport_guide {

struct RoutingSettings {
    int bus_wait_time = 0;
    double bus_velocity = 0.0;
};

namespace router {

using Graph = graph::DirectedWeightedGraph<double>;
using Router = graph::Router<double>;
using VertexId = size_t;
using EdgeId = size_t;



struct VertexInfo {
	VertexId id = 0;
	const info::Stop* stop_info;
};

struct Route {
	bool is_cycled = false;
	std::vector<const VertexInfo*> route;
};

struct EdgeInfo {
	std::string_view bus_name;
	int span = 0;
	double weight = 0.0;
	std::string_view from_stop;
	std::string_view to_stop;
};

struct RouteInfo {
	double overall_time = 0.0;
	std::vector<const EdgeInfo*> route_edges;
};

using MapOfRoutes = std::map<std::string_view, Route>;

class TransportRouter {
public:
	
	explicit TransportRouter(const TransportCatalogue& catalogue, RoutingSettings routing_settings);

	std::optional<RouteInfo> GetRouteInfo(std::string_view from, std::string_view to) const ;

	double getWaitWeight() const;

	double getBusVelocity() const;

	const std::vector<VertexInfo>& getVerticesInfo() const ;

	const std::unordered_map<EdgeId, EdgeInfo> getEdgesInfo() const ;

	const Graph& getGraph() const;

	const Router& getRouter() const;

private:
	const TransportCatalogue& catalogue_;
	double wait_weight_ = 0.0;
	double bus_velocity_ = 0.0;
	std::unique_ptr<Graph> graph_;
	std::unique_ptr<Router> router_;

	std::vector<VertexInfo> vertices_info_;
	std::unordered_map<std::string_view,const VertexInfo*> stops_info_;
	std::unordered_map<EdgeId, EdgeInfo> edges_info_;


	template<typename O>
	void processOneDirection(std::vector<const VertexInfo*> vertex_vector,const int from_pos, const int lim
													,std::string_view bus_name, O op);

	void assignVertices();

	MapOfRoutes getRoutes() const ;

	void connectVertexToReachableNoTransfer(const Route& route_info, const int from_position, std::string_view bus_name);

	void fillGraphWithEdges(const MapOfRoutes& routes);

	double calculateWeight(int distance) const ;

	int getDistance(const info::Stop* from, const info::Stop* to) const ;
};

} //namespace router

}//namespace transport_guide