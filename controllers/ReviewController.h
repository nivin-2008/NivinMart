#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class ReviewController
    : public drogon::HttpController<ReviewController>
{
public:

    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        ReviewController::addReview,
        "/reviews",
        Post);

    ADD_METHOD_TO(
        ReviewController::getProductReviews,
        "/reviews",
        Get);

    METHOD_LIST_END

    void addReview(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback);

    void getProductReviews(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback);
};