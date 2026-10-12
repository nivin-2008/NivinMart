
#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class ProductController
    : public drogon::HttpController<ProductController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(ProductController::getProducts,
                  "/products", Get);

    ADD_METHOD_TO(ProductController::addProduct,
                  "/products", Post);

    ADD_METHOD_TO(ProductController::updateProduct,
                  "/products/{1}", Put);

    ADD_METHOD_TO(ProductController::deleteProduct,
                  "/products/{1}", Delete);

    ADD_METHOD_TO(ProductController::getSellerProducts,
                  "/seller/products", Get);

    METHOD_LIST_END

    void getProducts(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback);

    void addProduct(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback);

    void updateProduct(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback,
        int id);

    void deleteProduct(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback,
        int id);

    void getSellerProducts(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback);
};
