#pragma once
#include <variant>
#include <vector>
#include <string>
#include <sstream>
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

class MapRenderer {
public:
    MapRenderer(const transport_guide::TransportCatalogue* catalogue, RenderSettings settings) :
        catalogue_(catalogue),
        settings_(settings)
    {}

    MapRenderer(const catalogue_proto::TransportCatalogue* proto_catalogue) :
        proto_catalogue_(proto_catalogue),
    {}

    svg::Document GetSvgDocument();

private:

    struct ScalerStruct{
        double min_lat = 0.0;
        double max_lat = 0.0;
        double min_lon = 0.0;
        double max_lon = 0.0;
        double zoom_coef = 0.0;
        double padding = 0.0;
    };

    const transport_guide::TransportCatalogue* catalogue_ = nullptr;
    const catalogue_proto::TransportCatalogue* proto_catalogue_ = nullptr;
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

    void addObjectsToDoc(svg::Document& document);
    
    void makeScaler();
    
    svg::Text getBusnameUnder(const std::string& bus_name, geo::Coordinates coordinates);
    
    svg::Text getBusnameText(const std::string& bus_name, geo::Coordinates coordinates, int route_counter);
    
    void parsePolylinesAndRouteNames();

    void parsePolylinesAndRouteNamesProto();

    void parseStopCirclesAndNamesProto();
    
    void parseStopCirclesAndNames();
    
    svg::Point GetSvgPoint(geo::Coordinates stop_coordinates);

    void makeScalerOfProtoCatalogue();

    void makeScalerOfCatalogue();

}; 

svg::Color getSvgColorOfProto(const catalogue_proto::Color& proto_color);

}//namespace map_renderer
