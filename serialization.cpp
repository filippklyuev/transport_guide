#include "serialization.h"

namespace transport_guide {


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
	for (int i = 0; i < stop_map.size(); i++){
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
	for (int i = 0; i < bus_map.size(); i++){
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
	for (int i = 0; i < render_settings_.color_palette.size(); i++){
		settings->add_color_palette();
		*settings->mutable_color_palette(i) = getProtoColor(render_settings_.color_palette[i]);
	}
}

void Serializer::updateProtoWithRouter(){
	router::TransportRouter router(catalogue_, routing_settings_);
	catalogue_proto::TransportRouter* pr_router = proto_catalogue.mutable_router();
	pr_router->set_wait_weight(router.getWaitWeight());
	pr_router->set_bus_velocity(router.getBusVelocity());
	updateProtoRouterWithVerticesInfo(router.getVerticesInfo(), pr_router); // write function
	updateProtoRouterWithEdgesInfo(router.getEdgesInfo(), pr_router); //
	fillProtoGraph(router, pr_router->mutable_graph()); // write function
}

void Serializer::updateProtoRouterWithVerticesInfo(const std::vector<router::VertexInfo>& vertices_info,
						catalogue_proto::TransportRouter* pr_router) const {
	int i = 0;
	for (const router::VertexInfo& vertex_info : vertices_info){
		pr_router->add_vertices_info();
		catalogue_proto::VertexInfo *pr_vert_info = pr_router->mutable_vertices_info(i);
		pr_vert_info->set_stop_array_index(vertex_info.stop_info->id_);
		i++;
	}
}

void Serializer::updateProtoRouterWithEdgesInfo(const std::unordered_map<router::EdgeId, router::EdgeInfo>& edges_info,
								catalogue_proto::TransportRouter* pr_router) const {
	for (int i = 0; i < edges_info.size(); i++){ // reserve 
		pr_router->add_edges_info();
	}
	for (const auto& [edge_id, edge_info] : edges_info){
		catalogue_proto::EdgeInfo* pr_edge_info = pr_router->mutable_edges_info(static_cast<int>(edge_id));
		pr_edge_info->set_bus_array_index(catalogue_.GetBusInfo(edge_info.bus_name).id_);
		pr_edge_info->set_span(edge_info.span);
		pr_edge_info->set_weight(edge_info.weight);
		pr_edge_info->set_from_stop_index(catalogue_.GetStopInfo(edge_info.from_stop).id_);
		pr_edge_info->set_to_stop_index(catalogue_.GetStopInfo(edge_info.to_stop).id_);
	}
}

void Serializer::fillProtoGraph(const router::TransportRouter& router, graph_proto::Graph* graph) const {
	for (int i = 0; i < router.getGraph().GetEdgeCount(); i++){
		const graph::Edge<double>& edge = router.getGraph().GetEdge(i);
		graph->add_edge();
		graph_proto::Edge* pr_edge = graph->mutable_edge(i);
		pr_edge->set_vertex_from(static_cast<int>(edge.from));
		pr_edge->set_vertex_to(static_cast<int>(edge.to));
		pr_edge->set_weight(edge.weight);
	}
	const auto& routes_internal_data_matrix = router.getRouter().getRoutesInternalData();
	for (int i = 0; i < routes_internal_data_matrix.size(); i++){
		graph->add_routes_internal_data_matrix();
		graph_proto::RoutesInternalData* pr_routes_internal_data = graph->mutable_routes_internal_data_matrix(i);
		const auto& routes_internal_data = routes_internal_data_matrix[i];
		for (int j = 0; j < routes_internal_data.size(); j++){
			pr_routes_internal_data->add_route_internal_data();
			graph_proto::RouteInternalData* pr_route_internal_data = pr_routes_internal_data->mutable_route_internal_data(j);
			if (routes_internal_data[j].has_value()){
				pr_route_internal_data->set_exists(true);
				pr_route_internal_data->set_weight(routes_internal_data[j]->weight);
				if (routes_internal_data[j]->prev_edge.has_value()){
					pr_route_internal_data->set_prev_exists(true);
					pr_route_internal_data->set_prev_edge_id(static_cast<int>(*routes_internal_data[j]->prev_edge));
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

} // namespace transport_guide