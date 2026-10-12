
#include "ProductController.h"
#include <drogon/drogon.h>
#include <string>

using namespace drogon;
using namespace drogon::orm;

void ProductController::getProducts(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "SELECT id, seller_id, name, description, price, stock, "
        "category_id, image_data FROM products ORDER BY id DESC",

        [callback](const Result& result)
        {
            Json::Value response;
            response["success"] = true;
            response["products"] = Json::arrayValue;

            for (const auto& row : result)
            {
                Json::Value product;
                product["id"] = row["id"].as<int>();
                product["seller_id"] = row["seller_id"].as<int>();
                product["name"] = row["name"].as<std::string>();
                product["description"] = row["description"].isNull()
                    ? "" : row["description"].as<std::string>();
                product["price"] = row["price"].as<double>();
                product["stock"] = row["stock"].as<int>();
                product["image_data"] = row["image_data"].isNull()
                    ? "" : row["image_data"].as<std::string>();

                if (row["category_id"].isNull())
                    product["category_id"] = Json::nullValue;
                else
                    product["category_id"] = row["category_id"].as<int>();

                response["products"].append(product);
            }

            callback(HttpResponse::newHttpJsonResponse(response));
        },

        [callback](const DrogonDbException& e)
        {
            LOG_ERROR << e.base().what();

            Json::Value response;
            response["success"] = false;
            response["message"] = "Database error";

            callback(HttpResponse::newHttpJsonResponse(response));
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
        Json::Value response;
        response["success"] = false;
        response["message"] =
            "seller_id, name, price and stock are required";

        callback(HttpResponse::newHttpJsonResponse(response));
        return;
    }

    int sellerId = (*json)["seller_id"].asInt();
    std::string name = (*json)["name"].asString();
    std::string description = json->isMember("description")
        ? (*json)["description"].asString() : "";
    double price = (*json)["price"].asDouble();
    int stock = (*json)["stock"].asInt();
    std::string imageData = json->isMember("image_data")
        ? (*json)["image_data"].asString() : "";

    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "INSERT INTO products "
        "(seller_id, name, description, price, stock, image_data) "
        "VALUES ($1, $2, $3, $4, $5, $6) "
        "RETURNING id, seller_id, name, description, price, stock, image_data",

        [callback](const Result& result)
        {
            Json::Value response;
            response["success"] = true;
            response["message"] = "Product added successfully";

            const auto& row = result[0];

            response["product"]["id"] = row["id"].as<int>();
            response["product"]["seller_id"] = row["seller_id"].as<int>();
            response["product"]["name"] = row["name"].as<std::string>();
            response["product"]["description"] =
                row["description"].isNull()
                ? "" : row["description"].as<std::string>();
            response["product"]["price"] = row["price"].as<double>();
            response["product"]["stock"] = row["stock"].as<int>();
            response["product"]["image_data"] =
                row["image_data"].isNull()
                ? "" : row["image_data"].as<std::string>();

            callback(HttpResponse::newHttpJsonResponse(response));
        },

        [callback](const DrogonDbException& e)
        {
            LOG_ERROR << e.base().what();

            Json::Value response;
            response["success"] = false;
            response["message"] = "Database error";

            callback(HttpResponse::newHttpJsonResponse(response));
        },

        sellerId, name, description, price, stock, imageData);
}


