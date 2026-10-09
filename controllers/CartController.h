#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class CartController : public drogon::HttpController<CartController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(CartController::addToCart, "/cart", Post);
    ADD_METHOD_TO(CartController::getCart, "/cart", Get);
    ADD_METHOD_TO(CartController::updateCart, "/cart/update", Put);
    ADD_METHOD_TO(CartController::removeFromCart, "/cart/remove", Delete);

    METHOD_LIST_END

    void addToCart(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback);

    void getCart(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback);

    void updateCart(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback);

    void removeFromCart(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback);
};