#pragma once
#include <algorithm>
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

// #include "log_duration.h"

namespace transport_guide {

namespace router {

using Graph = graph::DirectedWeightedGraph<double>;
using Router = graph::Router<double>;
using VertexId = size_t;
using EdgeId = size_t;



struct VertexInfo {
	VertexId id;
	const info::Stop* stop_info;
};

struct Route {
	bool is_cycled = false;
	std::vector<const VertexInfo*> route;
};

struct EdgeInfo {
	std::string_view bus_name;
	int span;
	double weight;
	std::string_view from_stop;
	std::string_view to_stop;
};

struct RouteInfo {
	double overall_time;
	std::vector<const EdgeInfo*> route_edges;
};


class TransportRouter {
public:
	using MapOfRoutes = std::map<std::string_view, Route>;

	explicit TransportRouter(const TransportCatalogue& catalogue, info::RoutingSettings routing_settings)
		: catalogue_(catalogue)
		, wait_weight_(routing_settings.bus_wait_time)
		, bus_velocity_(routing_settings.bus_velocity)
		{
			// LOG_DURATION("TRANSPORT_ROUTER_CONSTRUCTOR");
			MapOfRoutes routes = assignVerticesGetRoutes();
			graph_ = std::make_unique<Graph>(vertices_info_.size());
			fillGraphWithEdges(std::move(routes));
			router_ = std::make_unique<Router>(*graph_);
		}

	std::optional<RouteInfo> GetRouteInfo(std::string_view from, std::string_view to) const ;

	double getWaitWeight() const;

private:
	const TransportCatalogue& catalogue_;
	double wait_weight_;
	double bus_velocity_;
	std::unique_ptr<Graph> graph_;
	std::unique_ptr<Router> router_;

	std::unordered_map<VertexId, VertexInfo> vertices_info_;
	std::unordered_map<std::string_view,const VertexInfo*> stops_info_;
	std::unordered_map<EdgeId, EdgeInfo> edges_info_;

	MapOfRoutes assignVerticesGetRoutes();

	void connectVertexToReachableNoTransfer(const Route& route_info, const int from_position, std::string_view bus_name);

	void fillGraphWithEdges(const MapOfRoutes& routes);

	double calculateWeight(int distance) const ;

	int getDistance(const info::Stop* from, const info::Stop* to) const ;
};

} //namespace router

}//namespace transport_guide