#include "ProductController.h"
#include <drogon/drogon.h>

void ProductController::getProducts(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "SELECT id, name, description, price, stock, category_id "
        "FROM products ORDER BY id",

        [callback](const drogon::orm::Result &result)
        {
            Json::Value responseJson;
            responseJson["success"] = true;
            responseJson["products"] = Json::arrayValue;

            for (const auto &row : result)
            {
                Json::Value product;

                product["id"] = row["id"].as<int>();
                product["name"] = row["name"].as<std::string>();

                product["description"] =
                    row["description"].isNull()
                    ? ""
                    : row["description"].as<std::string>();

                product["price"] = row["price"].as<double>();
                product["stock"] = row["stock"].as<int>();

                if (row["category_id"].isNull())
                    product["category_id"] = Json::nullValue;
                else
                    product["category_id"] =
                        row["category_id"].as<int>();

                responseJson["products"].append(product);
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
        });
}


void ProductController::addProduct(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto json = req->getJsonObject();

    if (!json ||
        !json->isMember("seller_id") ||
        !json->isMember("name") ||
        !json->isMember("price") ||
        !json->isMember("stock"))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["message"] =
            "seller_id, name, price and stock are required";

        callback(
            HttpResponse::newHttpJsonResponse(responseJson));
        return;
    }

    int sellerId = (*json)["seller_id"].asInt();
    std::string name = (*json)["name"].asString();

    std::string description =
        json->isMember("description")
        ? (*json)["description"].asString()
        : "";

    double price = (*json)["price"].asDouble();
    int stock = (*json)["stock"].asInt();

    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "INSERT INTO products "
        "(seller_id, name, description, price, stock) "
        "VALUES ($1, $2, $3, $4, $5) "
        "RETURNING id, name, description, price, stock",

        [callback](const drogon::orm::Result &result)
        {
            Json::Value responseJson;
            responseJson["success"] = true;
            responseJson["message"] =
                "Product added successfully";

            responseJson["product"]["id"] =
                result[0]["id"].as<int>();

            responseJson["product"]["name"] =
                result[0]["name"].as<std::string>();

            responseJson["product"]["description"] =
                result[0]["description"].as<std::string>();

            responseJson["product"]["price"] =
                result[0]["price"].as<double>();

            responseJson["product"]["stock"] =
                result[0]["stock"].as<int>();

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

        sellerId,
        name,
        description,
        price,
        stock);
}


void ProductController::updateProduct(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    int id)
{
    auto json = req->getJsonObject();

    if (!json ||
        !json->isMember("name") ||
        !json->isMember("price") ||
        !json->isMember("stock"))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["message"] =
            "name, price and stock are required";

        callback(
            HttpResponse::newHttpJsonResponse(responseJson));
        return;
    }

    std::string name = (*json)["name"].asString();

    std::string description =
        json->isMember("description")
        ? (*json)["description"].asString()
        : "";

    double price = (*json)["price"].asDouble();
    int stock = (*json)["stock"].asInt();

    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "UPDATE products "
        "SET name = $1, description = $2, price = $3, stock = $4 "
        "WHERE id = $5 "
        "RETURNING id, name, description, price, stock",

        [callback](const drogon::orm::Result &result)
        {
            if (result.empty())
            {
                Json::Value responseJson;
                responseJson["success"] = false;
                responseJson["message"] = "Product not found";

                callback(
                    HttpResponse::newHttpJsonResponse(responseJson));
                return;
            }

            Json::Value responseJson;
            responseJson["success"] = true;
            responseJson["message"] =
                "Product updated successfully";

            responseJson["product"]["id"] =
                result[0]["id"].as<int>();

            responseJson["product"]["name"] =
                result[0]["name"].as<std::string>();

            responseJson["product"]["description"] =
                result[0]["description"].isNull()
                ? ""
                : result[0]["description"].as<std::string>();

            responseJson["product"]["price"] =
                result[0]["price"].as<double>();

            responseJson["product"]["stock"] =
                result[0]["stock"].as<int>();

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

        name,
        description,
        price,
        stock,
        id);
}


void ProductController::deleteProduct(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    int id)
{
    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "DELETE FROM products "
        "WHERE id = $1 "
        "RETURNING id",

        [callback](const drogon::orm::Result &result)
        {
            if (result.empty())
            {
                Json::Value responseJson;
                responseJson["success"] = false;
                responseJson["message"] = "Product not found";

                callback(
                    HttpResponse::newHttpJsonResponse(responseJson));
                return;
            }

            Json::Value responseJson;
            responseJson["success"] = true;
            responseJson["message"] =
                "Product deleted successfully";

            responseJson["product_id"] =
                result[0]["id"].as<int>();

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

        id);
}
void ProductController::getSellerProducts(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    std::string sellerId =
        req->getParameter("seller_id");

    if (sellerId.empty())
    {
        Json::Value response;
        response["success"] = false;
        response["message"] =
            "seller_id is required";

        callback(
            HttpResponse::newHttpJsonResponse(response));
        return;
    }

    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "SELECT id, name, description, price, stock, category_id "
        "FROM products "
        "WHERE seller_id = $1 "
        "ORDER BY id DESC",

        [callback](const drogon::orm::Result &result)
        {
            Json::Value response;
            response["success"] = true;
            response["products"] = Json::arrayValue;

            for (const auto &row : result)
            {
                Json::Value product;

                product["id"] =
                    row["id"].as<int>();

                product["name"] =
                    row["name"].as<std::string>();

                product["description"] =
                    row["description"].isNull()
                    ? ""
                    : row["description"].as<std::string>();

                product["price"] =
                    row["price"].as<double>();

                product["stock"] =
                    row["stock"].as<int>();

                if (row["category_id"].isNull())
                {
                    product["category_id"] =
                        Json::nullValue;
                }
                else
                {
                    product["category_id"] =
                        row["category_id"].as<int>();
                }

                response["products"].append(product);
            }

            callback(
                HttpResponse::newHttpJsonResponse(response));
        },

        [callback](const drogon::orm::DrogonDbException &e)
        {
            Json::Value response;
            response["success"] = false;
            response["message"] =
                "Database error";

            LOG_ERROR << e.base().what();

            callback(
                HttpResponse::newHttpJsonResponse(response));
        },

        sellerId);
}