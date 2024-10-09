#pragma once

#include "geo.h"
#include "svg.h"
#include <string>
#include <vector>
#include <set>

namespace constants {
	inline const double EPSILON = 1e-6;
}

namespace trans_ctl {
	struct Stop {
		std::string name;
		geo::Coordinates coordinates;
	};

	struct Segment {
		Stop* stop_a;
		Stop* stop_b;
		double distance = 0;
	};

	struct BusStat {
		size_t stops_count = 0;
		size_t unique_stops_count = 0;
		double route_distance = 0;
		double route_length = 0;
	};

	struct Bus {
		std::string name;
		std::vector<Stop*> stops;
		bool isCircleRoute = false;
		BusStat stat;
	};

	struct BusCmp {
		bool operator()(const Bus* lhs, const Bus* rhs) const
		{
			return (*lhs).name < (*rhs).name;
		}
	};

	struct StopCmp {
		bool operator()(const Stop* lhs, const Stop* rhs) const
		{
			return (*lhs).name < (*rhs).name;
		}
	};

	struct StopsHasher {
		size_t operator() (const std::pair<Stop*, Stop*> segment) const {
			return reinterpret_cast<size_t>(segment.first) + 43 * reinterpret_cast<size_t>(segment.second);
		}
	};
}

namespace render {
	struct Size {
		double width;
		double height;
	};

	struct BusLabelOffset {
		double dx;
		double dy;
	};

	struct StopLabelOffset {
		double dx;
		double dy;
	};

	struct RenderSettings {
		Size size;
		double padding;
		double line_width;
		double stop_radius;
		int bus_label_font_size;
		BusLabelOffset bus_label_offset;
		int stop_label_font_size;
		StopLabelOffset stop_label_offset;
		svg::Color underlayer_color;
		double underlayer_width;
		std::vector<svg::Color> color_palette;
	};
}

namespace transport_router {
	struct Routing_settings {
		int bus_wait_time;
		double bus_velocity;
	};
}