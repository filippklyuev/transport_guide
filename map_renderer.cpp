#include "map_renderer.h"

namespace map_renderer {

svg::Document MapRenderer::GetSvgDocument(){
    svg::Document document;
    makeScaler();
    parsePolylinesAndRouteNames();
    parseStopCirclesAndNames();
    addObjectsToDoc(polylines_.begin(), polylines_.end(), document);
    addObjectsToDoc(route_names_.begin(), route_names_.end(), document);
    addObjectsToDoc(stop_circles_.begin(), stop_circles_.end(), document);
    addObjectsToDoc(stop_names_.begin(), stop_names_.end(), document);
    return document;
}

void MapRenderer::makeScaler(){
    bool begin = true;
    for (const auto& [bus_name, info] : catalogue_.GetBusesMap()){
        for (const auto& stop : info.stops){
            const auto& stop_info = catalogue_.GetStopInfo(stop->getName());
            if (begin){

                scaler_.min_lat = stop_info.coordinates.lat;
                scaler_.max_lat = stop_info.coordinates.lat;
                scaler_.min_lon  = stop_info.coordinates.lng;
                scaler_.max_lon = stop_info.coordinates.lng;
                begin = false;
            }
            auto& crds = stop_info.coordinates;
            if (crds.lat < scaler_.min_lat) { scaler_.min_lat = crds.lat; }
                else if (crds.lat > scaler_.max_lat) { scaler_.max_lat = crds.lat; }
            if (crds.lng < scaler_.min_lon) { scaler_.min_lon = crds.lng; }
                else if (crds.lng > scaler_.max_lon) { scaler_.max_lon = crds.lng; }
        }
    }
    double width_zoom_coef = 0.0; 
    double height_zoom_coef = 0.0;
    if ((scaler_.max_lon - scaler_.min_lon) != 0.0){
        width_zoom_coef = (settings_.width - 2 * settings_.padding) / (scaler_.max_lon - scaler_.min_lon);
    }
    if ((scaler_.max_lat - scaler_.min_lat) != 0.0){
        height_zoom_coef = (settings_.height - 2 * settings_.padding) / (scaler_.max_lat - scaler_.min_lat);
    }
    if (width_zoom_coef < height_zoom_coef){
        scaler_.zoom_coef = width_zoom_coef;
    } else {
        scaler_.zoom_coef = height_zoom_coef;
    }
    scaler_.padding = settings_.padding;        
}

svg::Text MapRenderer::getBusnameUnder(const std::string& bus_name, geo::Coordinates coordinates){
    svg::Text busname_under;

    busname_under.SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color).
            SetPosition(GetSvgPoint(coordinates)).SetOffset(settings_.bus_label_offset).SetFontSize(settings_.bus_label_font_size).
            SetFontFamily("Verdana").SetFontWeight("bold").SetData(bus_name).SetStrokeWidth(settings_.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).
            SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    return busname_under;
}

svg::Text MapRenderer::getBusnameText(const std::string& bus_name, geo::Coordinates coordinates, int route_counter){
    svg::Text busname_text;

    busname_text.SetFillColor(settings_.color_palette[route_counter % settings_.color_palette.size()]).
        SetPosition(GetSvgPoint(coordinates)).
        SetOffset(settings_.bus_label_offset).SetFontSize(settings_.bus_label_font_size).
        SetFontFamily("Verdana").SetFontWeight("bold").SetData(bus_name);
    return busname_text;
}

void MapRenderer::parsePolylinesAndRouteNames(){
    polylines_.reserve(catalogue_.GetBusesSet().size());
    route_names_.reserve(catalogue_.GetBusesSet().size());
    int route_counter = 0;
    for (const auto& bus_name : catalogue_.GetBusesSet()){
        const auto& info = catalogue_.GetBusInfo(bus_name);
        if (info.stops.size() == 0){
            continue ;
        }
        svg::Polyline route;

        route_names_.push_back(getBusnameUnder(bus_name, info.stops[0]->coordinates));
        route_names_.push_back(getBusnameText(bus_name, info.stops[0]->coordinates, route_counter));

        for (const transport_guide::info::Stop* stop : info.stops){
            route.AddPoint(GetSvgPoint(stop->coordinates));
        }
        if (info.stops.back()->coordinates != info.stops.front()->coordinates){
            route_names_.push_back(getBusnameUnder(bus_name, info.stops.back()->coordinates));
            route_names_.push_back(getBusnameText(bus_name,info.stops.back()->coordinates, route_counter));
        }
        if (!info.is_cycled) {    
            for (int i = info.stops.size() - 2; i >= 0; i--){ 
                route.AddPoint(GetSvgPoint(info.stops[i]->coordinates));
            }
        }
        polylines_.push_back(route.SetFillColor(svg::Color()).SetStrokeColor(settings_.color_palette[route_counter % settings_.color_palette.size()]).SetStrokeWidth(settings_.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
        route_counter += 1;
    }
}

svg::Point getSvgPointOfProto(const catalogue_proto::Point& point_proto){
    return svg::Point(point_proto.x(), point_proto.y());
}

void MapRenderer::parseStopCirclesAndNames(){
    stop_circles_.reserve(catalogue_.GetStopsSet().size());
    stop_names_.reserve(catalogue_.GetStopsSet().size());
    for (const auto& stop_name : catalogue_.GetStopsSet()){
        const auto& stop_info = catalogue_.GetStopInfo(stop_name);
        if (stop_info.passing_buses.size() == 0){
            continue ;
        }
        svg::Circle circle;
        stop_circles_.push_back(circle.SetCenter(GetSvgPoint(stop_info.coordinates)).SetRadius(settings_.stop_radius).SetFillColor(svg::Color("white")));

        svg::Text underliner, stoptext;
        stop_names_.push_back(underliner.SetPosition(GetSvgPoint(stop_info.coordinates)).SetOffset(settings_.stop_label_offset).
            SetFontSize(settings_.stop_label_font_size).SetFontFamily("Verdana").SetData(stop_name).SetFillColor(settings_.underlayer_color).
            SetStrokeColor(settings_.underlayer_color).SetStrokeWidth(settings_.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).
            SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));

        stop_names_.push_back(stoptext.SetPosition(GetSvgPoint(stop_info.coordinates)).SetOffset(settings_.stop_label_offset).
            SetFontSize(settings_.stop_label_font_size).SetFontFamily("Verdana").SetData(stop_name).SetFillColor(svg::Color("black")));
    }
}

svg::Point MapRenderer::GetSvgPoint(geo::Coordinates coordinates){
    double x, y;
    if (coordinates.lng == scaler_.min_lon){
        x = scaler_.padding;
    } else {
        x = (coordinates.lng - scaler_.min_lon) * scaler_.zoom_coef + scaler_.padding;
    }
    if (coordinates.lat == scaler_.max_lat){
        y = scaler_.padding;
    } else {
        y = (scaler_.max_lat - coordinates.lat) * scaler_.zoom_coef + scaler_.padding; 
    }
    return svg::Point(x, y);
}

} // namespace map_renderer 