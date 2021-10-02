#include "json_reader.h"

namespace transport_guide {

namespace json_reader {

namespace parser {

static void ParseAndInsertColor(svg::Color& empty_color, const json::Node& color_node){
    if (color_node.IsString()){
        empty_color = (color_node.AsString());
    } else {
        json::Array color_array = color_node.AsArray();
        if (color_array.size() == 3){
            empty_color = (svg::Rgb(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt()));
        } else {
            empty_color = (svg::Rgba(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt(), color_array[3].AsDouble()));
        }
    }    
}     

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

    const auto& color_node = render_settings.at("underlayer_color");
    ParseAndInsertColor(settings.underlayer_color, color_node);

    settings.underlayer_width = render_settings.at("underlayer_width").AsDouble();

    const json::Array& palette_colors = render_settings.at("color_palette").AsArray();
    settings.color_palette.resize(palette_colors.size());
    for (size_t i = 0; i < palette_colors.size(); i++){
        ParseAndInsertColor(settings.color_palette[i], palette_colors[i]);
    }
    return settings;
}

static DistanceMap GetDistanceToStops(const json::Dict& distance_to_stops){ 
    DistanceMap result;
    for (auto& [stop, distance] : distance_to_stops){
        result.emplace(stop, distance.AsInt());
    }
    return result;
}

ParsedStopQuery parseStopRequest(const json::Dict& stop_request){ //NEW
    ParsedStopQuery result;
    result.name = stop_request.at("name").AsString();
    result.coordinates.lat = stop_request.at("latitude").AsDouble();
    result.coordinates.lng = stop_request.at("longitude").AsDouble();
    result.distance_to_stops = GetDistanceToStops(stop_request.at("road_distances").AsDict());
    return result;
}

static std::vector<std::string_view> parseStopsArray(const json::Array& stops){ 
    std::vector<std::string_view> result;
    for (const auto& stop : stops){
        result.push_back(stop.AsString());
    }
    return result;
}

ParsedBusQuery parseBusRequest(const json::Dict& bus_request){ 
    ParsedBusQuery result;
    result.name = bus_request.at("name").AsString();
    result.stops_on_route = parseStopsArray(bus_request.at("stops").AsArray());
    result.is_cycled = bus_request.at("is_roundtrip").AsBool();
    return result;
}

void updateCatalogue(const json::Array& requests_vector, transport_guide::TransportCatalogue& catalogue){
    std::vector<int> bus_query_positions;   
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

bool StatParser::isValidRequest(const json::Dict& request, QueryType type){
    if (type == QueryType::STOP || type == QueryType::BUS){
        std::string_view name = request.at("name").AsString();
        if (type == QueryType::STOP){
            if (!catalogue_.IsStopListed(name)){
                return false;
            }
        } else {
            if (!catalogue_.IsBusListed(name)){
                return false;
            }
        }
    }
    // If request_type is MAP request is always valid
    return true;
}

 QueryType StatParser::defineRequestType(std::string_view type){
    if (type == "Stop"){
        return QueryType::STOP;
    } else if (type == "Bus"){
        return QueryType::STOP;
    } else {
        return QueryType::MAP;
    }
}

json::Node StatParser::getMapAsNode(){
    std::stringstream strm;
    map_renderer::MapRenderer renderer(catalogue_, settings_);
    svg::Document doc = renderer.GetSvgDocument();
    doc.Render(strm);
    return json::Node(static_cast<std::string>(strm.str()));
}

void StatParser::parseSingleStatRequest(const json::Dict& request, json::Builder& builder){
    QueryType request_type = defineRequestType(request.at("type").AsString());
    if (isValidRequest(request, request_type)){
        if (request_type == QueryType::STOP || request_type == QueryType::BUS){
            std::string_view name = request.at("name").AsString();
            if (request_type == QueryType::STOP){
                const info::Stop& stop_info = catalogue_.GetStopInfo(name);
                builder.StartDict()
                    .Key("buses").StartArray();
                        for (const auto& bus_string_node : request_handler::getPassingBuses(stop_info)){
                             builder.Value(bus_string_node);
                        }
                        builder.EndArray()
                .EndDict();
            } else {
                const info::Bus& bus_info = catalogue_.GetBusInfo(name);
                builder.StartDict()
                    .Key("curvature").Value(json::Node(static_cast<double>(bus_info.curvature)))
                    .Key("route_length").Value(json::Node(static_cast<int>(bus_info.factial_route_length)))
                    .Key("stop_count").Value(json::Node(static_cast<int>(bus_info.getStopsCount())))
                    .Key("unique_stop_count").Value(json::Node(static_cast<int>(bus_info.getUniqueStopsCount())))
                .EndDict();
            }
        } else { 
            builder.StartDict()
                .Key("map").Value(getMapAsNode())
            .EndDict();
        }
    } else {
        builder.StartDict()
            .Key("error_message").Value(json::Node(static_cast<std::string>("not found")))
        .EndDict();        
    }
}

json::Document StatParser::parseStatArray(const json::Array& requests_vector){
    json::Builder builder;
    builder.StartArray();
        for (const auto& request : requests_vector){
            parseSingleStatRequest(request.AsDict(), builder);
        }
        builder.EndArray();
    return json::Document(builder.Build());
}

} // namespace parser

} // namespace json_reader

} // namespace transport_guide