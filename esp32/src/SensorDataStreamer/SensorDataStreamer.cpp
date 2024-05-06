#include "SensorDataStreamer/SensorDataStreamer.h"

int *timestamps;
float *values;
int total_rows;

#define ROWSIZE_MAX 50 // row size in bytes of a single json object (timestamp, value)
uint16_t current_index;
String current_sensor;
bool is_streaming_active = false;

void SensorDataStreamer::getHistoricSensorData(AsyncWebServerRequest *request)
{
    // exit early if there is a pending request on this API endpoint as we do not have enough memory to handle more than one request of this type at a time
    if (is_streaming_active)
    {
        request->send(503, "text/plain", "The server is busy while another sensor data request is already being processed. Try again.");
        return;
    }

    Serial.println("Stream sensor data requested");
    is_streaming_active = true;
    current_index = 0;

    try
    {
        // query the requested sensor data from the database
        persistenceManager.queryHistoricSensorData(request);
        timestamps = persistenceManager.getSensorTimestamp();
        values = persistenceManager.getSensorValue();
        total_rows = persistenceManager.getTotalRows();
    }
    catch (const std::runtime_error &e)
    {
        Serial.println("Error. Aborting sensor data stream.");
        request->send(400, "text/plain", "Error occured: " + String(e.what()));
        is_streaming_active = false;
        return;
    }

    // remember the requested sensor as it is needed while creating the chunked response
    AsyncWebParameter *p = request->getParam("sensorName");
    current_sensor = p->value();

    // start chunked response
    Serial.println("Starting chunked response.");
    request->onDisconnect([this]()
                          { is_streaming_active = false; });
    AsyncWebServerResponse *response = request->beginChunkedResponse("application/json", [this](uint8_t *buffer, size_t maxLen, size_t index)
                                                                     { return this->streamChunk(buffer, maxLen, index); });
    request->send(response);
}

size_t SensorDataStreamer::streamChunk(uint8_t *buffer, size_t maxLen, size_t index)
{
    // half the maxLen of a response chunk to avoid sending large packets
    maxLen = maxLen >> 1;
    // remember the length of the current chunk
    size_t len = 0;

    // if we are in the first chunk, add json structure to the buffer and add to len
    if (index == 0)
    {
        len += sprintf(((char *)buffer), "{\"%s\":[", current_sensor.c_str());
    }

    // in case no rows were found at all, close the array immediately
    if (total_rows == 0 && current_index == 0)
    {
        len += sprintf(((char *)buffer + len), "]}");
        current_index++;
    }

    // while the length of the response chunk has capacity for at least one more row and there are still rows available
    while ((len + ROWSIZE_MAX) < maxLen && current_index < total_rows)
    {
        // add a json object to the buffer and add to len
        len += sprintf(((char *)buffer + len), "{\"t\":%d,\"v\":%.2f}", timestamps[current_index], values[current_index]);
        if (current_index == total_rows - 1)
        {
            // if we are in the last chunk, close the json object
            len += sprintf(((char *)buffer + len), "]}");
            is_streaming_active = false;
        }
        else
        {
            // otherwise add a separator as there will be more rows available
            len += sprintf(((char *)buffer + len), ",");
        }
        current_index++;
    }
    // send single response chunk of this len from the buffer. If len is ever 0 the overall request will end successfully
    return len;
}
