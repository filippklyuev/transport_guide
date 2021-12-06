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
	// updateProtoWithRouter(TransportRouter(catalogue_, render_settings_));
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
	std::cerr << proto_catalogue.stop(0).stopid_distance_size() << '\n';
	return proto_catalogue;
}

} // namespace transport_guide