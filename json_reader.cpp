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
            result.distance_to_stops = detail::GetDistanceToStops(value.AsMap());
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

void updateCatalogue(const json::Array& requests_vector, TransportCatalogue& catalogue){
    std::vector<int> bus_query_positions;   
    for (int i = 0; i < requests_vector.size(); i++){
        const json::Dict& input_request = requests_vector[i].AsMap();
        if (input_request.at("type").AsString() == "Bus"){
            bus_query_positions.push_back(i);
        } else if (input_request.at("type").AsString() == "Stop"){
            auto [name_temp, coordinates, distance_to_stops_temp] = parseStopRequest(input_request);
            catalogue.AddStop(name_temp, coordinates, std::move(distance_to_stops_temp));
        }
    }
    for (int i = 0; i < bus_query_positions.size(); i++){
        auto [bus_name_temp, is_cycled, stops_on_route] = parseBusRequest(requests_vector[bus_query_positions[i]].AsMap());
            catalogue.AddRoute(bus_name_temp, is_cycled, std::move(stops_on_route));
    }
}

json::Dict StatParser::parseSingleStatRequest(const json::Dict& request){
    json::Dict result;
    for (const auto& [key, value] : request){
        if (key == "id"){
            result.emplace(std::make_pair("request_id", json::Node(value.AsInt())));
        } else if (key == "type"){
            if (value.AsString() == "Stop"){
                std::string_view stop_name = request.at("name").AsString();
                if (catalogue_.IsStopListed(stop_name)){
                    updateResultWithStopInfo(result, catalogue_.GetStopInfo(stop_name));
                    return result;
                } else {
                    break;
                }
            } else if (value.AsString() == "Bus"){
                std::string_view bus_name = request.at("name").AsString();
                if (catalogue_.IsBusListed(bus_name)){
                    updateResultWithBusInfo(result, catalogue_.GetBusInfo(bus_name));
                    return result;
                } else {
                    break;
                }
            } else if (value.AsString() == "Map"){
                updateResultWithMap(result);
                return result;
            }
        }
    }
    result.emplace(std::make_pair("error_message", json::Node(static_cast<std::string>("not found"))));
    return result;
}

json::Array StatParser::parseStatArray(const json::Array& requests_vector){
    json::Array result;
    for (const auto& request : requests_vector){
        result.push_back(parseSingleStatRequest(request.AsMap()));
    }
    return result;
}

void StatParser::updateResultWithBusInfo(json::Dict& result, const info::Bus& bus_info){
    result.emplace(std::make_pair("curvature", json::Node(static_cast<double>(bus_info.curvature))));
    result.emplace(std::make_pair("route_length", json::Node(static_cast<int>(bus_info.factial_route_length))));
    result.emplace(std::make_pair("stop_count", json::Node(static_cast<int>(bus_info.getStopsCount()))));
    result.emplace(std::make_pair("unique_stop_count", json::Node(static_cast<int>(bus_info.getUniqueStopsCount()))));
}

void StatParser::updateResultWithStopInfo(json::Dict& result, const info::Stop& stop_info){
    result.emplace(std::make_pair("buses", json::Node(request_handler::getPassingBuses(stop_info))));
}

void StatParser::updateResultWithMap(json::Dict& result){
    std::stringstream strm;
    map_renderer::MapRenderer renderer(catalogue_, settings_);
    svg::Document doc = renderer.GetSvgDocument();
    doc.Render(strm);
    result.emplace(std::make_pair("map", json::Node(std::move(strm.str()))));    
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