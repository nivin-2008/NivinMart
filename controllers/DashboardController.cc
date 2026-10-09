#include "DashboardController.h"
#include <drogon/drogon.h>

void DashboardController::getDashboard(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    std::string sellerId = req->getParameter("seller_id");

    if (sellerId.empty())
    {
        Json::Value response;
        response["success"] = false;
        response["message"] = "seller_id is required";

        callback(HttpResponse::newHttpJsonResponse(response));
        return;
    }

    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "SELECT "
        "(SELECT COUNT(*) "
        " FROM products "
        " WHERE seller_id = $1) AS total_products, "

        "(SELECT COUNT(DISTINCT o.id) "
        " FROM orders o "
        " JOIN order_items oi ON o.id = oi.order_id "
        " JOIN products p ON p.id = oi.product_id "
        " WHERE p.seller_id = $1) AS total_orders, "

        "(SELECT COUNT(DISTINCT o.id) "
        " FROM orders o "
        " JOIN order_items oi ON o.id = oi.order_id "
        " JOIN products p ON p.id = oi.product_id "
        " WHERE p.seller_id = $1 "
        " AND o.status = 'Pending') AS pending_orders, "

        "(SELECT COALESCE(SUM(oi.price * oi.quantity), 0) "
        " FROM order_items oi "
        " JOIN products p ON p.id = oi.product_id "
        " JOIN orders o ON o.id = oi.order_id "
        " WHERE p.seller_id = $1 "
        " AND o.status <> 'Cancelled') AS total_sales",

        [callback](const drogon::orm::Result &result)
        {
            Json::Value response;
            response["success"] = true;

            if (!result.empty())
            {
                response["total_products"] =
                    result[0]["total_products"].as<int>();

                response["total_orders"] =
                    result[0]["total_orders"].as<int>();

                response["pending_orders"] =
                    result[0]["pending_orders"].as<int>();

                response["total_sales"] =
                    result[0]["total_sales"].as<double>();
            }

            callback(
                HttpResponse::newHttpJsonResponse(response));
        },

        [callback](const drogon::orm::DrogonDbException &e)
        {
            Json::Value response;
            response["success"] = false;
            response["message"] = "Database error";

            LOG_ERROR << e.base().what();

            callback(
                HttpResponse::newHttpJsonResponse(response));
        },

        sellerId);
}