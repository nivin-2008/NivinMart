#include "OrderController.h"
#include <drogon/drogon.h>


void OrderController::placeOrder(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto json = req->getJsonObject();

    if (!json || !json->isMember("buyer_id"))
    {
        Json::Value response;

        response["success"] = false;
        response["message"] = "buyer_id is required";

        callback(
            HttpResponse::newHttpJsonResponse(response));

        return;
    }

    int buyerId = (*json)["buyer_id"].asInt();

    auto dbClient = app().getDbClient();


    dbClient->execSqlAsync(
        "SELECT ci.product_id, ci.quantity, p.price "
        "FROM cart c "
        "JOIN cart_items ci ON c.id = ci.cart_id "
        "JOIN products p ON p.id = ci.product_id "
        "WHERE c.buyer_id = $1 "
        "ORDER BY ci.id",

        [callback, buyerId, dbClient]
        (const drogon::orm::Result &result)
        {
            if (result.empty())
            {
                Json::Value response;

                response["success"] = false;
                response["message"] = "Cart is empty";

                callback(
                    HttpResponse::newHttpJsonResponse(
                        response));

                return;
            }

            double totalAmount = 0.0;

            for (const auto &row : result)
            {
                double price =
                    row["price"].as<double>();

                int quantity =
                    row["quantity"].as<int>();

                totalAmount += price * quantity;
            }


            dbClient->execSqlAsync(
                "INSERT INTO orders "
                "(buyer_id, total_amount, status) "
                "VALUES ($1, $2, 'Pending') "
                "RETURNING id",

                [callback,
                 buyerId,
                 result,
                 totalAmount,
                 dbClient]
                (const drogon::orm::Result &orderResult)
                {
                    if (orderResult.empty())
                    {
                        Json::Value response;

                        response["success"] = false;
                        response["message"] =
                            "Could not create order";

                        callback(
                            HttpResponse::newHttpJsonResponse(
                                response));

                        return;
                    }

                    int orderId =
                        orderResult[0]["id"].as<int>();


                    auto insertItem =
                        std::make_shared<
                            std::function<void(size_t)>>();


                    *insertItem =
                        [callback,
                         buyerId,
                         result,
                         totalAmount,
                         orderId,
                         dbClient,
                         insertItem]
                        (size_t index)
                    {
                        if (index >= result.size())
                        {
                            dbClient->execSqlAsync(
                                "DELETE FROM cart_items "
                                "WHERE cart_id = "
                                "(SELECT id FROM cart "
                                "WHERE buyer_id = $1)",

                                [callback,
                                 orderId,
                                 totalAmount]
                                (const drogon::orm::Result &)
                                {
                                    Json::Value response;

                                    response["success"] =
                                        true;

                                    response["message"] =
                                        "Order placed successfully";

                                    response["order_id"] =
                                        orderId;

                                    response["total_amount"] =
                                        totalAmount;

                                    callback(
                                        HttpResponse::
                                        newHttpJsonResponse(
                                            response));
                                },

                                [callback]
                                (const drogon::orm::DrogonDbException &e)
                                {
                                    Json::Value response;

                                    response["success"] =
                                        false;

                                    response["message"] =
                                        "Order created but cart could not be cleared";

                                    LOG_ERROR
                                        << e.base().what();

                                    callback(
                                        HttpResponse::
                                        newHttpJsonResponse(
                                            response));
                                },

                                buyerId);

                            return;
                        }


                        const auto &row =
                            result[index];


                        int productId =
                            row["product_id"].as<int>();

                        int quantity =
                            row["quantity"].as<int>();

                        double price =
                            row["price"].as<double>();


                        dbClient->execSqlAsync(
                            "INSERT INTO order_items "
                            "(order_id, product_id, quantity, price) "
                            "VALUES ($1, $2, $3, $4)",

                            [insertItem, index]
                            (const drogon::orm::Result &)
                            {
                                (*insertItem)(index + 1);
                            },

                            [callback]
                            (const drogon::orm::DrogonDbException &e)
                            {
                                Json::Value response;

                                response["success"] =
                                    false;

                                response["message"] =
                                    "Could not save order items";

                                LOG_ERROR
                                    << e.base().what();

                                callback(
                                    HttpResponse::
                                    newHttpJsonResponse(
                                        response));
                            },

                            orderId,
                            productId,
                            quantity,
                            price);
                    };


                    (*insertItem)(0);
                },

                [callback]
                (const drogon::orm::DrogonDbException &e)
                {
                    Json::Value response;

                    response["success"] = false;
                    response["message"] =
                        std::string("Could not create order: ") + e.base().what();

                    LOG_ERROR
                        << e.base().what();

                    callback(
                        HttpResponse::newHttpJsonResponse(
                            response));
                },

                buyerId,
                totalAmount);
        },


        [callback]
        (const drogon::orm::DrogonDbException &e)
        {
            Json::Value response;

            response["success"] = false;
            response["message"] =
                "Could not read cart";

            LOG_ERROR
                << e.base().what();

            callback(
                HttpResponse::newHttpJsonResponse(
                    response));
        },

        buyerId);
}



