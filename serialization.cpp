#include "serialization.h"

namespace transport_guide {

namespace proto {

void Serializer::SerializeTransportCatalogue(){
	createProtoCatalogue();
	std::ofstream ofs(filename_, std::ios::binary);
	proto_catalogue.SerializeToOstream(&ofs);
}

void Serializer::createProtoCatalogue(){
	updateProtoWithStops(catalogue_.GetStopsMap());
	updateProtoWithBuses(catalogue_.GetBusesMap());
	updateProtoWithRenderSettings();
	updateProtoWithRouter();
}

void Serializer::updateProtoWithStops(const TransportCatalogue::StopMap& stop_map){
	proto_catalogue.mutable_stop()->Reserve(stop_map.size());
	for (size_t i = 0; i < stop_map.size(); i++){
		proto_catalogue.add_stop(); // Бред, но у protobuf::RepeatedPtrField не нашел адекватного способа сделать resize :(
	}
	for (const auto& [name, info] : stop_map){
		proto_catalogue.add_stop();
		catalogue_proto::Stop* stop = proto_catalogue.mutable_stop(info.id_);
		stop->set_name(std::string(name));
		stop->set_lattitude(info.coordinates.lat);
		stop->set_longtitude(info.coordinates.lng);
		for (const transport_guide::info::Bus* bus : info.passing_buses){
			stop->add_bus_index(bus->id_);
		}
		for (const auto& [stop_name, distance] : info.distance_to_stops){
			(*stop->mutable_stopid_distance())[stop_map.at(stop_name).id_] = distance;
		}
	}
}

void Serializer::updateProtoWithBuses(const TransportCatalogue::BusMap& bus_map){
	proto_catalogue.mutable_bus()->Reserve(bus_map.size());
	for (size_t i = 0; i < bus_map.size(); i++){
		proto_catalogue.add_bus();
	}	
	for (const auto& [name, info] : bus_map){
		catalogue_proto::Bus* bus = proto_catalogue.mutable_bus(info.id_);
		bus->set_name(std::string(name));
		bus->set_factual_route_length(info.factial_route_length);
		bus->set_geo_route_length(info.geo_route_length);
		bus->set_curvature(info.curvature);
		bus->set_unique_stops_count(info.unique_stops.size());
		bus->set_is_cycled(info.is_cycled);
		for (const transport_guide::info::Stop* stop : info.stops){
			bus->add_stop_index(stop->id_);
		}
	}
}


void Serializer::updateProtoWithRenderSettings(){
	catalogue_proto::RenderSettings* settings = proto_catalogue.mutable_render_settings();
	settings->set_width(render_settings_.width);
	settings->set_height(render_settings_.height);
	settings->set_padding(render_settings_.padding);
	settings->set_line_width(render_settings_.line_width);
	settings->set_stop_radius(render_settings_.stop_radius);
	settings->set_bus_label_font_size(render_settings_.bus_label_font_size);
	*settings->mutable_bus_label_offset() = getProtoPoint(render_settings_.bus_label_offset);
	settings->set_stop_label_font_size(render_settings_.stop_label_font_size);
	*settings->mutable_stop_label_offset() = getProtoPoint(render_settings_.stop_label_offset);
	*settings->mutable_underlayer_color() = getProtoColor(render_settings_.underlayer_color);
	settings->set_underlayer_width(render_settings_.underlayer_width);
	for (size_t i = 0; i < render_settings_.color_palette.size(); i++){
		settings->add_color_palette();
		*settings->mutable_color_palette(i) = getProtoColor(render_settings_.color_palette[i]);
	}
}

void Serializer::updateProtoWithRouter(){
	router::TransportRouter router(catalogue_, routing_settings_);
	catalogue_proto::TransportRouter* pr_router = proto_catalogue.mutable_router();
	pr_router->set_wait_weight(router.getWaitWeight());
	pr_router->set_bus_velocity(router.getBusVelocity());
	updateProtoRouterWithVerticesInfo(router.getVerticesInfo(), pr_router);
	updateProtoRouterWithEdgesInfo(router.getEdgesInfo(), pr_router); 
	fillProtoGraph(router, pr_router->mutable_graph());
}

void Serializer::updateProtoRouterWithVerticesInfo(const std::vector<router::VertexInfo>& vertices_info,
						catalogue_proto::TransportRouter* pr_router) const {
	for (const router::VertexInfo& vertex_info : vertices_info){
		(*pr_router->mutable_stop_id_vertex_id())
					[vertex_info.stop_info->id_] = vertex_info.id;
	}
}

void Serializer::updateProtoRouterWithEdgesInfo(const std::unordered_map<router::EdgeId, router::EdgeInfo>& edges_info,
								catalogue_proto::TransportRouter* pr_router) const {
	for (size_t i = 0; i < edges_info.size(); i++){
		pr_router->add_edges_info();
	}
	for (const auto& [edge_id, edge_info] : edges_info){
		catalogue_proto::EdgeInfo* pr_edge_info = pr_router->mutable_edges_info(static_cast<size_t>(edge_id));
		pr_edge_info->set_bus_array_index(catalogue_.GetBusInfo(edge_info.bus_name).id_);
		pr_edge_info->set_span(edge_info.span);
		pr_edge_info->set_weight(edge_info.weight);
		pr_edge_info->set_from_stop_index(catalogue_.GetStopInfo(edge_info.from_stop).id_);
		pr_edge_info->set_to_stop_index(catalogue_.GetStopInfo(edge_info.to_stop).id_);
	}
}

void Serializer::fillProtoGraph(const router::TransportRouter& router, graph_proto::Graph* graph) const {
	for (size_t i = 0; i < router.getGraph().GetEdgeCount(); i++){
		const graph::Edge<double>& edge = router.getGraph().GetEdge(i);
		graph->add_edge();
		graph_proto::Edge* pr_edge = graph->mutable_edge(i);
		pr_edge->set_vertex_from(static_cast<size_t>(edge.from));
		pr_edge->set_vertex_to(static_cast<size_t>(edge.to));
		pr_edge->set_weight(edge.weight);
	}
	const auto& routes_internal_data_matrix = router.getRouter().getRoutesInternalData();
	for (size_t i = 0; i < routes_internal_data_matrix.size(); i++){
		graph->add_routes_internal_data_matrix();
		graph_proto::RoutesInternalData* pr_routes_internal_data = graph->mutable_routes_internal_data_matrix(i);
		const auto& routes_internal_data = routes_internal_data_matrix[i];
		for (size_t j = 0; j < routes_internal_data.size(); j++){
			pr_routes_internal_data->add_route_internal_data();
			graph_proto::RouteInternalData* pr_route_internal_data = pr_routes_internal_data->mutable_route_internal_data(j);
			if (routes_internal_data[j].has_value()){
				pr_route_internal_data->set_exists(true);
				pr_route_internal_data->set_weight(routes_internal_data[j]->weight);
				if (routes_internal_data[j]->prev_edge.has_value()){
					pr_route_internal_data->set_prev_exists(true);
					pr_route_internal_data->set_prev_edge_id(static_cast<size_t>(*routes_internal_data[j]->prev_edge));
				} else {
					pr_route_internal_data->set_prev_exists(false);
				}
			} else {
				pr_route_internal_data->set_exists(false);
			}
		}
	}
}

catalogue_proto::Color getProtoColor(const svg::Color& color){
	catalogue_proto::Color proto_color;
	if (std::holds_alternative<svg::Rgb>(color)){
		catalogue_proto::RGB* rgb_proto = proto_color.mutable_rgb();
		const svg::Rgb& rgb = std::get<svg::Rgb>(color);
		rgb_proto->set_red(rgb.red);
		rgb_proto->set_green(rgb.green);
		rgb_proto->set_blue(rgb.blue);
	} else if (std::holds_alternative<svg::Rgba>(color)){
		catalogue_proto::RGBA* rgba_proto = proto_color.mutable_rgba();
		const svg::Rgba& rgba = std::get<svg::Rgba>(color);
		rgba_proto->set_red(rgba.red);
		rgba_proto->set_green(rgba.green);
		rgba_proto->set_blue(rgba.blue);
		rgba_proto->set_opacity(rgba.opacity);
	} else {
		const std::string& str = std::get<std::string>(color);
		proto_color.set_str(str);
	}
	return proto_color;
}

catalogue_proto::Point getProtoPoint(svg::Point point){
	catalogue_proto::Point proto_point;
	proto_point.set_x(point.x);
	proto_point.set_y(point.y);
	return proto_point;
}

catalogue_proto::TransportCatalogue DeserializeCatalogue(const std::filesystem::path filename){
	std::ifstream ifs(filename);
	catalogue_proto::TransportCatalogue proto_catalogue;
	if (!proto_catalogue.ParseFromIstream(&ifs)){
		return {};
	}
	return proto_catalogue;
}

bool StatParser_Deserialized::isValidRequest(const json::Dict& request, QueryType type) const {
    if (type == QueryType::STOP){
        return proto_stops_map_.count(request.at("name").AsString());
    } 
    if (type == QueryType::BUS){
        return proto_buses_map_.count(request.at("name").AsString());
    }
    if (type == QueryType::MAP){
        return true;
    }
    // query type ROUTE will be checked for validity later
    return true;
}

svg::Document StatParser_Deserialized::getSvgDoc() const {
    map_renderer::MapRenderer renderer(&proto_catalogue_);
    return renderer.GetSvgDocument();	
}

void StatParser_Deserialized::HandleError(const json::Dict& request, json::Builder& builder){
        builder.StartDict()
               .Key("request_id").Value(json::Node(static_cast<int>(request.at("id").AsInt())))
               .Key("error_message").Value(json::Node(static_cast<std::string>("not found")))
        .EndDict();    
}

static graph_proto::RouteInfo buildProtoRoute(size_t id_from, size_t id_to
    , const graph_proto::Graph& graph){
    const auto& route_internal_data = graph.routes_internal_data_matrix(id_from).route_internal_data(id_to);
    graph_proto::RouteInfo route_info;
    if (route_internal_data.exists()){
        route_info.set_overall_time(route_internal_data.weight());
        bool prev_exists = route_internal_data.prev_exists();
        if (prev_exists){
            for (size_t edge_id = route_internal_data.prev_edge_id(); prev_exists;
                edge_id = graph.routes_internal_data_matrix(id_from)
                                .route_internal_data(graph.edge(edge_id).vertex_from()).prev_edge_id()){
                route_info.add_edge_index_reversed(edge_id);
                prev_exists = graph.routes_internal_data_matrix(id_from)
                                .route_internal_data(graph.edge(edge_id).vertex_from()).prev_exists();
            }
        }
        route_info.set_exists(true);
        return route_info;
    }
    route_info.set_exists(false);
    return route_info;
}

static std::vector<catalogue_proto::EdgeInfo> getProtoEdgesVector(const graph_proto::RouteInfo& route
                                                                    , const catalogue_proto::TransportRouter& router){
    std::vector<catalogue_proto::EdgeInfo> route_edges;
    for (int i = route.edge_index_reversed_size() - 1; i >= 0; i--){
        route_edges.push_back(router.edges_info(route.edge_index_reversed(i)));
    }
    return route_edges;
}

bool StatParser_Deserialized::isRouteValid(const json::Dict& request) const {
    if (proto_stops_map_.count(request.at("from").AsString()) && proto_stops_map_.count(request.at("to").AsString())){
        int from_id_catalogue = proto_stops_map_.at(request.at("from").AsString());
        int to_id_catalogue = proto_stops_map_.at(request.at("to").AsString());
        if (proto_catalogue_.router().stop_id_vertex_id().contains(from_id_catalogue) && 
                    proto_catalogue_.router().stop_id_vertex_id().contains(to_id_catalogue)){
            return true;
        }
    }
    return false;
}

void StatParser_Deserialized::parseRouteRequest(const json::Dict& request, json::Builder& builder){
    const catalogue_proto::TransportRouter& router = proto_catalogue_.router();
    const graph_proto::Graph& graph = router.graph();
    if (isRouteValid(request)){
        int from_id_catalogue = proto_stops_map_.at(request.at("from").AsString());
        int to_id_catalogue = proto_stops_map_.at(request.at("to").AsString());
        int id_from = proto_catalogue_.router().stop_id_vertex_id().at(proto_stops_map_.at(request.at("from").AsString()));
        int id_to = proto_catalogue_.router().stop_id_vertex_id().at(proto_stops_map_.at(request.at("to").AsString()));
        graph_proto::RouteInfo route = buildProtoRoute(id_from, id_to, graph);
        if (route.exists()){
            std::vector<catalogue_proto::EdgeInfo> route_edges = getProtoEdgesVector(route, router);
            builder.StartDict()
                   .Key("request_id").Value(json::Node(static_cast<int>(request.at("id").AsInt())))
                   .Key("items")
                        .StartArray();
                        for (const auto& edge : route_edges){
                            builder.StartDict()
                                .Key("stop_name").Value(json::Node(static_cast<std::string>(proto_catalogue_.stop(edge.from_stop_index()).name())))
                                .Key("time").Value(json::Node(static_cast<double>(router.wait_weight())))
                                .Key("type").Value(json::Node(static_cast<std::string>("Wait")))
                            .EndDict()
                            .StartDict()
                                .Key("bus").Value(json::Node(static_cast<std::string>(proto_catalogue_.bus(edge.bus_array_index()).name())))
                                .Key("span_count").Value(json::Node(static_cast<int>(edge.span())))
                                .Key("time").Value(json::Node(static_cast<double>(edge.weight() - router.wait_weight())))
                                .Key("type").Value(json::Node(static_cast<std::string>("Bus")))
                            .EndDict();
                        }
                        builder.EndArray()
                    .Key("total_time").Value(json::Node(static_cast<double>(route.overall_time())))
                .EndDict();
            return ;
        }        
    }
    HandleError(request, builder);
}

void StatParser_Deserialized::parseStopRequest(const json::Dict& request, json::Builder& builder) const {
    std::string name = request.at("name").AsString();
    const catalogue_proto::Stop& stop_info = proto_catalogue_.stop(proto_stops_map_.at(name));

    builder.StartDict()
           .Key("request_id").Value(json::Node(static_cast<int>(request.at("id").AsInt())))
           .Key("buses").StartArray();
           for (int i = 0; i < stop_info.bus_index_size(); i++){
                builder.Value(json::Node(static_cast<std::string>(proto_catalogue_.bus(stop_info.bus_index(i)).name())));
           }
           builder.EndArray()
    .EndDict();
}

void StatParser_Deserialized::parseBusRequest(const json::Dict& request, json::Builder& builder) const {
    std::string name = request.at("name").AsString();
    const catalogue_proto::Bus& bus_info = proto_catalogue_.bus(proto_buses_map_.at(name));
    builder.StartDict()
           .Key("request_id").Value(json::Node(static_cast<int>(request.at("id").AsInt())))
           .Key("curvature").Value(json::Node(static_cast<double>(bus_info.curvature())))
           .Key("route_length").Value(json::Node(static_cast<int>(bus_info.factual_route_length())))
           .Key("stop_count").Value(json::Node(static_cast<int>(bus_info.is_cycled()
                                                                        ? bus_info.stop_index_size()
                                                                        : bus_info.stop_index_size() * 2 - 1)))
           .Key("unique_stop_count").Value(json::Node(static_cast<int>(bus_info.unique_stops_count())))
    .EndDict();
}

static std::string SvgToStr(svg::Document doc){
    std::stringstream strm;
    doc.Render(strm);
    return std::move(strm.str());
}

void StatParser_Deserialized::parseMapRequest(const json::Dict& request, json::Builder& builder) const {
    builder.StartDict()
           .Key("request_id").Value(json::Node(static_cast<int>(request.at("id").AsInt())))
           .Key("map").Value(json::Node(static_cast<std::string>(SvgToStr(getSvgDoc()))))
    .EndDict();   
}

void StatParser_Deserialized::initializeProtoNameMaps(){
    for (int i = 0; i < proto_catalogue_.stop_size(); i++){
        proto_stops_map_.emplace(proto_catalogue_.stop(i).name(), i);
    }
    for (int i = 0; i < proto_catalogue_.bus_size(); i++){
        proto_buses_map_.emplace(proto_catalogue_.bus(i).name(), i);
    }
}

void StatParser_Deserialized::parseSingleStatRequest(const json::Dict& request, json::Builder& builder){
    QueryType request_type = defineRequestType(request.at("type").AsString());
    if (!isValidRequest(request, request_type)){
        HandleError(request, builder);
        return;   
    }
    switch (request_type){
        case QueryType::STOP : {     
            parseStopRequest(request, builder);
            break;
        }
        case QueryType::BUS : {    
            parseBusRequest(request, builder);
            break;
        }
        case QueryType::MAP : {  
            parseMapRequest(request, builder);
            break;
        }
        case QueryType::ROUTE : {    
            parseRouteRequest(request, builder);
            break;
        }
    }
}

json::Document StatParser_Deserialized::parseStatArray(const json::Array& requests_vector) { 
    json::Builder builder;
    builder.StartArray();
        for (const auto& request : requests_vector){
            parseSingleStatRequest(request.AsDict(), builder);
        }
        builder.EndArray();
    return json::Document(builder.Build());
}

} // namespace proto

} // namespace transport_guide