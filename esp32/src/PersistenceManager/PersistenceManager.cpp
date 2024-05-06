#include "PersistenceManager.h"
#include <SD.h>
#define SD_CS 5

const unsigned long interval = 60000; // interval of 1 minute in milliseconds (auto-save sensor data)
unsigned long previousMillis = 0;     // needed for 1 minute timer (auto-save sensor data)

sqlite3 *database;
const char *databaseNames[] = {"esp32_chip_temperature",
                               "esp32_system_uptime_millis",
                               "esp32_millivolts_pin"};

void PersistenceManager::initialize()
{

    // initialize micro sd card
    Serial.println("Initializing sd card.");
    SD.begin(SD_CS);

    // create individual database for each sensor key to maintain small db size for best query performance
    Serial.println("Initializing database.");
    for (int i = 0; i < sizeof(databaseNames) / sizeof(databaseNames[0]); i++)
    {
        String sensor = databaseNames[i];

        // create a database with schema
        String createSchema = "CREATE TABLE IF NOT EXISTS ";
        createSchema += sensor;
        createSchema += " (timestamp INTEGER PRIMARY KEY, value REAL);";
        executeSQL(createSchema.c_str(), sensor.c_str());

        // create an index for fast queries
        String createIndex = "CREATE INDEX IF NOT EXISTS index_";
        createIndex += sensor;
        createIndex += " ON ";
        createIndex += sensor;
        createIndex += "  (timestamp);";
        executeSQL(createIndex.c_str(), sensor.c_str());
    }
}

void PersistenceManager::run()
{
    // persist sensor data every minute
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
        if (!isLocked)
        {
            isLocked = true;
            persistSensorData();
            previousMillis = currentMillis;
            isLocked = false;
        }
    }
}

void PersistenceManager::persistSensorData()
{
    Serial.println("Persisting sensor data to database...");

    time_t now;
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
    }

    time(&now);
    char sql[1024];

    // insert sensor values
    float esp_temperature = temperatureRead();
    sprintf(sql, "INSERT INTO esp32_chip_temperature (timestamp, value) VALUES (%lu, %.2f);", (unsigned long)now, esp_temperature);
    executeSQL(sql, "esp32_chip_temperature");

    unsigned long esp32_uptime = millis();
    sprintf(sql, "INSERT INTO esp32_system_uptime_millis (timestamp, value) VALUES (%lu, %lu);", (unsigned long)now, esp32_uptime);
    executeSQL(sql, "esp32_system_uptime_millis");

    uint32_t esp_millivolts = analogReadMilliVolts(35);
    sprintf(sql, "INSERT INTO esp32_millivolts_pin (timestamp, value) VALUES (%lu, %i);", (unsigned long)now, esp_millivolts);
    executeSQL(sql, "esp32_millivolts_pin");
}

void PersistenceManager::queryHistoricSensorData(AsyncWebServerRequest *request)
{
    if (isLocked)
    {
        throw std::runtime_error("Database is currently locked by another process. Try again.");
        return;
    }
    isLocked = true;
    Serial.println("Query historic sensor data.");

    // Read params from AsyncWebServerRequest
    String sensorName;
    unsigned long fromTimestamp;
    unsigned long toTimestamp;
    unsigned long limit;
    unsigned long offset;

    // Attempt to read sensorName from the request
    if (request->hasParam("sensorName"))
    {
        AsyncWebParameter *p = request->getParam("sensorName");
        sensorName = p->value();
    }
    else
    {
        throw std::runtime_error("sensorName parameter is missing");
    }

    // Attempt to read fromTimestamp from the request
    if (request->hasParam("fromTimestamp"))
    {
        AsyncWebParameter *p = request->getParam("fromTimestamp");
        fromTimestamp = p->value().toInt();
    }
    else
    {
        throw std::runtime_error("fromTimestamp parameter is missing");
    }

    // Attempt to read toTimestamp from the request
    if (request->hasParam("toTimestamp"))
    {
        AsyncWebParameter *p = request->getParam("toTimestamp");
        toTimestamp = p->value().toInt();
    }
    else
    {
        throw std::runtime_error("toTimestamp parameter is missing");
    }

    // Attempt to read limit from the request
    if (request->hasParam("limit"))
    {
        AsyncWebParameter *p = request->getParam("limit");
        limit = p->value().toInt();
        if (limit > 8192)
        {
            throw std::runtime_error("limit parameter exceeds its maximum size of 8192");
        }
    }
    else
    {
        throw std::runtime_error("limit parameter is missing");
    }

    // Attempt to read offset from the request
    if (request->hasParam("offset"))
    {
        AsyncWebParameter *p = request->getParam("offset");
        offset = p->value().toInt();
    }
    else
    {
        throw std::runtime_error("offset parameter is missing");
    }

    // prepare the SQL query
    char sql[256];
    sprintf(sql, "SELECT timestamp, value FROM %s WHERE timestamp >= %lu AND timestamp <= %lu LIMIT %lu OFFSET %lu;", sensorName.c_str(), fromTimestamp, toTimestamp, limit, offset);

    // execute the SQL query
    openDatabase(sensorName);
    sqlite3_stmt *stmt;
    int ret = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK)
    {
        Serial.printf("Cannot prepare statement: %s\n", sqlite3_errmsg(database));
        sqlite3_close(database);
        throw std::runtime_error("Cannot prepare statement");
    }

    // reset the reserved memory
    memset(sensor_timestamp, 0, sizeof(sensor_timestamp));
    memset(sensor_value, 0, sizeof(sensor_value));

    // step through database result rows
    total_rows = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        // read database columns into reserved memory
        sensor_timestamp[total_rows] = sqlite3_column_int(stmt, 0);
        sensor_value[total_rows] = sqlite3_column_double(stmt, 1);
        total_rows++;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(database);
    isLocked = false;

    Serial.println("All rows read from database. Total is: ");
    Serial.println(total_rows);
}

void PersistenceManager::openDatabase(String sensorName)
{
    String databaseLocation = "/sd/sensor_db_" + sensorName + ".db";
    int rc = sqlite3_open(databaseLocation.c_str(), &database);
    if (rc != SQLITE_OK)
    {
        Serial.printf("Cannot open database: %s\n", sqlite3_errmsg(database));
        throw std::runtime_error("Cannot open database");
    }
}

void PersistenceManager::executeSQL(const char *sql, const char *dbName)
{
    String databaseName = dbName;
    openDatabase(dbName);

    char *errorMessage = 0;
    int rc = sqlite3_exec(database, sql, 0, 0, &errorMessage);
    if (rc != SQLITE_OK)
    {
        Serial.printf("SQL error: %s\n", errorMessage);
        sqlite3_free(errorMessage);
    }
    sqlite3_close(database);
}