#include "AdminController.h"
#include <drogon/drogon.h>

void AdminController::getUsers(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "SELECT id, name, email, role "
        "FROM users "
        "ORDER BY id",

        [callback](const drogon::orm::Result& result)
        {
            Json::Value responseJson;
            Json::Value users(Json::arrayValue);

            for (const auto& row : result)
            {
                Json::Value user;

                user["id"] = row["id"].as<int>();
                user["name"] = row["name"].as<std::string>();
                user["email"] = row["email"].as<std::string>();
                user["role"] = row["role"].as<std::string>();

                users.append(user);
            }

            responseJson["success"] = true;
            responseJson["users"] = users;

            callback(
                HttpResponse::newHttpJsonResponse(responseJson)
            );
        },

        [callback](const drogon::orm::DrogonDbException& e)
        {
            Json::Value responseJson;

            responseJson["success"] = false;
            responseJson["message"] = "Failed to load users";

            LOG_ERROR << "Admin get users error: "
                      << e.base().what();

            callback(
                HttpResponse::newHttpJsonResponse(responseJson)
            );
        }
    );
}


void AdminController::getOrders(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "SELECT id, buyer_id, status, total_amount "
        "FROM orders "
        "ORDER BY id DESC",

        [callback](const drogon::orm::Result& result)
        {
            Json::Value responseJson;
            Json::Value orders(Json::arrayValue);

            for (const auto& row : result)
            {
                Json::Value order;

                order["id"] = row["id"].as<int>();
                order["buyer_id"] =
                    row["buyer_id"].as<int>();
                order["status"] =
                    row["status"].as<std::string>();
                order["total_amount"] =
                    row["total_amount"].as<double>();

                orders.append(order);
            }

            responseJson["success"] = true;
            responseJson["orders"] = orders;

            callback(
                HttpResponse::newHttpJsonResponse(responseJson)
            );
        },

        [callback](const drogon::orm::DrogonDbException& e)
        {
            Json::Value responseJson;

            responseJson["success"] = false;
            responseJson["message"] = "Failed to load orders";

            LOG_ERROR << "Admin get orders error: "
                      << e.base().what();

            callback(
                HttpResponse::newHttpJsonResponse(responseJson)
            );
        }
    );
}


void AdminController::deleteProduct(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    int id)
{
    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "DELETE FROM products "
        "WHERE id = $1 "
        "RETURNING id",

        [callback](const drogon::orm::Result& result)
        {
            Json::Value responseJson;

            if (result.size() == 1)
            {
                responseJson["success"] = true;
                responseJson["message"] =
                    "Product deleted successfully";

                responseJson["id"] =
                    result[0]["id"].as<int>();
            }
            else
            {
                responseJson["success"] = false;
                responseJson["message"] =
                    "Product not found";
            }

            callback(
                HttpResponse::newHttpJsonResponse(responseJson)
            );
        },

        [callback](const drogon::orm::DrogonDbException& e)
        {
            Json::Value responseJson;

            responseJson["success"] = false;
            responseJson["message"] =
                "Failed to delete product";

            LOG_ERROR << "Admin delete product error: "
                      << e.base().what();

            callback(
                HttpResponse::newHttpJsonResponse(responseJson)
            );
        },

        id
    );
}