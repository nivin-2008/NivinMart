#include <drogon/drogon.h>
#include <cstdlib>
#include <string>

int main()
{
    const char *portValue = std::getenv("PORT");
    const unsigned short port = portValue
        ? static_cast<unsigned short>(std::stoi(portValue))
        : 5555;

    const char *dbHost = std::getenv("DB_HOST");
    const char *dbName = std::getenv("DB_NAME");
    const char *dbUser = std::getenv("DB_USER");
    const char *dbPassword = std::getenv("DB_PASSWORD");
    const char *dbPortValue = std::getenv("DB_PORT");

    auto &app = drogon::app();
    app.addListener("0.0.0.0", port)
       .setDocumentRoot("./frontend");

    // Configure PostgreSQL only when deployment database variables exist.
    // Set these in the hosting dashboard; never commit credentials.
    if (dbHost && dbName && dbUser && dbPassword)
    {
        const unsigned short dbPort = dbPortValue
            ? static_cast<unsigned short>(std::stoi(dbPortValue))
            : 5432;

        app.createDbClient(
            "postgresql",
            dbHost,
            dbPort,
            dbName,
            dbUser,
            dbPassword,
            4);

        // Ensure the tables required by Place Order exist in this database.
        // IF NOT EXISTS preserves existing order data and existing tables.
        auto dbClient = app.getDbClient();
        dbClient->execSqlSync(
            "CREATE TABLE IF NOT EXISTS orders ("
            "id SERIAL PRIMARY KEY, "
            "buyer_id INTEGER NOT NULL, "
            "total_amount NUMERIC(12,2) NOT NULL, "
            "status VARCHAR(50) NOT NULL DEFAULT 'Pending', "
            "created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP"
            ")");

        dbClient->execSqlSync(
            "CREATE TABLE IF NOT EXISTS order_items ("
            "id SERIAL PRIMARY KEY, "
            "order_id INTEGER NOT NULL REFERENCES orders(id) ON DELETE CASCADE, "
            "product_id INTEGER NOT NULL, "
            "quantity INTEGER NOT NULL CHECK (quantity > 0), "
            "price NUMERIC(12,2) NOT NULL"
            ")");
    }

    app.run();
    return 0;
}
