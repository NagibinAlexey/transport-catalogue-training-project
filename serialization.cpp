#include "serialization.h"

namespace serialize {

	/*--------------------------------------------------------------------- SERIALIZE ----------------------------------------------------------------------*/

	void Serializer::SerializeStops() {
		const auto& stops_map = catalogue_.GetAllStops();
		uint32_t id = 0;

		for (const auto& [key, stop_ptr] : stops_map) {
			trans_catalogue_serialize::Stop* serialize_stop = serialized_catalogue_.add_stop();
			//serialize name
			std::string stop_name = stop_ptr->name;
			std::vector<uint8_t> encoded_name(stop_name.begin(), stop_name.end());
			serialize_stop->set_name(encoded_name.data(), encoded_name.size());

			//serialize coordinates
			serialize_stop->mutable_coordinates()->set_lat(stop_ptr->coordinates.lat);
			serialize_stop->mutable_coordinates()->set_lng(stop_ptr->coordinates.lng);

			//serialize id
			serialize_stop->set_id(id);
			stop_to_id.insert({ stop_name, id });
			++id;
		}
	}

	void Serializer::SerializeStopDistances() {
		const auto& stop_lenghts = catalogue_.GetAllStopLengths();
		for (const auto& [stops, distance] : stop_lenghts) {
			trans_catalogue_serialize::StopDistances* serialize_stop_distances = serialized_catalogue_.add_distance();
			auto first_stop_name = stops.first->name;
			auto second_stop_name = stops.second->name;

			serialize_stop_distances->set_first_id(stop_to_id.at(first_stop_name));
			serialize_stop_distances->set_second_id(stop_to_id.at(second_stop_name));
			serialize_stop_distances->set_distance(distance);
		}
	}

	void Serializer::SerializeBusses() {
		const auto& routes_map = catalogue_.GetAllRoutes();
		for (const auto& [key, bus_ptr] : routes_map) {
			trans_catalogue_serialize::Bus* serialize_bus = serialized_catalogue_.add_route();
			//serialize name
			std::string bus_name = bus_ptr->name;
			std::vector<uint8_t> encoded_name(bus_name.begin(), bus_name.end());
			serialize_bus->set_name(encoded_name.data(), encoded_name.size());

			//serialize stops
			for (const auto& stop : bus_ptr->stops) {
				std::string stop_name = stop->name;
				serialize_bus->add_stops_by_id(stop_to_id.at(stop_name));
			}

			//serialize route type
			serialize_bus->set_is_circle_route(bus_ptr->isCircleRoute);

			//serialize statistics
			serialize_bus->mutable_stat()->set_stops_count(static_cast<int>(bus_ptr->stat.stops_count));
			serialize_bus->mutable_stat()->set_unique_stops_count(static_cast<int>(bus_ptr->stat.unique_stops_count));
			serialize_bus->mutable_stat()->set_route_distance(static_cast<int>(bus_ptr->stat.route_distance));
			serialize_bus->mutable_stat()->set_route_length(static_cast<int>(bus_ptr->stat.route_length));
		}
	}

	void Serializer::SerializeRenderSettings(const JsonReader& json_reader, const json::Document& document) {
		render::RenderSettings render_settings = json_reader.ParseRenderSettings(document);

		render_settings_serialize::RenderSettings* render_settings_ = serialized_catalogue_.mutable_render_settings();

		render_settings_->mutable_size()->set_width(render_settings.size.width);
		render_settings_->mutable_size()->set_height(render_settings.size.height);
		render_settings_->set_padding(render_settings.padding);
		render_settings_->set_line_width(render_settings.line_width);
		render_settings_->set_stop_radius(render_settings.stop_radius);
		render_settings_->set_bus_label_font_size(render_settings.bus_label_font_size);
		render_settings_->mutable_bus_label_offset()->set_dx(render_settings.bus_label_offset.dx);
		render_settings_->mutable_bus_label_offset()->set_dy(render_settings.bus_label_offset.dy);
		render_settings_->set_stop_label_font_size(render_settings.stop_label_font_size);
		render_settings_->mutable_stop_label_offset()->set_dx(render_settings.stop_label_offset.dx);
		render_settings_->mutable_stop_label_offset()->set_dy(render_settings.stop_label_offset.dy);
		*render_settings_->mutable_underlayer_color() = SerializeColor(render_settings.underlayer_color);
		render_settings_->set_underlayer_width(render_settings.underlayer_width);

		for (const auto& color : render_settings.color_palette) {
			*render_settings_->add_color_palette() = SerializeColor(color);
		}
	}

