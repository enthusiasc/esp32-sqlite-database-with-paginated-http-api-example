#include "WirelessConnectionManager/WirelessConnectionManager.h"
#include "PersistenceManager/PersistenceManager.h"
#include "ApiController/ApiController.h"

// create all components
WirelessConnectionManager wirelessConnectionManager;
PersistenceManager persistenceManager;
SensorDataStreamer sensorDataStreamer(persistenceManager);
ApiController apiController(sensorDataStreamer);

void setup()
{
  // enable serial interface for logging
  Serial.begin(9600);

  // initialize all components
  wirelessConnectionManager.initialize();
  persistenceManager.initialize();
  apiController.initialize();
}

void loop()
{
  // run all components
  wirelessConnectionManager.run();
  persistenceManager.run();
  delay(10);
}