void OrderController::getOrders(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    std::string buyerId =
        req->getParameter("buyer_id");


    if (buyerId.empty())
    {
        Json::Value response;

        response["success"] = false;
        response["message"] =
            "buyer_id is required";

        callback(
            HttpResponse::newHttpJsonResponse(
                response));

        return;
    }


    auto dbClient = app().getDbClient();


    dbClient->execSqlAsync(
        "SELECT id, total_amount, status, created_at "
        "FROM orders "
        "WHERE buyer_id = $1 "
        "ORDER BY id DESC",

        [callback]
        (const drogon::orm::Result &result)
        {
            Json::Value response;

            response["success"] = true;
            response["orders"] =
                Json::arrayValue;


            for (const auto &row : result)
            {
                Json::Value order;

                order["id"] =
                    row["id"].as<int>();

                order["total_amount"] =
                    row["total_amount"].as<double>();

                order["status"] =
                    row["status"].as<std::string>();

                order["created_at"] =
                    row["created_at"].as<std::string>();

                response["orders"].append(order);
            }


            callback(
                HttpResponse::newHttpJsonResponse(
                    response));
        },


        [callback]
        (const drogon::orm::DrogonDbException &e)
        {
            Json::Value response;

            response["success"] = false;
            response["message"] =
                "Database error";

            LOG_ERROR
                << e.base().what();

            callback(
                HttpResponse::newHttpJsonResponse(
                    response));
        },

        buyerId);
}



