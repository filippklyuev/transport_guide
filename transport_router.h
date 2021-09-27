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

namespace router {

using Graph = graph::DirectedWeightedGraph<double>;
using Router = graph::Router<double>;
using VertexId = size_t;
using EdgeId = size_t;


enum class VERTEX_TYPE {
	RIDE_START,
	WAIT_START,
	DOUBLER
};

enum class EDGE_TYPE {
	RIDE,
	WAIT
};

struct VertexInfo {
	VertexId id;
	VERTEX_TYPE type;
	std::string_view stop_name;
	std::string_view bus_name;
	VertexId wait_vertex;
	bool is_transferable;
	bool is_endpoint_of_route;
	
};

struct StopRouterInfo {
	explicit StopRouterInfo(VertexId wait_vertex_id, bool is_transferable_)
		: wait_vertex(wait_vertex_id)
		, is_transferable(is_transferable_)
		{}

	VertexId wait_vertex;
	bool is_transferable;
	std::unordered_map<std::string_view, VertexInfo*> routes_vertices;
	std::unordered_map<std::string_view, VertexInfo*> routes_doubles;

};


class TransportRouter {
public:
	explicit TransportRouter(const TransportCatalogue& catalogue, info::RoutingSettings routing_settings)
		: catalogue_(catalogue)
		, wait_weight_(routing_settings.bus_wait_time)
		, bus_velocity_(routing_settings.bus_velocity)
		{
			graph_ = std::make_unique<Graph>(getVerticesCount());		
			fillGraphWithEdges(assignVertices());
			router_ = std::make_unique<Router>(*graph_);
		}

	struct RouteInfo {
		struct ElemInfo {
			EDGE_TYPE type;
			double time;
			std::string_view name;
			int span = 0;
		};

		double overall_time;
		std::vector<ElemInfo> route_elems;
	};



	std::optional<TransportRouter::RouteInfo> GetRouteInfo(std::string_view from, std::string_view to);

private:
	const TransportCatalogue& catalogue_;
	double wait_weight_;
	double bus_velocity_;
	std::unique_ptr<Graph> graph_;
	std::unique_ptr<Router> router_;

	// RouteInfo route_info_;

	using RouteVerticesInfo = std::map<std::string_view, std::vector<VertexInfo*>>;

	std::map<VertexId, VertexInfo> vertices_info_;
	std::unordered_map<std::string_view, StopRouterInfo> stops_routerinfo_;
	std::unordered_map<EdgeId, EDGE_TYPE> edges_type_;

	size_t	getVerticesCount() const;

	void fillGraphWithEdges(const RouteVerticesInfo route_vertices);

	RouteVerticesInfo assignVertices();

	void addWaitEdges();

	void processSingleRoute(const std::vector<VertexInfo*>& vertex_vector, bool is_cycled);

	void processPairOfStops(const VertexInfo* from, const VertexInfo* to);

	double calculateWeight(const info::Stop& from, const info::Stop& to) const;

	std::optional<std::string_view> findBusIntersection(const info::Stop& from, const info::Stop& to) const ;

	VertexId getStopWaitVertex(std::string_view stop_name) const;

	RouteInfo translateRouteInfo(const Router::RouteInfo route_info) const ;

	std::set<VertexId> getPossibleToIds(std::string_view stop_to) const ;
};

} //namespace router

}//namespace transport_guide