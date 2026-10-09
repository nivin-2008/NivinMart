#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class SellerOrderController
    : public drogon::HttpController<SellerOrderController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        SellerOrderController::getOrders,
        "/seller/orders",
        Get);

    ADD_METHOD_TO(
        SellerOrderController::updateOrderStatus,
        "/seller/orders/{1}/status",
        Put);

    METHOD_LIST_END

    void getOrders(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback);

    void updateOrderStatus(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback,
        int orderId);
};