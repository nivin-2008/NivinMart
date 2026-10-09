#include "LoginController.h"
#include <drogon/drogon.h>
#include <sodium.h>
#include <string>

void LoginController::login(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto json = req->getJsonObject();

    if (!json ||
        !json->isMember("email") ||
        !json->isMember("password"))
    {
        Json::Value response;
        response["success"] = false;
        response["message"] = "Email and password are required";
        callback(HttpResponse::newHttpJsonResponse(response));
        return;
    }

    std::string email = (*json)["email"].asString();
    std::string password = (*json)["password"].asString();

    if (sodium_init() < 0)
    {
        Json::Value response;
        response["success"] = false;
        response["message"] = "Password security initialization failed";
        callback(HttpResponse::newHttpJsonResponse(response));
        return;
    }

    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "SELECT id, name, email, role, password "
        "FROM users WHERE email = $1",

        [callback, dbClient, password](
            const drogon::orm::Result& result)
        {
            if (result.size() != 1)
            {
                Json::Value response;
                response["success"] = false;
                response["message"] = "Invalid email or password";
                callback(HttpResponse::newHttpJsonResponse(response));
                return;
            }

            int userId = result[0]["id"].as<int>();
            std::string name = result[0]["name"].as<std::string>();
            std::string userEmail = result[0]["email"].as<std::string>();
            std::string role = result[0]["role"].as<std::string>();
            std::string storedPassword =
                result[0]["password"].as<std::string>();

            auto sendSuccess =
                [callback, userId, name, userEmail, role]()
            {
                Json::Value response;
                response["success"] = true;
                response["message"] = "Login successful";
                response["id"] = userId;
                response["name"] = name;
                response["email"] = userEmail;
                response["role"] = role;

                callback(HttpResponse::newHttpJsonResponse(response));
            };

            auto sendFailure = [callback]()
            {
                Json::Value response;
                response["success"] = false;
                response["message"] = "Invalid email or password";

                callback(HttpResponse::newHttpJsonResponse(response));
            };

            bool isHash =
                storedPassword.rfind("$argon2id$", 0) == 0 ||
                storedPassword.rfind("$argon2i$", 0) == 0;

            if (isHash)
            {
                if (crypto_pwhash_str_verify(
                        storedPassword.c_str(),
                        password.c_str(),
                        static_cast<unsigned long long>(password.size())
                    ) == 0)
                {
                    sendSuccess();
                }
                else
                {
                    sendFailure();
                }

                return;
            }

            // Old account: compare the existing plaintext password.
            if (storedPassword != password)
            {
                sendFailure();
                return;
            }

            char hashedPassword[crypto_pwhash_STRBYTES];

            if (crypto_pwhash_str(
                    hashedPassword,
                    password.c_str(),
                    static_cast<unsigned long long>(password.size()),
                    crypto_pwhash_OPSLIMIT_INTERACTIVE,
                    crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0)
            {
                Json::Value response;
                response["success"] = false;
                response["message"] = "Password hashing failed";

                callback(HttpResponse::newHttpJsonResponse(response));
                return;
            }

            std::string passwordHash = hashedPassword;

            // Send login success only after the database update succeeds.
            dbClient->execSqlAsync(
                "UPDATE users SET password = $1 "
                "WHERE id = $2 AND password = $3 "
                "RETURNING id",

                [callback, userId, name, userEmail, role](
                    const drogon::orm::Result& updateResult)
                {
                    Json::Value response;

                    if (updateResult.size() == 1)
                    {
                        response["success"] = true;
                        response["message"] = "Login successful";
                        response["id"] = userId;
                        response["name"] = name;
                        response["email"] = userEmail;
                        response["role"] = role;
                    }
                    else
                    {
                        response["success"] = false;
                        response["message"] =
                            "Password migration did not complete. Please try login again.";
                    }

                    callback(HttpResponse::newHttpJsonResponse(response));
                },

                [callback](const drogon::orm::DrogonDbException& e)
                {
                    LOG_ERROR << "Password migration error: "
                              << e.base().what();

                    Json::Value response;
                    response["success"] = false;
                    response["message"] =
                        "Could not update password securely";

                    callback(HttpResponse::newHttpJsonResponse(response));
                },

                passwordHash,
                userId,
                storedPassword);
        },

        [callback](const drogon::orm::DrogonDbException& e)
        {
            LOG_ERROR << "Login database error: " << e.base().what();

            Json::Value response;
            response["success"] = false;
            response["message"] = "Database error";

            callback(HttpResponse::newHttpJsonResponse(response));
        },

        email);
}