	void Serializer::SerializeRouterSettings(const JsonReader& json_reader, const json::Document& document) {
		transport_router::Routing_settings routing_settings = json_reader.ParseRoutingSettings(document);

		router_serialize::RoutingSettings* routing_settings_ = serialized_catalogue_.mutable_routing_settings();
		routing_settings_->set_bus_wait_time(routing_settings.bus_wait_time);
		routing_settings_->set_bus_velocity(routing_settings.bus_velocity);
	}

	void Serializer::SerializeGraph(const graph::DirectedWeightedGraph<double>& graph) {
		size_t edge_count = graph.GetEdgeCount();
		for (size_t i = 0; i < edge_count; ++i) {
			graph_serialize::Edge* serialize_edge = serialized_catalogue_.mutable_graph()->add_edge();

			serialize_edge->set_from(graph.GetEdge(i).from);
			serialize_edge->set_to(graph.GetEdge(i).to);
			serialize_edge->set_weight(graph.GetEdge(i).weight);
		}
		size_t incidence_list_count = graph.GetVertexCount();
		for (size_t i = 0; i < incidence_list_count; ++i) {
			graph_serialize::IncidenceList* serialize_incidence_list = serialized_catalogue_.mutable_graph()->add_incidence_list();

			for (const auto& edge_id : graph.GetIncidentEdges(i)) {
				serialize_incidence_list->add_edge_id(edge_id);
			}
		}
	}

	void Serializer::SerializeRouter(const transport_router::TRouter& router) {
		router_serialize::Router* router_ = serialized_catalogue_.mutable_router();

		auto waiting_stops_ids_ = router.GetWaitingStopsIds();
		for (const auto& [name, id] : waiting_stops_ids_) {
			router_serialize::MapWaitingStopsIds* serialize_waiting_stops = router_->add_waiting_stops_ids();
			serialize_waiting_stops->set_key(name.data(), name.size());
			serialize_waiting_stops->set_value(id);
		}

		auto stops_ids_ = router.GetStopsIds();
		for (const auto& [name, id] : stops_ids_) {
			router_serialize::MapStopsIds* serialize_stops = router_->add_stops_ids();
			serialize_stops->set_key(name.data(), name.size());
			serialize_stops->set_value(id);
		}

		auto edge_ids_ = router.GetEdgeIdMap();
		for (const auto& [id, edge] : edge_ids_) {
			router_serialize::MapEdges* serialize_edge = router_->add_id_to_bus_stop();
			serialize_edge->set_key(id);
			serialize_edge->mutable_value()->set_bus_name(edge.bus_name.data(), edge.bus_name.size());
			serialize_edge->mutable_value()->set_stop_name(edge.stop_name.data(), edge.stop_name.size());
			serialize_edge->mutable_value()->set_span_count(edge.span_count);
		}
	}

	void Serializer::SerializeTransportCatalogue()
	{
		SerializeStops();
		SerializeStopDistances();
		SerializeBusses();
	}

	void Serializer::SaveTo(std::ostream& output) {
		serialized_catalogue_.SerializeToOstream(&output);
	}

	/*--------------------------------------------------------------------- DESERIALIZE ----------------------------------------------------------------------*/

	void Deserializer::DeserializeStops(std::istream& input) {
		if (serialized_catalogue_.ParseFromIstream(&input)) {
			int stops_count = serialized_catalogue_.stop_size();
			for (int i = 0; i < stops_count; ++i) {
				trans_ctl::Stop stop;
				//deserialize name
				stop.name = serialized_catalogue_.mutable_stop(i)->name();

				//deserialize coordinates
				stop.coordinates.lat = serialized_catalogue_.mutable_stop(i)->coordinates().lat();
				stop.coordinates.lng = serialized_catalogue_.mutable_stop(i)->coordinates().lng();

				id_to_stop.insert({ serialized_catalogue_.mutable_stop(i)->id(), stop.name });

				//add full stop structs to catalogue
				catalogue_.AddStop(stop);
			}
		}
		else {
			throw std::runtime_error("Deserialization Error!");
		}
	}

