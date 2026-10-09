#include "CategoryController.h"
#include <drogon/drogon.h>

void CategoryController::createCategory(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto json = req->getJsonObject();

    if (!json || !json->isMember("name"))
    {
        Json::Value response;
        response["success"] = false;
        response["message"] = "name is required";

        callback(HttpResponse::newHttpJsonResponse(response));
        return;
    }

    std::string name = (*json)["name"].asString();

    if (name.empty())
    {
        Json::Value response;
        response["success"] = false;
        response["message"] = "Category name cannot be empty";

        callback(HttpResponse::newHttpJsonResponse(response));
        return;
    }

    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "INSERT INTO categories (name) "
        "VALUES ($1) "
        "RETURNING id, name",

        [callback](const drogon::orm::Result &result)
        {
            Json::Value response;
            response["success"] = true;
            response["message"] = "Category created successfully";
            response["category"]["id"] =
                result[0]["id"].as<int>();
            response["category"]["name"] =
                result[0]["name"].as<std::string>();

            callback(
                HttpResponse::newHttpJsonResponse(response));
        },

        [callback](const drogon::orm::DrogonDbException &e)
        {
            Json::Value response;
            response["success"] = false;
            response["message"] = "Category already exists or database error";

            LOG_ERROR << e.base().what();

            callback(
                HttpResponse::newHttpJsonResponse(response));
        },

        name);
}


void CategoryController::getCategories(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "SELECT id, name "
        "FROM categories "
        "ORDER BY id",

        [callback](const drogon::orm::Result &result)
        {
            Json::Value response;
            response["success"] = true;
            response["categories"] = Json::arrayValue;

            for (const auto &row : result)
            {
                Json::Value category;

                category["id"] =
                    row["id"].as<int>();

                category["name"] =
                    row["name"].as<std::string>();

                response["categories"].append(category);
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
        });
}