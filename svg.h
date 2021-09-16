#pragma once
#define _USE_MATH_DEFINES

#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <sstream>
#include <utility>
#include <variant>
#include <vector>

namespace svg {

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

struct Rgb {
    Rgb(){}
    explicit Rgb(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

struct Rgba {
    Rgba(){}
    explicit Rgba(uint8_t r, uint8_t g, uint8_t b, double op) : red(r), green(g), blue(b), opacity(op) {}
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
};

using Color = std::variant<std::monostate, svg::Rgb, svg::Rgba, std::string>;

inline const Color NoneColor;

struct ColorPrinter {
    std::ostream& out;

    void operator()(std::monostate){
        using namespace std::literals;
        out << "none"sv;
    }

    void operator()(std::string str){
        out << str;
    }

    void operator() (svg::Rgb rgb){
        using namespace std::literals;
        out << "rgb("sv << 
            static_cast<int>(rgb.red) << ',' <<
            static_cast<int>(rgb.green) << ',' <<
            static_cast<int>(rgb.blue) << ')';
    }

    void operator()(svg::Rgba rgba){
        using namespace std::literals;
        out << "rgba("sv <<
            static_cast<int>(rgba.red) << ',' <<
            static_cast<int>(rgba.green) << ',' <<
            static_cast<int>(rgba.blue) << ',' <<
            rgba.opacity << ')';
    }
};

std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap);

std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join);

class ObjectContainer;

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
template <typename Owner>
class PathProps {
public:    
    Owner& SetFillColor(Color fill_color){
        fill_color_ = std::move(fill_color);
        return AsOwner();
    }
    Owner& SetStrokeColor(Color stroke_color){
        stroke_color_ = std::move(stroke_color);
        return AsOwner();
    }
    Owner& SetStrokeWidth(double width){
        width_ = width;
        return AsOwner();
    }
    Owner& SetStrokeLineCap(svg::StrokeLineCap line_cap){
        line_cap_ = line_cap;
        return AsOwner();
    }
    Owner& SetStrokeLineJoin(svg::StrokeLineJoin line_join){
        line_join_ = line_join;
        return AsOwner();
    }
protected:
    ~PathProps() = default;
    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;
        if (fill_color_ ){
            out << " fill=\""sv;
            std::visit(ColorPrinter{out}, *fill_color_);
            out << "\""sv;
        }
        if (stroke_color_ ){
            out << " stroke=\""sv;
            std::visit(ColorPrinter{out}, *stroke_color_); 
            out << "\""sv;
        }
        if (width_){
            out << " stroke-width=\""sv << *width_ << "\""sv;
        }
        if (line_cap_ ){
            out << " stroke-linecap=\""s;
            out << *line_cap_;
            out << "\""s;
        }
        if (line_join_){
            out << " stroke-linejoin=\""s ;
            out << *line_join_;
            out << "\""s;
        }
    }
private:
    Owner& AsOwner(){
        return static_cast<Owner&>(*this);
    }
    std::optional<Color> fill_color_ ;
    std::optional<Color> stroke_color_ ;
    std::optional<double> width_ ;
    std::optional<StrokeLineCap> line_cap_ ;
    std::optional<StrokeLineJoin> line_join_ ;
};

class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

    void RenderObject(const RenderContext& context) const override;

    /*
     * Прочие методы и данные, необходимые для реализации элемента <polyline>*/
private:
    std::vector<Point> polyline_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

    void RenderObject(const RenderContext& context) const override;

    // Прочие данные и методы, необходимые для реализации элемента <text>
private:
    Point position_ = {0.0, 0.0};
    Point offset_ = {0.0, 0.0};
    uint32_t size_ = 1;
    std::string font_family_ = {};
    std::string font_weight_ = {};
    std::string data_ = "";
};

class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;

    virtual ~Drawable() = default;
};

namespace shapes {

class Triangle : public svg::Drawable {
public:
    Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
        : p1_(p1)
        , p2_(p2)
        , p3_(p3) {
    }

    // Реализует метод Draw интерфейса svg::Drawable
    void Draw(svg::ObjectContainer& container) const override;

private:
    svg::Point p1_, p2_, p3_;
};

class Star : public svg::Drawable{
public:
    Star(svg::Point center, double outer_rad, double inner_rad, int num_rays)
    : center_(center),
    outer_rad_(outer_rad),
    inner_rad_(inner_rad),
    num_rays_(num_rays)
    {
    }

    void Draw(svg::ObjectContainer& container) const override;

private:
    svg::Point center_;
    double outer_rad_, inner_rad_;
    int num_rays_;
    Color fill_color_ = "red";
    Color stroke_color_ = "black";

    svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) const;
};

class Snowman : public svg::Drawable{
public:     
    Snowman(svg::Point top_center, double top_rad)
    : top_center_(top_center),
    top_rad_(top_rad){}

    void Draw(svg::ObjectContainer& container) const override;

private:
    svg::Point top_center_;
    double top_rad_;
    Color fill_color_ = "rgb(240,240,240)";
    Color stroke_color_ = "black";
};

} // namespace shapes 

class ObjectContainer {
public: 
    template <typename Obj>
    void Add(Obj object){
        AddPtr(std::make_unique<Obj>(std::move(object)));
    }

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
};

class Document final : public svg::ObjectContainer {
public:
    // Document() = default;
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    void Render(std::ostream& out) const;

private:
    std::vector<std::unique_ptr<Object>> objects_;
};

}  // namespace svg

