#pragma once

#include <transport_catalogue.pb.h>
#include <fstream>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <iostream>
#include <optional>
#include <unordered_map>

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "domain.h"
#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "geo.h"
#include "json_reader.h"

namespace transport_guide {

namespace proto {

class Serializer {
public:
	Serializer(const std::filesystem::path filename
				, const TransportCatalogue& catalogue,const map_renderer::RenderSettings& render_settings
				, RoutingSettings routing_settings)
		: filename_(filename)
		, catalogue_(catalogue)
		, render_settings_(render_settings)
		, routing_settings_(routing_settings)
	{}

	void SerializeTransportCatalogue();

private:
	const std::filesystem::path filename_;
	const TransportCatalogue& catalogue_;
	const map_renderer::RenderSettings& render_settings_;
	RoutingSettings routing_settings_;
	catalogue_proto::TransportCatalogue proto_catalogue;


	void createProtoCatalogue();

	void updateProtoWithStops(const TransportCatalogue::StopMap& stop_map);

	void updateProtoWithBuses(const TransportCatalogue::BusMap& bus_map);	

	void updateProtoWithRenderSettings();

	void updateProtoRouterWithVerticesInfo(const std::vector<router::VertexInfo>& vertices_info,
								catalogue_proto::TransportRouter* pr_router) const;

	void updateProtoRouterWithEdgesInfo(const std::unordered_map<router::EdgeId, router::EdgeInfo>& edges_info,
								catalogue_proto::TransportRouter* pr_router) const;

	void updateProtoWithRouter();

	void fillProtoGraph(const router::TransportRouter& router, graph_proto::Graph* graph) const;
};

class StatParser_Deserialized {
public:
	StatParser_Deserialized(const catalogue_proto::TransportCatalogue& proto_catalogue)
		: proto_catalogue_(proto_catalogue)
	{
		initializeProtoNameMaps();
	}

	json::Document parseStatArray(const json::Array& requests_vector);	

private:
	const catalogue_proto::TransportCatalogue& proto_catalogue_;
    std::unordered_map<std::string_view, int> proto_stops_map_;
    std::unordered_map<std::string_view, int> proto_buses_map_;

	void parseSingleStatRequest(const json::Dict& request, json::Builder& builder);

	bool isValidRequest(const json::Dict& request, QueryType type) const;

	void parseStopRequest(const json::Dict& request, json::Builder& builder) const;

	void parseRouteRequest(const json::Dict& request, json::Builder& builder);

	void parseBusRequest(const json::Dict& request, json::Builder& builder) const;

	void parseMapRequest(const json::Dict& request, json::Builder& builder) const;

	bool isRouteValid(const json::Dict& request) const;

	void initializeProtoNameMaps();

	svg::Document getSvgDoc() const;

	void HandleError(const json::Dict& request, json::Builder& builder);
};

catalogue_proto::Color getProtoColor(const svg::Color& color);

catalogue_proto::Point getProtoPoint(svg::Point point);

json::Document DeserializeAndProcessOutputRequests(const std::filesystem::path filename, const json::Dict& output_requests);

catalogue_proto::TransportCatalogue DeserializeCatalogue(const std::filesystem::path filename);

} // namespace serialize

} // namespace transport_guide