#pragma once
#include <transport_catalogue.pb.h>
#include <fstream>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <iostream>
#include <optional>
#include "transport_catalogue.h"
#include "domain.h"
#include "json.h"
#include "json_builder.h"
#include "geo.h"

namespace transport_guide {

void updateProtoWithStops(const TransportCatalogue::StopMap& stop_map, catalogue_proto::TransportCatalogue& proto_catalogue);

void updateProtoWithBuses(const TransportCatalogue::BusMap& bus_map, catalogue_proto::TransportCatalogue& proto_catalogue);

void SerializeTransportCatalogue(const std::filesystem::path filename,
									const TransportCatalogue& catalogue, const map_renderer::RenderSettings& render_settings);

catalogue_proto::Color getProtoColor(const svg::Color& color);

catalogue_proto::Point getProtoPoint(svg::Point point);

void updateProtoWithRenderSettings(const map_renderer::RenderSettings& render_settings
	 							, catalogue_proto::TransportCatalogue& proto_catalogue);

catalogue_proto::TransportCatalogue createProtoCatalogue(const TransportCatalogue& catalogue
														, const map_renderer::RenderSettings& render_settings);

json::Document DeserializeAndProcessOutputRequests(const std::filesystem::path filename, const json::Dict& output_requests);

catalogue_proto::TransportCatalogue DeserializeCatalogue(const std::filesystem::path filename);

// class ProtoStatParser {
// public:	
// 	ProtoStatParser(const catalogue_proto::TransportCatalogue& catalogue)
// 	: proto_catalogue_(catalogue)
// 	{}

// 	json::Document parseStatArray(const json::Array& requests_vector);
// private:
// 	const catalogue_proto::TransportCatalogue& proto_catalogue_;
// };

// TransportCatalogue DeserializeTransportCatalogue(const std::filesystem::path& filename);

// void parseStopsFromProto(const catalogue_proto::TransportCatalogue& proto_catalogue, TransportCatalogue& catalogue);

// void parseBusesFromProto(const catalogue_proto::TransportCatalogue& proto_catalogue, TransportCatalogue& catalogue);

// std::vector<std::string_view> getStopsVector(const catalogue_proto::Bus& bus,const catalogue_proto::TransportCatalogue& proto_catalogue);

} // namespace transport_guide