#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class CategoryController
    : public drogon::HttpController<CategoryController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(CategoryController::createCategory, "/categories", Post);
    ADD_METHOD_TO(CategoryController::getCategories, "/categories", Get);

    METHOD_LIST_END

    void createCategory(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback);

    void getCategories(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback);
};