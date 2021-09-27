#include "json_reader.h"

namespace transport_guide {

namespace json_reader {

namespace parser {

map_renderer::RenderSettings parseRenderSettings(const json::Dict& render_settings){
    map_renderer::RenderSettings settings;

    settings.width = render_settings.at("width").AsDouble();
    settings.height = render_settings.at("height").AsDouble();

    settings.padding = render_settings.at("padding").AsDouble();
    settings.line_width = render_settings.at("line_width").AsDouble();
    settings.stop_radius = render_settings.at("stop_radius").AsDouble();

    settings.bus_label_font_size = render_settings.at("bus_label_font_size").AsInt();
    json::Array offset_array = render_settings.at("bus_label_offset").AsArray();
    settings.bus_label_offset = svg::Point(offset_array[0].AsDouble(), offset_array[1].AsDouble());

    settings.stop_label_font_size = render_settings.at("stop_label_font_size").AsInt();
    offset_array = render_settings.at("stop_label_offset").AsArray();
    settings.stop_label_offset = svg::Point(offset_array[0].AsDouble(), offset_array[1].AsDouble());

    if (render_settings.at("underlayer_color").IsString()){
        settings.underlayer_color = (render_settings.at("underlayer_color").AsString());
    } else {
        json::Array color_array = render_settings.at("underlayer_color").AsArray();
        if (color_array.size() == 3){
            settings.underlayer_color = (svg::Rgb(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt()));
        } else {
            settings.underlayer_color = (svg::Rgba(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt(), color_array[3].AsDouble()));
        }
    }

    settings.underlayer_width = render_settings.at("underlayer_width").AsDouble();

    const json::Array& palette_colors = render_settings.at("color_palette").AsArray();
    for (const auto& color_node : palette_colors){
        if (color_node.IsString()){
            settings.color_palette.push_back(color_node.AsString());
        } else {
                const json::Array& color_array = color_node.AsArray();
            if (color_array.size() == 3){ // RGB format
                settings.color_palette.push_back(svg::Rgb(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt()));
            } else { // RGBA format
                settings.color_palette.push_back(svg::Rgba(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt(), color_array[3].AsDouble()));
            }
        }
    }
    return settings;
}

input::ParsedStopQuery parseStopRequest(const json::Dict& stop_request){
    input::ParsedStopQuery result;
    for (const auto& [key, value] : stop_request){
        if (key == "name"){
            result.name = value.AsString();
        } else if (key == "latitude"){
            result.coordinates.lat = value.AsDouble();
        } else if (key == "longitude"){
            result.coordinates.lng = value.AsDouble();
        } else if (key == "road_distances"){
            result.distance_to_stops = detail::GetDistanceToStops(value.AsDict());
        }
    }
    return result;
}

input::ParsedBusQuery parseBusRequest(const json::Dict& bus_request){
    input::ParsedBusQuery result;
    for (const auto& [key, value] : bus_request){
        if (key == "name"){
            result.name = value.AsString();
        } else if (key == "stops"){
            result.stops_on_route = detail::parseStopsArray(value.AsArray());
        } else if (key == "is_roundtrip"){
            result.is_cycled = value.AsBool();
        }
    }
    return result;
}

info::RoutingSettings parseRoutingSettings(const json::Dict& routing_settings){
    info::RoutingSettings result;
    result.bus_wait_time = routing_settings.at("bus_wait_time").AsInt();
    result.bus_velocity = routing_settings.at("bus_velocity").AsDouble();
    return result;
}

void updateCatalogue(const json::Array& requests_vector, TransportCatalogue& catalogue){
    std::vector<int> bus_query_positions;
    // info::RoutingSettings routing_settings = parseRoutingSettings(routing_settings_);
    for (size_t i = 0; i < requests_vector.size(); i++){
        const json::Dict& input_request = requests_vector[i].AsDict();
        if (input_request.at("type").AsString() == "Bus"){
            bus_query_positions.push_back(i);
        } else if (input_request.at("type").AsString() == "Stop"){
            auto [name_temp, coordinates, distance_to_stops_temp] = parseStopRequest(input_request);
            catalogue.AddStop(name_temp, coordinates, std::move(distance_to_stops_temp));
        }
    }
    for (size_t i = 0; i < bus_query_positions.size(); i++){
        auto [bus_name_temp, is_cycled, stops_on_route] = parseBusRequest(requests_vector[bus_query_positions[i]].AsDict());
            catalogue.AddRoute(bus_name_temp, is_cycled, std::move(stops_on_route));
    }
}

void StatParser::parseSingleStatRequest(const json::Dict& request, json::Builder& builder){
    builder.StartDict();
    for (const auto& [key, value] : request){
        if (key == "id"){
            builder.Key("request_id").Value(value.AsInt());
        } else if (key == "type"){
            if (value.AsString() == "Stop"){
                std::string_view stop_name = request.at("name").AsString();
                if (catalogue_.IsStopListed(stop_name)){
                    updateResultWithStopInfo(builder, catalogue_.GetStopInfo(stop_name));
                    return;
                } else {
                    break;
                }
            } else if (value.AsString() == "Bus"){
                std::string_view bus_name = request.at("name").AsString();
                if (catalogue_.IsBusListed(bus_name)){
                    updateResultWithBusInfo(builder, catalogue_.GetBusInfo(bus_name));
                    return;
                } else {
                    break;
                }
            } else if (value.AsString() == "Map"){
                updateResultWithMap(builder);
                return ;
            } else if (value.AsString() == "Route"){
                updateResultWithRoute(builder, request.at("from").AsString(), request.at("to").AsString());
                return ;
            }
        }
    }
    builder.Key("error_message").Value(json::Node(static_cast<std::string>("not found")));
    return ;
}

void StatParser::updateResultWithRoute(json::Builder& builder, const std::string& from, const std::string& to){
    if (!router_manager_){
        router_manager_ = std::make_unique<router::TransportRouter>(catalogue_, routing_settings_);
    }
    std::optional<router::TransportRouter::RouteInfo> result = router_manager_->GetRouteInfo(from, to);
    if (!result.has_value()){
        // std::cout << "FAIL\n";
        builder.Key("error_message").Value(json::Node(static_cast<std::string>("not found")));
        return ;
    }
    const auto& route_elems = result->route_elems;
    const double time = result->overall_time;
    // StartDict()
            builder.Key("items")
                .StartArray();
                    for (const auto& elem : route_elems){
                        builder.StartDict();
                        if (elem.type == router::EDGE_TYPE::WAIT){
                            builder.Key("stop_name").Value(json::Node(static_cast<std::string>(elem.name)))
                            .Key("time").Value(json::Node(static_cast<double>(elem.time)))
                            .Key("type").Value(json::Node(static_cast<std::string>("Wait")));
                        } else {
                            builder.Key("bus").Value(json::Node(static_cast<std::string>(elem.name)))
                            .Key("span_count").Value(json::Node(static_cast<int>(elem.span)))
                            .Key("time").Value(json::Node(static_cast<double>(elem.time)))
                            .Key("type").Value(json::Node(static_cast<std::string>("Bus")));
                        }
                        builder.EndDict();
                    }
                builder.EndArray()
            .Key("total_time").Value(json::Node(static_cast<double>(time)));
}

// void StatParser::processRoute(const info::Bus& bus_info){
//     graph::Edge<double> edge;
//     for (int i = 0; i < bus_info.stops.size() - 1; i++){
        
//     }
// }

json::Document StatParser::parseStatArray(const json::Array& requests_vector){
    json::Builder builder;
    builder.StartArray();
        for (const auto& request : requests_vector){
            parseSingleStatRequest(request.AsDict(), builder);
            builder.EndDict();
        }
        builder.EndArray();
    return json::Document(builder.Build());
}

void StatParser::updateResultWithBusInfo(json::Builder& builder, const info::Bus& bus_info){
    builder.Key("curvature").Value(json::Node(static_cast<double>(bus_info.curvature)))
            .Key("route_length").Value(json::Node(static_cast<int>(bus_info.factial_route_length)))
            .Key("stop_count").Value(json::Node(static_cast<int>(bus_info.getStopsCount())))
            .Key("unique_stop_count").Value(json::Node(static_cast<int>(bus_info.getUniqueStopsCount())));
}

void StatParser::updateResultWithStopInfo(json::Builder& builder, const info::Stop& stop_info){
    builder.Key("buses").StartArray();
    for (const auto& string_node : request_handler::getPassingBuses(stop_info)){
        builder.Value(string_node);
    }
    builder.EndArray();
}

void StatParser::updateResultWithMap(json::Builder& builder){
    std::stringstream strm;
    map_renderer::MapRenderer renderer(catalogue_, settings_);
    svg::Document doc = renderer.GetSvgDocument();
    doc.Render(strm);
    builder.Key("map").Value(json::Node(static_cast<std::string>(std::move(strm.str()))));
}
namespace detail {

DistanceMap GetDistanceToStops(const json::Dict& distance_to_stops){
    std::unordered_map<std::string_view, int> result;
    for (auto& [stop, distance] : distance_to_stops){
        result.emplace(stop, distance.AsInt());
    }
    return result;
}

std::vector<std::string_view> parseStopsArray(const json::Array& stops){
    std::vector<std::string_view> result;
    for (const auto& stop : stops){
        result.push_back(stop.AsString());
    }
    return result;
}

} // namespace detail

} // namespace parser

} // namespace json_reader

} // namespace transport_guide