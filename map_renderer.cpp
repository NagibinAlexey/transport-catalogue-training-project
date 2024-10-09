#include "map_renderer.h"

namespace renderer {

    SphereProjector MapRenderer::CalcSphereProjector() const
    {
        // Точки, подлежащие проецированию
        std::vector<geo::Coordinates> geo_coords;

        for (const auto& [bus_ptr, color] : buses_colors_) {
            for (const auto& stop : bus_ptr->stops) {
                geo_coords.push_back(stop->coordinates);
            }
        }

        // Создаём проектор сферических координат на карту
        SphereProjector proj = {
            geo_coords.begin(), geo_coords.end(), render_settings_.size.width, 
                                                  render_settings_.size.height, 
                                                  render_settings_.padding
        };
        return proj;
    }

    void MapRenderer::SetBusesColors(std::map<trans_ctl::Bus*, svg::Color, trans_ctl::BusCmp> colors_map) {
        buses_colors_ = colors_map;

        for (const auto& [bus_ptr, color] : buses_colors_) {
            for (const auto& stop : bus_ptr->stops) {
                map_stops_.insert(stop);
            }
        }
    }

    void MapRenderer::RenderLines(SphereProjector& proj, svg::Document& doc) const
    {
        for (const auto& [bus_ptr, color] : buses_colors_) {
            svg::Polyline bus_line;
            for (const auto& stop : bus_ptr->stops) {
                bus_line.AddPoint(proj(stop->coordinates));
            }
            if (!bus_ptr->isCircleRoute) {
                for (auto it = bus_ptr->stops.rbegin() + 1; it != bus_ptr->stops.rend(); ++it) {
                    bus_line.AddPoint(proj((*it)->coordinates));
                }
            }
            bus_line.SetStrokeColor(color).SetFillColor("none")
                                          .SetStrokeWidth(render_settings_.line_width)
                                          .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                                          .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            doc.Add(std::move(bus_line));
        }
    }

    void MapRenderer::RenderBusNames(SphereProjector& proj, svg::Document& doc) const
    {
        for (const auto& [bus_ptr, color] : buses_colors_) {
            svg::Text first_bg_bus_name;
            svg::Text first_fg_bus_name;

            first_bg_bus_name.SetData(bus_ptr->name)
                .SetPosition(proj(bus_ptr->stops.front()->coordinates))
                .SetOffset({ render_settings_.bus_label_offset.dx, render_settings_.bus_label_offset.dy })
                .SetFontSize(render_settings_.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetFillColor(render_settings_.underlayer_color)
                .SetStrokeColor(render_settings_.underlayer_color)
                .SetStrokeWidth(render_settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            first_fg_bus_name.SetData(bus_ptr->name)
                .SetPosition(proj(bus_ptr->stops.front()->coordinates))
                .SetOffset({ render_settings_.bus_label_offset.dx, render_settings_.bus_label_offset.dy })
                .SetFontSize(render_settings_.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetFillColor(color);

            doc.Add(std::move(first_bg_bus_name));
            doc.Add(std::move(first_fg_bus_name));

            if (!bus_ptr->isCircleRoute && bus_ptr->stops.front() != bus_ptr->stops.back()) {
                svg::Text second_bg_bus_name;
                svg::Text second_fg_bus_name;

                second_bg_bus_name.SetData(bus_ptr->name)
                    .SetPosition(proj(bus_ptr->stops.back()->coordinates))
                    .SetOffset({ render_settings_.bus_label_offset.dx, render_settings_.bus_label_offset.dy })
                    .SetFontSize(render_settings_.bus_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetFontWeight("bold")
                    .SetFillColor(render_settings_.underlayer_color)
                    .SetStrokeColor(render_settings_.underlayer_color)
                    .SetStrokeWidth(render_settings_.underlayer_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

                second_fg_bus_name.SetData(bus_ptr->name)
                    .SetPosition(proj(bus_ptr->stops.back()->coordinates))
                    .SetOffset({ render_settings_.bus_label_offset.dx, render_settings_.bus_label_offset.dy })
                    .SetFontSize(render_settings_.bus_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetFontWeight("bold")
                    .SetFillColor(color);

                doc.Add(std::move(second_bg_bus_name));
                doc.Add(std::move(second_fg_bus_name));
            }
        }
    }

    void MapRenderer::RenderStops(SphereProjector& proj, svg::Document& doc) const
    {
        for (const auto& stop_ptr : map_stops_) {
            svg::Circle stop;

            stop.SetCenter(proj(stop_ptr->coordinates))
                .SetRadius(render_settings_.stop_radius)
                .SetFillColor("white");

            doc.Add(std::move(stop));
        }
    }

    void MapRenderer::RenderStopsNames(SphereProjector& proj, svg::Document& doc) const
    {
        for (const auto& stop_ptr : map_stops_) {
            svg::Text bg_stop_name;
            svg::Text fg_stop_name;

            bg_stop_name.SetData(stop_ptr->name)
                .SetPosition(proj(stop_ptr->coordinates))
                .SetOffset({ render_settings_.stop_label_offset.dx, render_settings_.stop_label_offset.dy })
                .SetFontSize(render_settings_.stop_label_font_size)
                .SetFontFamily("Verdana")
                .SetFillColor(render_settings_.underlayer_color)
                .SetStrokeColor(render_settings_.underlayer_color)
                .SetStrokeWidth(render_settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            fg_stop_name.SetData(stop_ptr->name)
                .SetPosition(proj(stop_ptr->coordinates))
                .SetOffset({ render_settings_.stop_label_offset.dx, render_settings_.stop_label_offset.dy })
                .SetFontSize(render_settings_.stop_label_font_size)
                .SetFontFamily("Verdana")
                .SetFillColor("black");

            doc.Add(std::move(bg_stop_name));
            doc.Add(std::move(fg_stop_name));
        }
    }

    svg::Document MapRenderer::Render() const
    {
        svg::Document doc;

        SphereProjector proj = CalcSphereProjector();

        RenderLines(proj, doc);
        RenderBusNames(proj, doc);
        RenderStops(proj, doc);
        RenderStopsNames(proj, doc);
        
        return doc;
    }
}