	void Deserializer::DeserializeStopDistances() {
		int distances_count = serialized_catalogue_.distance_size();
		for (int i = 0; i < distances_count; ++i) {
			auto first_stop_name = id_to_stop.at(serialized_catalogue_.mutable_distance(i)->first_id());
			auto second_stop_name = id_to_stop.at(serialized_catalogue_.mutable_distance(i)->second_id());

			trans_ctl::Stop* first_stop = catalogue_.FindStop(first_stop_name);
			trans_ctl::Stop* second_stop = catalogue_.FindStop(second_stop_name);
			double distance = serialized_catalogue_.mutable_distance(i)->distance();

			//add distance struct to catalogue
			catalogue_.SetStopsLength(first_stop, second_stop, distance);
		}
	}

	void Deserializer::DeserializeBusses() {
		int buses_count = serialized_catalogue_.route_size();
		for (int i = 0; i < buses_count; ++i) {
			trans_ctl::Bus bus;
			//deserialize name
			bus.name = serialized_catalogue_.mutable_route(i)->name();

			//deserialize stops
			int stops_in_bus_count = serialized_catalogue_.mutable_route(i)->stops_by_id_size();
			for (int j = 0; j < stops_in_bus_count; ++j) {
				std::string stop_name = id_to_stop.at(serialized_catalogue_.mutable_route(i)->stops_by_id(j));
				bus.stops.push_back(catalogue_.FindStop(stop_name));
			}

			//deserialize route type
			bus.isCircleRoute = serialized_catalogue_.mutable_route(i)->is_circle_route();

			//deserialize bus stats
			bus.stat.stops_count = serialized_catalogue_.mutable_route(i)->stat().stops_count();
			bus.stat.unique_stops_count = serialized_catalogue_.mutable_route(i)->stat().unique_stops_count();
			bus.stat.route_distance = serialized_catalogue_.mutable_route(i)->stat().route_distance();
			bus.stat.route_length = serialized_catalogue_.mutable_route(i)->stat().route_length();

			//add full bus structs to catalogue
			catalogue_.AddBus(bus);
		}
	}

	render::RenderSettings Deserializer::DeserializeRenderSettings() {
		render::RenderSettings render_settings_;

		render_settings_.size.width = serialized_catalogue_.render_settings().size().width();
		render_settings_.size.height = serialized_catalogue_.render_settings().size().height();
		render_settings_.padding = serialized_catalogue_.render_settings().padding();
		render_settings_.line_width = serialized_catalogue_.render_settings().line_width();
		render_settings_.stop_radius = serialized_catalogue_.render_settings().stop_radius();
		render_settings_.bus_label_font_size = serialized_catalogue_.render_settings().bus_label_font_size();
		render_settings_.bus_label_offset.dx = serialized_catalogue_.render_settings().bus_label_offset().dx();
		render_settings_.bus_label_offset.dy = serialized_catalogue_.render_settings().bus_label_offset().dy();
		render_settings_.stop_label_font_size = serialized_catalogue_.render_settings().stop_label_font_size();
		render_settings_.stop_label_offset.dx = serialized_catalogue_.render_settings().stop_label_offset().dx();
		render_settings_.stop_label_offset.dy = serialized_catalogue_.render_settings().stop_label_offset().dy();
		render_settings_.underlayer_color = DeserializeColor(serialized_catalogue_.render_settings().underlayer_color());
		render_settings_.underlayer_width = serialized_catalogue_.render_settings().underlayer_width();

		int color_palette_size = serialized_catalogue_.render_settings().color_palette_size();
		for (int i = 0; i < color_palette_size; ++i) {
			svg::Color current_color = DeserializeColor(*serialized_catalogue_.mutable_render_settings()->mutable_color_palette(i));
			render_settings_.color_palette.push_back(current_color);
		}

		return render_settings_;
	}

	transport_router::Routing_settings Deserializer::DeserializeRouterSettings() {
		transport_router::Routing_settings routing_settings_;

		routing_settings_.bus_wait_time = serialized_catalogue_.routing_settings().bus_wait_time();
		routing_settings_.bus_velocity = serialized_catalogue_.routing_settings().bus_velocity();

		return routing_settings_;
	}

	graph::DirectedWeightedGraph<double> Deserializer::DeserializeGraph() {
		int vertex_count = serialized_catalogue_.graph().incidence_list_size();
		graph::DirectedWeightedGraph<double> graph_(vertex_count);

		int edge_count = serialized_catalogue_.graph().edge_size();
		for (int i = 0; i < edge_count; ++i) {
			graph_.AddEdge({ serialized_catalogue_.graph().edge(i).from(), serialized_catalogue_.graph().edge(i).to(), serialized_catalogue_.graph().edge(i).weight() });
		}

		return graph_;
	}

