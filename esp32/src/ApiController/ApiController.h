#include "SensorDataStreamer/SensorDataStreamer.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

class ApiController
{
public:
    ApiController(SensorDataStreamer &sensorDataStreamer)
        : sensorDataStreamer(sensorDataStreamer) {}
    void initialize();

private:
    SensorDataStreamer &sensorDataStreamer;
};