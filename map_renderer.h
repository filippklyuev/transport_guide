#pragma once
#include <variant>
#include <vector>
#include <string>
#include <sstream>
#include <utility>
#include "transport_catalogue.h"
#include "geo.h"
#include "svg.h"

namespace map_renderer {

struct RenderSettings {
    double width;                                                                   
    double height;
    double padding;
    double line_width;
    double stop_radius;
    int bus_label_font_size;
    svg::Point bus_label_offset;
    int stop_label_font_size;
    svg::Point stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width;
    std::vector<svg::Color> color_palette;
};

class MapRenderer {
public:
    MapRenderer(const transport_guide::TransportCatalogue& catalogue, RenderSettings settings) :
        catalogue_(catalogue),
        settings_(settings)
    {
    }
    svg::Document GetSvgDocument();


private:

    struct ScalerStruct{
        double min_lat;
        double max_lat;
        double min_lon;
        double max_lon;
        double zoom_coef = 0.0;
        double padding = 0.0;
    };

    const transport_guide::TransportCatalogue& catalogue_;
    RenderSettings settings_;
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
    
    void parseStopCirclesAndNames();
    
    svg::Point GetSvgPoint(geo::Coordinates stop_coordinates);

}; 

}//namespace map_renderer


// width и height — ключи, которые задают ширину и высоту в пикселях. Вещественное число в диапазоне от 0 до 100000.
// padding — отступ краёв карты от границ SVG-документа. Вещественное число не меньше 0 и меньше min(width, height)/2.
// line_width — толщина линий, которыми рисуются автобусные маршруты. Вещественное число в диапазоне от 0 до 100000.
// stop_radius — радиус окружностей, которыми обозначаются остановки. Вещественное число в диапазоне от 0 до 100000.
// bus_label_font_size — размер текста, которым написаны названия автобусных маршрутов. Целое число в диапазоне от 0 до 100000.
// bus_label_offset — смещение надписи с названием маршрута относительно координат конечной остановки на карте. Массив из двух элементов типа double. Задаёт значения свойств dx и dy SVG-элемента <text>. Элементы массива — числа в диапазоне от –100000 до 100000.
// stop_label_font_size — размер текста, которым отображаются названия остановок. Целое число в диапазоне от 0 до 100000.
// stop_label_offset — смещение названия остановки относительно её координат на карте. Массив из двух элементов типа double. Задаёт значения свойств dx и dy SVG-элемента <text>. Числа в диапазоне от –100000 до 100000.
// underlayer_color — цвет подложки под названиями остановок и маршрутов. Формат хранения цвета будет ниже.
// underlayer_width — толщина подложки под названиями остановок и маршрутов. Задаёт значение атрибута stroke-width элемента <text>. Вещественное число в диапазоне от 0 до 100000.
// color_palette — цветовая палитра. Непустой массив.