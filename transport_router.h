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
	
	explicit TransportRouter(const TransportCatalogue& catalogue, RoutingSettings routing_settings)
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

	std::optional<RouteInfo> GetRouteInfo(std::string_view from, std::string_view to) const ;

	double getWaitWeight() const;

private:
	const TransportCatalogue& catalogue_;
	double wait_weight_ = 0.0;
	double bus_velocity_ = 0.0;
	std::unique_ptr<Graph> graph_;
	std::unique_ptr<Router> router_;

	struct RouteDetails {
		explicit RouteDetails(double wait_weight)
			: wait_weight_(wait_weight)
		{
			Reset();
		}
	
		void Reset(){
			weight = wait_weight_;
			span = 0;
			distance = 0;		
		}
	
		const double wait_weight_ = 0.0;
		double weight = 0.0;
		int span = 0;
		int distance = 0;
	};	

	std::vector<VertexInfo> vertices_info_;
	std::unordered_map<std::string_view,const VertexInfo*> stops_info_;
	std::unordered_map<EdgeId, EdgeInfo> edges_info_;

	void updateRouteDetails(int distance_between_adjacent_stops, TransportRouter::RouteDetails& route_details) const;

	void assignVertices();

	MapOfRoutes getRoutes() const ;

	void connectVertexToReachableNoTransfer(const Route& route_info, const int from_position, std::string_view bus_name);

	void fillGraphWithEdges(const MapOfRoutes& routes);

	double calculateWeight(int distance) const ;

	int getDistance(const info::Stop* from, const info::Stop* to) const ;
};

} //namespace router

}//namespace transport_guide