void OrderController::updateOrderStatus(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    int id)
{
    auto json = req->getJsonObject();


    if (!json || !json->isMember("status"))
    {
        Json::Value response;

        response["success"] = false;
        response["message"] =
            "Status is required";

        callback(
            HttpResponse::newHttpJsonResponse(
                response));

        return;
    }


    std::string newStatus =
        (*json)["status"].asString();


    if (newStatus != "Pending" &&
        newStatus != "Confirmed" &&
        newStatus != "Shipped" &&
        newStatus != "Delivered")
    {
        Json::Value response;

        response["success"] = false;
        response["message"] =
            "Invalid status";

        callback(
            HttpResponse::newHttpJsonResponse(
                response));

        return;
    }


    auto dbClient = app().getDbClient();


    dbClient->execSqlAsync(
        "SELECT status "
        "FROM orders "
        "WHERE id = $1",

        [callback,
         newStatus,
         id,
         dbClient]
        (const drogon::orm::Result &result)
        {
            if (result.empty())
            {
                Json::Value response;

                response["success"] = false;
                response["message"] =
                    "Order not found";

                callback(
                    HttpResponse::newHttpJsonResponse(
                        response));

                return;
            }


            std::string currentStatus =
                result[0]["status"].as<std::string>();


            bool valid = false;


            if (currentStatus == "Pending" &&
                newStatus == "Confirmed")
            {
                valid = true;
            }
            else if (currentStatus == "Confirmed" &&
                     newStatus == "Shipped")
            {
                valid = true;
            }
            else if (currentStatus == "Shipped" &&
                     newStatus == "Delivered")
            {
                valid = true;
            }


            if (!valid)
            {
                Json::Value response;

                response["success"] = false;
                response["message"] =
                    "Invalid status change";

                callback(
                    HttpResponse::newHttpJsonResponse(
                        response));

                return;
            }


            dbClient->execSqlAsync(
                "UPDATE orders "
                "SET status = $1 "
                "WHERE id = $2 "
                "RETURNING id, status",

                [callback]
                (const drogon::orm::Result &result)
                {
                    Json::Value response;


                    if (result.empty())
                    {
                        response["success"] =
                            false;

                        response["message"] =
                            "Could not update status";
                    }
                    else
                    {
                        response["success"] =
                            true;

                        response["message"] =
                            "Order status updated successfully";

                        response["order_id"] =
                            result[0]["id"].as<int>();

                        response["status"] =
                            result[0]["status"].as<std::string>();
                    }


                    callback(
                        HttpResponse::newHttpJsonResponse(
                            response));
                },


                [callback]
                (const drogon::orm::DrogonDbException &e)
                {
                    Json::Value response;

                    response["success"] = false;
                    response["message"] =
                        "Database error";

                    LOG_ERROR
                        << e.base().what();

                    callback(
                        HttpResponse::newHttpJsonResponse(
                            response));
                },


                newStatus,
                id);
        },


        [callback]
        (const drogon::orm::DrogonDbException &e)
        {
            Json::Value response;

            response["success"] = false;
            response["message"] =
                "Database error";

            LOG_ERROR
                << e.base().what();

            callback(
                HttpResponse::newHttpJsonResponse(
                    response));
        },

        id);
}



void OrderController::getSellerOrders(
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
            HttpResponse::newHttpJsonResponse(
                response));

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
        "JOIN order_items oi "
        "ON o.id = oi.order_id "
        "JOIN products p "
        "ON oi.product_id = p.id "
        "WHERE p.seller_id = $1 "
        "ORDER BY o.id DESC",

        [callback]
        (const drogon::orm::Result &result)
        {
            Json::Value response;

            response["success"] = true;
            response["orders"] =
                Json::arrayValue;


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

                response["orders"].append(order);
            }


            callback(
                HttpResponse::newHttpJsonResponse(
                    response));
        },


        [callback]
        (const drogon::orm::DrogonDbException &e)
        {
            Json::Value response;

            response["success"] = false;
            response["message"] =
                "Database error";

            LOG_ERROR
                << e.base().what();

            callback(
                HttpResponse::newHttpJsonResponse(
                    response));
        },

        sellerId);
}



void OrderController::getOrderItems(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    int id)
{
    auto dbClient = app().getDbClient();


    if (!dbClient)
    {
        Json::Value response;

        response["success"] = false;
        response["message"] =
            "Database connection not available";

        callback(
            HttpResponse::newHttpJsonResponse(
                response));

        return;
    }


    dbClient->execSqlAsync(
        "SELECT "
        "oi.product_id, "
        "p.name, "
        "oi.quantity, "
        "oi.price "
        "FROM order_items oi "
        "JOIN products p "
        "ON oi.product_id = p.id "
        "WHERE oi.order_id = $1 "
        "ORDER BY oi.id",

        [callback]
        (const drogon::orm::Result &result)
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

                item["product_name"] =
                    row["name"].as<std::string>();

                item["quantity"] =
                    row["quantity"].as<int>();

                item["price"] =
                    row["price"].as<double>();

                response["items"].append(item);
            }


            callback(
                HttpResponse::newHttpJsonResponse(
                    response));
        },


        [callback]
        (const drogon::orm::DrogonDbException &e)
        {
            Json::Value response;

            response["success"] = false;
            response["message"] =
                "Database error";

            LOG_ERROR
                << e.base().what();

            callback(
                HttpResponse::newHttpJsonResponse(
                    response));
        },

        id);
}