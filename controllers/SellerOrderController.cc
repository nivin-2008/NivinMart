#include "SellerOrderController.h"
#include <drogon/drogon.h>

void SellerOrderController::getOrders(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    std::string sellerId =
        req->getParameter("seller_id");

    if (sellerId.empty())
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["message"] = "seller_id is required";

        callback(
            HttpResponse::newHttpJsonResponse(responseJson));
        return;
    }

    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "SELECT DISTINCT "
        "o.id, "
        "o.buyer_id, "
        "o.total_amount, "
        "o.status, "
        "o.created_at "
        "FROM orders o "
        "JOIN order_items oi ON o.id = oi.order_id "
        "JOIN products p ON p.id = oi.product_id "
        "WHERE p.seller_id = $1 "
        "ORDER BY o.id DESC",

        [callback](const drogon::orm::Result &result)
        {
            Json::Value responseJson;
            responseJson["success"] = true;
            responseJson["orders"] = Json::arrayValue;

            for (const auto &row : result)
            {
                Json::Value order;

                order["id"] =
                    row["id"].as<int>();

                order["buyer_id"] =
                    row["buyer_id"].as<int>();

                order["total_amount"] =
                    row["total_amount"].as<double>();

                order["status"] =
                    row["status"].as<std::string>();

                order["created_at"] =
                    row["created_at"].as<std::string>();

                responseJson["orders"].append(order);
            }

            callback(
                HttpResponse::newHttpJsonResponse(responseJson));
        },

        [callback](const drogon::orm::DrogonDbException &e)
        {
            Json::Value responseJson;
            responseJson["success"] = false;
            responseJson["message"] = "Database error";

            LOG_ERROR << e.base().what();

            callback(
                HttpResponse::newHttpJsonResponse(responseJson));
        },

        sellerId);
}


void SellerOrderController::updateOrderStatus(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    int orderId)
{
    auto json = req->getJsonObject();

    if (!json || !json->isMember("status"))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["message"] = "status is required";

        callback(
            HttpResponse::newHttpJsonResponse(responseJson));
        return;
    }

    std::string status =
        (*json)["status"].asString();

    if (status != "Pending" &&
        status != "Confirmed" &&
        status != "Shipped" &&
        status != "Delivered" &&
        status != "Cancelled")
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["message"] = "Invalid status";

        callback(
            HttpResponse::newHttpJsonResponse(responseJson));
        return;
    }

    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "UPDATE orders "
        "SET status = $1 "
        "WHERE id = $2 "
        "RETURNING id, status",

        [callback](const drogon::orm::Result &result)
        {
            if (result.empty())
            {
                Json::Value responseJson;
                responseJson["success"] = false;
                responseJson["message"] = "Order not found";

                callback(
                    HttpResponse::newHttpJsonResponse(responseJson));
                return;
            }

            Json::Value responseJson;
            responseJson["success"] = true;
            responseJson["message"] =
                "Order status updated successfully";

            responseJson["order_id"] =
                result[0]["id"].as<int>();

            responseJson["status"] =
                result[0]["status"].as<std::string>();

            callback(
                HttpResponse::newHttpJsonResponse(responseJson));
        },

        [callback](const drogon::orm::DrogonDbException &e)
        {
            Json::Value responseJson;
            responseJson["success"] = false;
            responseJson["message"] = "Database error";

            LOG_ERROR << e.base().what();

            callback(
                HttpResponse::newHttpJsonResponse(responseJson));
        },

        status,
        orderId);
}