	transport_router::TRouter Deserializer::DeserializeRouter(trans_ctl::TransportCatalogue& catalogue) {
		std::unordered_map<std::string_view, size_t> waiting_stops_ids_;
		int waiting_stops_ids_size = serialized_catalogue_.router().waiting_stops_ids_size();
		for (int i = 0; i < waiting_stops_ids_size; ++i) {
			trans_ctl::Stop* stop = catalogue.FindStop(serialized_catalogue_.router().waiting_stops_ids(i).key());
			waiting_stops_ids_.insert({ stop->name, serialized_catalogue_.router().waiting_stops_ids(i).value() });
		}

		std::unordered_map<std::string_view, size_t> stops_ids_;
		int stops_ids_size = serialized_catalogue_.router().stops_ids_size();
		for (int i = 0; i < stops_ids_size; ++i) {
			trans_ctl::Stop* stop = catalogue.FindStop(serialized_catalogue_.router().stops_ids(i).key());
			stops_ids_.insert({ stop->name, serialized_catalogue_.router().stops_ids(i).value() });
		}

		std::unordered_map<size_t, transport_router::EdgeIdtoBus> id_to_bus_stop_;
		int edge_ids_size = serialized_catalogue_.router().id_to_bus_stop_size();
		for (int i = 0; i < edge_ids_size; ++i) {
			transport_router::EdgeIdtoBus edge;
			trans_ctl::Bus* bus = catalogue.FindBus(serialized_catalogue_.router().id_to_bus_stop(i).value().bus_name());
			if (bus == nullptr) {
				edge.bus_name = "waiting";
			}
			else {
				edge.bus_name = bus->name;
			}
			trans_ctl::Stop* stop = catalogue.FindStop(serialized_catalogue_.router().id_to_bus_stop(i).value().stop_name());
			edge.stop_name = stop->name;
			edge.span_count = serialized_catalogue_.router().id_to_bus_stop(i).value().span_count();

			id_to_bus_stop_.insert({ serialized_catalogue_.router().id_to_bus_stop(i).key(), std::move(edge) });
		}

		transport_router::TRouter filled_router(DeserializeRouterSettings(), 
											    catalogue,
			                                    waiting_stops_ids_,
												stops_ids_,
											    id_to_bus_stop_
			                                    );

		return filled_router;
	}

	trans_ctl::TransportCatalogue Deserializer::DeserializeTransportCatalogue(std::istream& input)
	{
		DeserializeStops(input);
		DeserializeStopDistances();
		DeserializeBusses();
		return catalogue_;
	}

	svg_serialize::Color SerializeColor(const svg::Color& color) {
		svg_serialize::Color color_;

		if (color.index() == 1) {
			color_.Clear();
			color_.set_string_color(std::get<std::string>(color));
		}

		else if (color.index() == 2) {
			color_.Clear();
			color_.mutable_rgb()->set_red(std::get<svg::Rgb>(color).red);
			color_.mutable_rgb()->set_green(std::get<svg::Rgb>(color).green);
			color_.mutable_rgb()->set_blue(std::get<svg::Rgb>(color).blue);
		}

		else if (color.index() == 3) {
			color_.Clear();
			color_.mutable_rgba()->set_red(std::get<svg::Rgba>(color).red);
			color_.mutable_rgba()->set_green(std::get<svg::Rgba>(color).green);
			color_.mutable_rgba()->set_blue(std::get<svg::Rgba>(color).blue);
			color_.mutable_rgba()->set_opacity(std::get<svg::Rgba>(color).opacity);
		}
		return color_;
	}

	svg::Color DeserializeColor(const svg_serialize::Color& color) {
		svg::Color color_{};

		if (color.color_case() == 1) { color_ = svg::Color{ color.string_color() }; }
		else if (color.color_case() == 2) { color_ = svg::Color{ svg::Rgb{static_cast<uint8_t>(color.rgb().red()), static_cast<uint8_t>(color.rgb().green()), static_cast<uint8_t>(color.rgb().blue())}}; }
		else if (color.color_case() == 3) { color_ = svg::Color{ svg::Rgba{static_cast<uint8_t>(color.rgba().red()), static_cast<uint8_t>(color.rgba().green()), static_cast<uint8_t>(color.rgba().blue()), color.rgba().opacity()} }; }

		return color_;
	}
}
