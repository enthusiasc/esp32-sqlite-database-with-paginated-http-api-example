#ifndef PERSISTENCEMANAGER_H_INCLUDED
#define PERSISTENCEMANAGER_H_INCLUDED
#include "SD.h"
#include "sqlite3.h"
#include <ESPAsyncWebServer.h>

#define ITEMS_TOTAL 8192 // max database result rows that we can ever hold in memory at once (query historic sensor data)

class PersistenceManager
{
public:
    void initialize();
    void run();
    void persistSensorData();
    void queryHistoricSensorData(AsyncWebServerRequest *request);
    int (&getSensorTimestamp())[ITEMS_TOTAL] { return sensor_timestamp; }
    float (&getSensorValue())[ITEMS_TOTAL] { return sensor_value; }
    int getTotalRows() const { return total_rows; }

private:
    boolean isLocked = false;
    int sensor_timestamp[ITEMS_TOTAL]; // reserved space for db query result (query historic sensor data)
    float sensor_value[ITEMS_TOTAL];   // reserved space for db query result (query historic sensor data)
    int total_rows;                    // holds the total rows of a query result
    void openDatabase(String sensorName);
    void executeSQL(const char *sql, const char *dbName);
    size_t streamChunk(uint8_t *buffer, size_t maxLen, size_t index);
};
#endif // PERSISTENCEMANAGER_H_INCLUDED