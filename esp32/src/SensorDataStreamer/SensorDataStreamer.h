#include "PersistenceManager/PersistenceManager.h"
#include <ESPAsyncWebServer.h>

class SensorDataStreamer
{
public:
    SensorDataStreamer(PersistenceManager &persistenceManager)
        : persistenceManager(persistenceManager) {}
    void getHistoricSensorData(AsyncWebServerRequest *request);

private:
    PersistenceManager &persistenceManager;
    size_t streamChunk(uint8_t *buffer, size_t maxLen, size_t index);
};