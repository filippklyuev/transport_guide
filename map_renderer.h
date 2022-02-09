#pragma once
#include <variant>
#include <vector>
#include <string>
#include <sstream>
#include <optional>
#include <utility>
#include <transport_catalogue.pb.h>
#include "transport_catalogue.h"
#include "geo.h"
#include "svg.h"

namespace map_renderer {

struct RenderSettings {
    double width = 0.0;                                                                   
    double height = 0.0;
    double padding = 0.0;
    double line_width = 0.0;
    double stop_radius = 0.0;
    int bus_label_font_size = 0;
    svg::Point bus_label_offset;
    int stop_label_font_size = 0;
    svg::Point stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width = 0.0;
    std::vector<svg::Color> color_palette;
};

struct ScalerStruct{
    double min_lat = 0.0;
    double max_lat = 0.0;
    double min_lon = 0.0;
    double max_lon = 0.0;
    double zoom_coef = 0.0;
    double padding = 0.0;
};

class MapRenderer {
public:
    MapRenderer(const transport_guide::TransportCatalogue& catalogue, RenderSettings settings) :
        catalogue_(catalogue),
        settings_(settings)
    {}

    svg::Document GetSvgDocument();

private:

    const transport_guide::TransportCatalogue& catalogue_;
    RenderSettings settings_ = {};
    ScalerStruct scaler_;
    std::vector<svg::Polyline> polylines_;
    std::vector<svg::Text> route_names_;
    std::vector<svg::Circle> stop_circles_;
    std::vector<svg::Text> stop_names_;    
    
    template<typename IterIt>
    void addObjectsToDoc(IterIt begin, IterIt end, svg::Document& document){
        for (auto it = begin; it != end; it++){
            document.Add(std::move(*it));
        }
    }

    template<typename Repeated,typename Map>
    void InitilizeCatalogueMap(const Repeated& repeated, Map& map){
        map.emplace(std::map<std::string_view, int>{});
        for (int i = 0; i < repeated.size(); i++){
            map->emplace(repeated.at(i).name(), i);
        }
    }

    void addObjectsToDoc(svg::Document& document);
    
    svg::Text getBusnameUnder(const std::string& bus_name, geo::Coordinates coordinates);
    
    svg::Text getBusnameText(const std::string& bus_name, geo::Coordinates coordinates, int route_counter);
    
    void parsePolylinesAndRouteNames();   
    
    void parseStopCirclesAndNames();
    
    svg::Point GetSvgPoint(geo::Coordinates stop_coordinates);

    void makeScaler();

}; 

}//namespace map_renderer
