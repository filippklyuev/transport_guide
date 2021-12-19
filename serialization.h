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

//Хотел сделать класс наследуемым от StatParser для избежания дублирований, но как понял суть задания в том,
// чтобы это был независимый модуль
class ProtoStatParser {
public:
	ProtoStatParser(const catalogue_proto::TransportCatalogue& proto_catalogue)
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

svg::Color getSvgColorOfProto(const catalogue_proto::Color& proto_color);

svg::Point getSvgPointOfProto(const catalogue_proto::Point& point_proto);

class ProtoMapRenderer {
public:	
    ProtoMapRenderer(const catalogue_proto::TransportCatalogue& proto_catalogue
    		, const std::unordered_map<std::string_view, int>& proto_stops_map
    		, const std::unordered_map<std::string_view, int>& proto_buses_map)
    	: proto_catalogue_(proto_catalogue)
        , proto_stops_map_(proto_stops_map)
        , proto_buses_map_(proto_stops_map_)
    {
        settings_.width = proto_catalogue_->render_settings().width();
        settings_.height = proto_catalogue_->render_settings().height();
        settings_.padding = proto_catalogue_->render_settings().padding();
        settings_.line_width = proto_catalogue_->render_settings().line_width();
        settings_.stop_radius = proto_catalogue_->render_settings().stop_radius();
        settings_.bus_label_font_size = proto_catalogue_->render_settings().bus_label_font_size();
        settings_.bus_label_offset = getSvgPointOfProto(proto_catalogue_->render_settings().bus_label_offset());
        settings_.stop_label_font_size = proto_catalogue_->render_settings().stop_label_font_size();
        settings_.stop_label_offset = getSvgPointOfProto(proto_catalogue_->render_settings().stop_label_offset());
        settings_.underlayer_color = getSvgColorOfProto(proto_catalogue_->render_settings().underlayer_color());
        settings_.underlayer_width = proto_catalogue_->render_settings().underlayer_width();
        for (int i = 0; i < proto_catalogue_->render_settings().color_palette_size(); i++){
            settings_.color_palette.push_back(
                getSvgColorOfProto(proto_catalogue_->render_settings().color_palette(i))
            );
        // InitilizeCatalogueMap(proto_catalogue_->bus(), bus_index_map_);
        // InitilizeCatalogueMap(proto_catalogue_->stop(), stop_index_map_);
        }
    }

    svg::Document GetSvgDocument();

    template<typename IterIt>
    void addObjectsToDoc(IterIt begin, IterIt end, svg::Document& document){
        for (auto it = begin; it != end; it++){
            document.Add(std::move(*it));
        }
    }    

private:
	const catalogue_proto::TransportCatalogue& proto_catalogue_;
    const std::optional<std::map<std::string_view, int>>& bus_index_map_;
    const std::optional<std::map<std::string_view, int>>& stop_index_map_;
    map_renderer::RenderSettings settings_;
    map_renderer::ScalerStruct scaler_;
    std::vector<svg::Polyline> polylines_;
    std::vector<svg::Text> route_names_;
    std::vector<svg::Circle> stop_circles_;
    std::vector<svg::Text> stop_names_;     

    void parseStopCirclesAndNames();

    void parsePolylinesAndRouteNames();

    void makeScalerOfCatalogue();

    void makeScaler();

    void addObjectsToDoc(svg::Document& document);
    
    svg::Text getBusnameUnder(const std::string& bus_name, geo::Coordinates coordinates);
    
    svg::Text getBusnameText(const std::string& bus_name, geo::Coordinates coordinates, int route_counter);    
};

catalogue_proto::Color getProtoColor(const svg::Color& color);

catalogue_proto::Point getProtoPoint(svg::Point point);

json::Document DeserializeAndProcessOutputRequests(const std::filesystem::path filename, const json::Dict& output_requests);

catalogue_proto::TransportCatalogue DeserializeCatalogue(const std::filesystem::path filename);

} // namespace serialize

} // namespace transport_guide