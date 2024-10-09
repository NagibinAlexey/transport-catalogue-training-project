#include "transport_catalogue.h"

namespace trans_ctl {
	void TransportCatalogue::SetStopsLength(Stop* first_stop, Stop* second_stop, double distance)
	{
		segments_map_.insert({ std::make_pair(first_stop, second_stop), distance });
	}

	double TransportCatalogue::GetStopsLength(Stop* first_stop, Stop* second_stop) const
	{
		if (first_stop == second_stop && segments_map_.count({ first_stop, second_stop }) == 0) { return 0.0; }
		else if (segments_map_.count({ first_stop, second_stop }) == 0)
		{
			return segments_map_.at({ second_stop, first_stop });
		}
		return segments_map_.at({ first_stop, second_stop });
	}

	std::unordered_map<std::pair<Stop*, Stop*>, double, StopsHasher> TransportCatalogue::GetAllStopLengths() const {
		return segments_map_;
	}

	void TransportCatalogue::AddStop(const Stop& stop)
	{
		stops_.push_back(stop);
		stops_map_.insert({ stops_.back().name, &stops_.back() });
	}

	Stop* TransportCatalogue::FindStop(const std::string_view stop_name) const
	{
		if (stops_map_.count(stop_name) == 0) return nullptr;
		return stops_map_.at(stop_name);
	}

	void TransportCatalogue::AddBus(const Bus& bus)
	{
		routes_.push_back(bus);
		routes_info_.insert({ routes_.back().name, &routes_.back() });

		for (const auto& stop_ptr : bus.stops) {
			stops_info_[stop_ptr].insert(&routes_.back());
		}
	}

	Bus* TransportCatalogue::FindBus(const std::string_view bus_name) const
	{
		if (routes_info_.count(bus_name) == 0) return nullptr;
		return routes_info_.at(bus_name);
	}

	bool TransportCatalogue::BusIsEmpty(Bus* bus) const {
		return bus->stops.empty();
	}

	size_t TransportCatalogue::GetStopsCount() const
	{
		return stops_.size();
	}

	std::unordered_map<std::string_view, Bus*> TransportCatalogue::GetAllRoutes() const
	{
		return routes_info_;
	}
	
	std::unordered_map<std::string_view, Stop*> TransportCatalogue::GetAllStops() const
	{
		return stops_map_;
	}

	BusStat* TransportCatalogue::ExecuteBusRequest(Bus* bus)
	{
		if (bus == nullptr) return nullptr;

		if ((*bus).stat.stops_count == 0) {
			return CalcBusStat(bus);
		}

		return &bus->stat;
	}

	std::set<Bus*, BusCmp> TransportCatalogue::ExecuteStopRequest(Stop* stop) const
	{
		std::set<Bus*, BusCmp> buses;

		if (stop == nullptr || stops_info_.count(stop) == 0) {
			return buses;
		}

		buses = stops_info_.at(stop);
		return buses;
	}

	BusStat* TransportCatalogue::CalcBusStat(Bus* route)
	{
		calc::RouteStops(route);
		calc::RouteUniqueStops(route);
		calc::RouteDistance(route);
		calc::RouteLength(*this, route);

		return &(*route).stat;
	}

	namespace calc {
		void RouteDistance(Bus* route)
		{
			double route_distance = 0.0;
			for (size_t i = 0; i < (*route).stops.size() - 1; ++i) {
				auto stop_left_ptr = (*route).stops[i];
				auto stop_right_ptr = (*route).stops[i + 1];

				route_distance += geo::ComputeDistance({ (*stop_left_ptr).coordinates.lat, (*stop_left_ptr).coordinates.lng },
					{ (*stop_right_ptr).coordinates.lat, (*stop_right_ptr).coordinates.lng });
			}
			if (!(*route).isCircleRoute) { route_distance *= 2; }

			(*route).stat.route_distance = route_distance;
		}

		void RouteLength(TransportCatalogue& catalogue, Bus* route)
		{
			double route_length = 0;
			if ((*route).isCircleRoute) {
				for (size_t i = 0; i < (*route).stops.size() - 1; ++i) {
					auto stop_left_ptr = (*route).stops[i];
					auto stop_right_ptr = (*route).stops[i + 1];

					route_length += catalogue.GetStopsLength(stop_left_ptr, stop_right_ptr);
				}
			}
			else {
				for (size_t i = 0; i < (*route).stops.size() - 1; ++i) {
					auto stop_left_ptr = (*route).stops[i];
					auto stop_right_ptr = (*route).stops[i + 1];

					route_length += catalogue.GetStopsLength(stop_left_ptr, stop_right_ptr) +
						catalogue.GetStopsLength(stop_right_ptr, stop_left_ptr);
				}
			}
			(*route).stat.route_length = route_length;
		}

		void RouteStops(Bus* route)
		{
			if ((*route).isCircleRoute) {
				(*route).stat.stops_count = (*route).stops.size();
			}
			else {
				(*route).stat.stops_count = 2 * (*route).stops.size() - 1;
			}
		}

		void RouteUniqueStops(Bus* route)
		{
			Bus temp_route = (*route);
			std::sort(temp_route.stops.begin(), temp_route.stops.end());
			auto last = std::unique(temp_route.stops.begin(), temp_route.stops.end());
			(*route).stat.unique_stops_count = (last - temp_route.stops.begin());
		}
	}
}