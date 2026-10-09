#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class OrderController : public drogon::HttpController<OrderController>
{
public:

    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        OrderController::placeOrder,
        "/orders",
        Post
    );

    ADD_METHOD_TO(
        OrderController::getOrders,
        "/orders",
        Get
    );

    ADD_METHOD_TO(
        OrderController::updateOrderStatus,
        "/orders/{1}/status",
        Put
    );

    ADD_METHOD_TO(
        OrderController::getSellerOrders,
        "/seller/orders",
        Get
    );

    ADD_METHOD_TO(
        OrderController::getOrderItems,
        "/orders/{1}/items",
        Get
    );

    METHOD_LIST_END


    void placeOrder(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback
    );


    void getOrders(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback
    );


    void updateOrderStatus(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback,
        int id
    );


    void getSellerOrders(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback
    );


    void getOrderItems(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback,
        int id
    );
};