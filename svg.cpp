
#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap){
    using namespace std::literals; 
    if (line_cap == StrokeLineCap::BUTT){
        out << "butt"s;
    } else if (line_cap == StrokeLineCap::ROUND){
        out << "round"s;
    } else if (line_cap == StrokeLineCap::SQUARE){
        out << "square"s;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join){
    using namespace std::literals;
    if (line_join == StrokeLineJoin::ARCS){
        out << "arcs"s;
    } else if (line_join == StrokeLineJoin::BEVEL){
        out << "bevel"s;
    } else if (line_join == StrokeLineJoin::MITER){
        out << "miter"sv;
    } else if (line_join == StrokeLineJoin::MITER_CLIP){
        out << "miter-clip"s;
    } else if (line_join == StrokeLineJoin::ROUND){
        out << "round"s;
    }
    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

//----------- Polyline -------------

Polyline& Polyline::AddPoint(Point point) {
    polyline_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    bool begin = true;
    auto& out = context.out;
    out << "<polyline points=\""sv;
    for (const auto& point : polyline_){
        if (!begin){
            out << ' ';
        }
        out << point.x << ',' << point.y;
        begin = false;
    }
    out << "\""sv;
    RenderAttrs(context.out);
    out << " />"sv;
}

//----------- Text -------------

Text& Text::SetPosition(Point point){
    position_ = point;
    return *this;
}

Text& Text::SetOffset(Point point){
    offset_ = point;
    return *this;
}

Text& Text::SetFontSize(uint32_t size){
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family){
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight){
    font_weight_ = std::move(font_weight);
    return *this;
}

Text& Text::SetData(std::string data){
    data_ = std::move(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text "sv 
        << "x=\""sv << position_.x << "\" "sv
        << "y=\""sv << position_.y << "\" "sv
        << "dx=\""sv << offset_.x << "\" "sv
        << "dy=\""sv << offset_.y << "\" "sv
        << "font-size=\""sv << size_ << "\" "sv;
        if (font_family_.size()){
            out << "font-family=\""sv << font_family_ << "\" "sv;
        }
        if (font_weight_.size()){
            out << "font-weight=\""sv << font_weight_ << "\" "sv;
        }
        RenderAttrs(context.out);
        out << ">"sv << data_ << "</text>"sv;

}

//----------- Document -------------

void Document::AddPtr(std::unique_ptr<Object>&& obj){
    objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    RenderContext ctx(out, 2, 2);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for (auto& obj : objects_){
        obj->Render(ctx);
    }
    out << "</svg>"sv << std::endl;
}

namespace shapes {

void Triangle::Draw(svg::ObjectContainer& container) const {
    container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
}

void Star::Draw(svg::ObjectContainer& container) const {
    container.Add(CreateStar(center_, outer_rad_, inner_rad_, num_rays_));
}

svg::Polyline Star::CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) const {
    using namespace svg;
    Polyline polyline;
    for (int i = 0; i <= num_rays; ++i) {
        double angle = 2 * M_PI * (i % num_rays) / num_rays;
        polyline.AddPoint({center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle)});
        if (i == num_rays) {
            break;
        }
        angle += M_PI / num_rays;
        polyline.AddPoint({center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle)});
    }
    polyline.SetFillColor(fill_color_).SetStrokeColor(stroke_color_);
    return polyline;
}

void Snowman::Draw(svg::ObjectContainer& container) const {
    container.Add(svg::Circle().SetCenter({top_center_.x, top_center_.y + top_rad_ * 5}).SetRadius(top_rad_ * 2).SetFillColor(fill_color_).SetStrokeColor(stroke_color_));
    container.Add(svg::Circle().SetCenter({top_center_.x, top_center_.y + top_rad_ * 2}).SetRadius(top_rad_ * 1.5).SetFillColor(fill_color_).SetStrokeColor(stroke_color_));
    container.Add(svg::Circle().SetCenter(top_center_).SetRadius(top_rad_).SetFillColor(fill_color_).SetStrokeColor(stroke_color_));
}

} // namespace shapes

}  // namespace svg