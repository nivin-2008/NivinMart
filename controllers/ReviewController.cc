#include "ReviewController.h"
#include <drogon/drogon.h>
#include <string>
#include <stdexcept>

void ReviewController::addReview(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto json = req->getJsonObject();

    Json::Value response;

    if (!json ||
        !json->isMember("buyer_id") ||
        !json->isMember("product_id") ||
        !json->isMember("rating") ||
        !json->isMember("comment"))
    {
        response["success"] = false;
        response["message"] = "All review fields are required";

        callback(HttpResponse::newHttpJsonResponse(response));
        return;
    }

    if (!(*json)["buyer_id"].isInt() ||
        !(*json)["product_id"].isInt() ||
        !(*json)["rating"].isInt())
    {
        response["success"] = false;
        response["message"] = "Invalid review details";

        callback(HttpResponse::newHttpJsonResponse(response));
        return;
    }

    int buyerId = (*json)["buyer_id"].asInt();
    int productId = (*json)["product_id"].asInt();
    int rating = (*json)["rating"].asInt();
    std::string comment = (*json)["comment"].asString();

    if (buyerId <= 0 || productId <= 0 ||
        rating < 1 || rating > 5)
    {
        response["success"] = false;
        response["message"] =
            "Buyer, product or rating is invalid";

        callback(HttpResponse::newHttpJsonResponse(response));
        return;
    }

    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "INSERT INTO reviews "
        "(buyer_id, product_id, rating, comment) "
        "SELECT $1, $2, $3, $4 "
        "WHERE EXISTS ("
        " SELECT 1 "
        " FROM orders o "
        " JOIN order_items oi ON oi.order_id = o.id "
        " WHERE o.buyer_id = $1 "
        " AND oi.product_id = $2 "
        " AND o.status = 'Delivered' "
        ") "
        "RETURNING id, buyer_id, product_id, rating, "
        "comment, created_at",

        [callback](const drogon::orm::Result& result)
        {
            Json::Value response;

            if (result.size() == 0)
            {
                response["success"] = false;
                response["message"] =
                    "You can review this product only after your order is delivered.";
            }
            else
            {
                response["success"] = true;
                response["message"] =
                    "Review submitted successfully";

                response["id"] =
                    result[0]["id"].as<int>();

                response["buyer_id"] =
                    result[0]["buyer_id"].as<int>();

                response["product_id"] =
                    result[0]["product_id"].as<int>();

                response["rating"] =
                    result[0]["rating"].as<int>();

                response["comment"] =
                    result[0]["comment"].as<std::string>();

                response["created_at"] =
                    result[0]["created_at"].as<std::string>();
            }

            callback(HttpResponse::newHttpJsonResponse(response));
        },

        [callback](const drogon::orm::DrogonDbException& e)
        {
            Json::Value response;
            response["success"] = false;
            response["message"] = "Database error";

            LOG_ERROR << "Review error: " << e.base().what();

            callback(HttpResponse::newHttpJsonResponse(response));
        },

        buyerId,
        productId,
        rating,
        comment);
}

void ReviewController::getProductReviews(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value response;

    std::string productIdText =
        req->getParameter("product_id");

    if (productIdText.empty())
    {
        productIdText = req->getParameter("productId");
    }

    int productId;

    try
    {
        std::size_t position = 0;

        productId = std::stoi(productIdText, &position);

        if (position != productIdText.size() || productId <= 0)
        {
            throw std::invalid_argument("Invalid product ID");
        }
    }
    catch (...)
    {
        response["success"] = false;
        response["message"] = "Valid product_id is required";

        callback(HttpResponse::newHttpJsonResponse(response));
        return;
    }

    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "SELECT r.id, r.buyer_id, u.name AS buyer_name, "
        "r.product_id, r.rating, r.comment, r.created_at "
        "FROM reviews r "
        "JOIN users u ON u.id = r.buyer_id "
        "WHERE r.product_id = $1 "
        "ORDER BY r.created_at DESC",

        [callback](const drogon::orm::Result& result)
        {
            Json::Value response;
            Json::Value reviews(Json::arrayValue);

            for (const auto& row : result)
            {
                Json::Value review;

                review["id"] = row["id"].as<int>();
                review["buyer_id"] =
                    row["buyer_id"].as<int>();

                review["buyer_name"] =
                    row["buyer_name"].as<std::string>();

                review["product_id"] =
                    row["product_id"].as<int>();

                review["rating"] =
                    row["rating"].as<int>();

                review["comment"] =
                    row["comment"].as<std::string>();

                review["created_at"] =
                    row["created_at"].as<std::string>();

                reviews.append(review);
            }

            response["success"] = true;
            response["reviews"] = reviews;

            callback(HttpResponse::newHttpJsonResponse(response));
        },

        [callback](const drogon::orm::DrogonDbException& e)
        {
            Json::Value response;
            response["success"] = false;
            response["message"] = "Database error";

            LOG_ERROR << "Get reviews error: " << e.base().what();

            callback(HttpResponse::newHttpJsonResponse(response));
        },

        productId);
}