#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class AdminController
    : public drogon::HttpController<AdminController>
{
public:

    METHOD_LIST_BEGIN

    // Admin - view all users
    ADD_METHOD_TO(
        AdminController::getUsers,
        "/admin/users",
        Get);

    // Admin - view all orders
    ADD_METHOD_TO(
        AdminController::getOrders,
        "/admin/orders",
        Get);

    // Admin - delete product
    ADD_METHOD_TO(
        AdminController::deleteProduct,
        "/admin/products/{1}",
        Delete);

    METHOD_LIST_END;


    void getUsers(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback);


    void getOrders(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback);


    void deleteProduct(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback,
        int id);
};