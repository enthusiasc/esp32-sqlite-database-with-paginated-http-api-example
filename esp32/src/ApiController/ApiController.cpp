#include "ApiController.h"
#include "ApiController/ApiSpecification_yaml.h"
#include "ApiController/ApiDocumentation_html.h"

AsyncWebServer server(80);

void ApiController::initialize()
{
  // api endpoint: documentation
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/html", API_DOCUMENTATION_HTML); });
  server.on("/api.yaml", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", API_SPECIFICATION_YAML); });

  // api endpoint: get historic sensor data
  server.on("/api/sensors", HTTP_GET, [this](AsyncWebServerRequest *request)
            { xTaskCreate(
                  [](void *arg)
                  {
                    auto *args = static_cast<std::pair<AsyncWebServerRequest *, SensorDataStreamer *> *>(arg);
                    AsyncWebServerRequest *request = args->first;
                    SensorDataStreamer *sensorDataStreamer = args->second;
                    request->client()->setRxTimeout(180);
                    sensorDataStreamer->getHistoricSensorData(request);
                    delete args;
                    vTaskDelete(NULL);
                  },
                  "sensordatahandler",
                  5000,
                  new std::pair<AsyncWebServerRequest *, SensorDataStreamer *>(request, &sensorDataStreamer),
                  1,
                  NULL); });

  server.begin();
}