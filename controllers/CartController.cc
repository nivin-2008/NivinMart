#include "CartController.h"
#include <drogon/drogon.h>

// =========================
// ADD TO CART
// =========================
void CartController::addToCart(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto json = req->getJsonObject();

    if (!json ||
        !json->isMember("buyer_id") ||
        !json->isMember("product_id") ||
        !json->isMember("quantity"))
    {
        Json::Value response;
        response["success"] = false;
        response["message"] =
            "buyer_id, product_id and quantity are required";

        callback(HttpResponse::newHttpJsonResponse(response));
        return;
    }

    int buyerId = (*json)["buyer_id"].asInt();
    int productId = (*json)["product_id"].asInt();
    int quantity = (*json)["quantity"].asInt();

    if (quantity <= 0)
    {
        Json::Value response;
        response["success"] = false;
        response["message"] =
            "Quantity must be greater than 0";

        callback(HttpResponse::newHttpJsonResponse(response));
        return;
    }

    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "INSERT INTO cart (buyer_id) VALUES ($1) "
        "ON CONFLICT (buyer_id) DO NOTHING",

        [callback, buyerId, productId, quantity, dbClient]
        (const drogon::orm::Result &)
        {
            dbClient->execSqlAsync(
                "INSERT INTO cart_items "
                "(cart_id, product_id, quantity) "
                "SELECT id, $2, $3 "
                "FROM cart "
                "WHERE buyer_id = $1 "
                "ON CONFLICT (cart_id, product_id) "
                "DO UPDATE SET quantity = "
                "cart_items.quantity + EXCLUDED.quantity",

                [callback](const drogon::orm::Result &)
                {
                    Json::Value response;
                    response["success"] = true;
                    response["message"] =
                        "Product added to cart";

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

                buyerId,
                productId,
                quantity);
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

        buyerId);
}


// =========================
// GET CART
// =========================
void CartController::getCart(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto buyerIdText =
        req->getParameter("buyer_id");

    if (buyerIdText.empty())
    {
        Json::Value response;
        response["success"] = false;
        response["message"] =
            "buyer_id is required";

        callback(
            HttpResponse::newHttpJsonResponse(response));
        return;
    }

    int buyerId;

    try
    {
        buyerId = std::stoi(buyerIdText);
    }
    catch (...)
    {
        Json::Value response;
        response["success"] = false;
        response["message"] =
            "Invalid buyer_id";

        callback(
            HttpResponse::newHttpJsonResponse(response));
        return;
    }

    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "SELECT ci.product_id, "
        "p.name, "
        "p.price, "
        "ci.quantity "
        "FROM cart c "
        "JOIN cart_items ci "
        "ON c.id = ci.cart_id "
        "JOIN products p "
        "ON p.id = ci.product_id "
        "WHERE c.buyer_id = $1 "
        "ORDER BY ci.id",

        [callback](const drogon::orm::Result &result)
        {
            Json::Value response;
            response["success"] = true;
            response["items"] =
                Json::arrayValue;

            for (const auto &row : result)
            {
                Json::Value item;

                item["product_id"] =
                    row["product_id"].as<int>();

                item["name"] =
                    row["name"].as<std::string>();

                item["price"] =
                    row["price"].as<double>();

                item["quantity"] =
                    row["quantity"].as<int>();

                response["items"].append(item);
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

        buyerId);
}


// =========================
// UPDATE CART QUANTITY
// =========================
void CartController::updateCart(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto json = req->getJsonObject();

    if (!json ||
        !json->isMember("buyer_id") ||
        !json->isMember("product_id") ||
        !json->isMember("quantity"))
    {
        Json::Value response;
        response["success"] = false;
        response["message"] =
            "buyer_id, product_id and quantity are required";

        callback(
            HttpResponse::newHttpJsonResponse(response));
        return;
    }

    int buyerId =
        (*json)["buyer_id"].asInt();

    int productId =
        (*json)["product_id"].asInt();

    int quantity =
        (*json)["quantity"].asInt();

    if (quantity <= 0)
    {
        Json::Value response;
        response["success"] = false;
        response["message"] =
            "Quantity must be greater than 0";

        callback(
            HttpResponse::newHttpJsonResponse(response));
        return;
    }

    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "UPDATE cart_items ci "
        "SET quantity = $3 "
        "FROM cart c "
        "WHERE ci.cart_id = c.id "
        "AND c.buyer_id = $1 "
        "AND ci.product_id = $2",

        [callback](const drogon::orm::Result &)
        {
            Json::Value response;
            response["success"] = true;
            response["message"] =
                "Cart quantity updated";

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

        buyerId,
        productId,
        quantity);
}


// =========================
// REMOVE FROM CART
// =========================
void CartController::removeFromCart(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto json = req->getJsonObject();

    if (!json ||
        !json->isMember("buyer_id") ||
        !json->isMember("product_id"))
    {
        Json::Value response;
        response["success"] = false;
        response["message"] =
            "buyer_id and product_id are required";

        callback(
            HttpResponse::newHttpJsonResponse(response));
        return;
    }

    int buyerId =
        (*json)["buyer_id"].asInt();

    int productId =
        (*json)["product_id"].asInt();

    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "DELETE FROM cart_items ci "
        "USING cart c "
        "WHERE ci.cart_id = c.id "
        "AND c.buyer_id = $1 "
        "AND ci.product_id = $2",

        [callback](const drogon::orm::Result &)
        {
            Json::Value response;
            response["success"] = true;
            response["message"] =
                "Product removed from cart";

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

        buyerId,
        productId);
}