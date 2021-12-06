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
#include "transport_catalogue.h"
#include "transport_router.h"
#include "domain.h"
#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "geo.h"

namespace transport_guide {

class Serializer {
public:
	Serializer(const std::filesystem::path filename
				, const TransportCatalogue& catalogue,const map_renderer::RenderSettings& render_settings
				, const RoutingSettings& routing_settings)
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
	const RoutingSettings routing_settings_;
	catalogue_proto::TransportCatalogue proto_catalogue;


	void createProtoCatalogue();

	void updateProtoWithStops(const TransportCatalogue::StopMap& stop_map);

	void updateProtoWithBuses(const TransportCatalogue::BusMap& bus_map);	

	void updateProtoWithRenderSettings();	
};

catalogue_proto::Color getProtoColor(const svg::Color& color);

catalogue_proto::Point getProtoPoint(svg::Point point);

json::Document DeserializeAndProcessOutputRequests(const std::filesystem::path filename, const json::Dict& output_requests);

catalogue_proto::TransportCatalogue DeserializeCatalogue(const std::filesystem::path filename);

} // namespace transport_guide