void ProductController::getSellerProducts(
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
        "SELECT id, seller_id, name, description, price, stock, "
        "category_id, image_data FROM products "
        "WHERE seller_id = $1 ORDER BY id DESC",

        [callback](const Result& result)
        {
            Json::Value response;
            response["success"] = true;
            response["products"] = Json::arrayValue;

            for (const auto& row : result)
            {
                Json::Value product;
                product["id"] = row["id"].as<int>();
                product["seller_id"] = row["seller_id"].as<int>();
                product["name"] = row["name"].as<std::string>();
                product["description"] = row["description"].isNull()
                    ? "" : row["description"].as<std::string>();
                product["price"] = row["price"].as<double>();
                product["stock"] = row["stock"].as<int>();
                product["image_data"] = row["image_data"].isNull()
                    ? "" : row["image_data"].as<std::string>();

                if (row["category_id"].isNull())
                    product["category_id"] = Json::nullValue;
                else
                    product["category_id"] = row["category_id"].as<int>();

                response["products"].append(product);
            }

            callback(HttpResponse::newHttpJsonResponse(response));
        },

        [callback](const DrogonDbException& e)
        {
            LOG_ERROR << e.base().what();

            Json::Value response;
            response["success"] = false;
            response["message"] = "Database error";

            callback(HttpResponse::newHttpJsonResponse(response));
        },

        sellerId);
}


void ProductController::updateProduct(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    int id)
{
    auto json = req->getJsonObject();

    if (!json ||
        !json->isMember("seller_id") ||
        !json->isMember("name") ||
        !json->isMember("price") ||
        !json->isMember("stock"))
    {
        Json::Value response;
        response["success"] = false;
        response["message"] =
            "seller_id, name, price and stock are required";

        callback(HttpResponse::newHttpJsonResponse(response));
        return;
    }

    int sellerId = (*json)["seller_id"].asInt();
    std::string name = (*json)["name"].asString();
    std::string description = json->isMember("description")
        ? (*json)["description"].asString() : "";
    double price = (*json)["price"].asDouble();
    int stock = (*json)["stock"].asInt();
    std::string imageData = json->isMember("image_data")
        ? (*json)["image_data"].asString() : "";

    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "UPDATE products SET name = $1, description = $2, "
        "price = $3, stock = $4, image_data = $5 "
        "WHERE id = $6 AND seller_id = $7 "
        "RETURNING id, seller_id, name, description, price, stock, image_data",

        [callback](const Result& result)
        {
            Json::Value response;

            if (result.empty())
            {
                response["success"] = false;
                response["message"] =
                    "Product not found or seller does not own this product";

                callback(HttpResponse::newHttpJsonResponse(response));
                return;
            }

            const auto& row = result[0];

            response["success"] = true;
            response["message"] = "Product updated successfully";
            response["product"]["id"] = row["id"].as<int>();
            response["product"]["seller_id"] = row["seller_id"].as<int>();
            response["product"]["name"] = row["name"].as<std::string>();
            response["product"]["description"] =
                row["description"].isNull()
                ? "" : row["description"].as<std::string>();
            response["product"]["price"] = row["price"].as<double>();
            response["product"]["stock"] = row["stock"].as<int>();
            response["product"]["image_data"] =
                row["image_data"].isNull()
                ? "" : row["image_data"].as<std::string>();

            callback(HttpResponse::newHttpJsonResponse(response));
        },

        [callback](const DrogonDbException& e)
        {
            LOG_ERROR << e.base().what();

            Json::Value response;
            response["success"] = false;
            response["message"] = "Database error";

            callback(HttpResponse::newHttpJsonResponse(response));
        },

        name, description, price, stock, imageData, id, sellerId);
}


void ProductController::deleteProduct(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    int id)
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
        "DELETE FROM products WHERE id = $1 AND seller_id = $2 "
        "RETURNING id",

        [callback](const Result& result)
        {
            Json::Value response;

            if (result.empty())
            {
                response["success"] = false;
                response["message"] =
                    "Product not found or seller does not own this product";

                callback(HttpResponse::newHttpJsonResponse(response));
                return;
            }

            response["success"] = true;
            response["message"] = "Product deleted successfully";
            response["product_id"] = result[0]["id"].as<int>();

            callback(HttpResponse::newHttpJsonResponse(response));
        },

        [callback](const DrogonDbException& e)
        {
            LOG_ERROR << e.base().what();

            Json::Value response;
            response["success"] = false;
            response["message"] =
                "Database delete failed. Product may be referenced by an order or cart.";

            callback(HttpResponse::newHttpJsonResponse(response));
        },

        id, sellerId);
}
