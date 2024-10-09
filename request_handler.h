#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

class RequestHandler {
public:
    RequestHandler(trans_ctl::TransportCatalogue& db, const renderer::MapRenderer& renderer, const transport_router::TRouter& router)
        : db_(db), renderer_(renderer), router_(router) {};

    // Возвращает маршрут 
    trans_ctl::Bus* GetBus(const std::string_view& bus_name) const;

    // Возвращает все маршруты
    std::unordered_map<std::string_view, trans_ctl::Bus*> GetAllBuses() const;

    // Возвращает статистику по маршруту 
    trans_ctl::BusStat* GetBusStats(const std::string_view& bus_name);

    // Возвращает цвета маршрутов
    std::map<trans_ctl::Bus*, svg::Color, trans_ctl::BusCmp> GetBusesColors();

    // Возвращает остановку 
    trans_ctl::Stop* GetStop(const std::string_view& stop_name) const;

    // Возвращает маршруты, проходящие через
    const std::set<trans_ctl::Bus*, trans_ctl::BusCmp> GetBusesByStop(const std::string_view& stop_name) const;

    // Находит кратчайший маршрут
    std::optional<graph::Router<double>::RouteInfo> FindRoute(const std::string_view& from, const std::string_view& to) const;

    graph::DirectedWeightedGraph<double> GetGraph() const;

    transport_router::EdgeIdtoBus GetEdgeInfo(size_t id) const;

    const render::RenderSettings GetRenderSettings() const;

    svg::Document RenderMap();

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник", "Визуализатор Карты" и "Маршрутизатор"
    trans_ctl::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
    const transport_router::TRouter& router_;
};
