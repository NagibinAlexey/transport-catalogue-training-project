#include "svg.h"

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ----------- Polyline -----------------

    Polyline& Polyline::AddPoint(Point point) {
        std::ostringstream s;
        s << point.x << "," << point.y;
        if (!points_.empty()) points_.append(" "sv);
        points_.append(s.str());
        return *this;
    }

    Polyline Polyline::CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) {
        using namespace svg;
        Polyline polyline;
        for (int i = 0; i <= num_rays; ++i) {
            double angle = 2 * M_PI * (i % num_rays) / num_rays;
            polyline.AddPoint({ center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle) });
            if (i == num_rays) {
                break;
            }
            angle += M_PI / num_rays;
            polyline.AddPoint({ center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle) });
        }
        return polyline;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv << points_ << "\" "sv;
        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Text ------------------------

    Text& Text::SetPosition(Point pos) {
        pos_.x = pos.x;
        pos_.y = pos.y;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_.x = offset.x;
        offset_.y = offset.y;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_.clear();
        for (char& c : data) {
            if (c == '"') { data_.append("&quot;"); }
            else if (c == '\'') { data_.append("&apos;"); }
            else if (c == '<') { data_.append("&lt;"); }
            else if (c == '>') { data_.append("&gt;"); }
            else if (c == '&') { data_.append("&amp;"); }
            else data_.push_back(c);
        }
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text";
        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << size_;
        if (!font_family_.empty()) {
            out << "\" font-family=\""sv << font_family_;
        }
        if (!font_weight_.empty()) {
            out << "\" font-weight=\""sv << font_weight_;
        }
        out << "\">"sv;
        out << data_ << "</text>"sv;
    }

    // --------------- Document --------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for (const auto& obj : objects_) {
            RenderContext ctx(out, 2, 2);
            obj->Render(ctx);
        }
        out << "</svg>"sv;
    }

}  // namespace svg