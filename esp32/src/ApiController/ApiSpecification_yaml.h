const char API_SPECIFICATION_YAML[] PROGMEM = R"=====(
swagger: '2.0'
info:
  title: ESP32 Example Project HTTP API Documentation
  description: >
     API Documentation of hardware interface.
  version: 1.0.0
basePath: /
paths:
  /api/sensors:
    get:
      tags:
        - Historic sensor data
      summary: Retrieve sensor data for a specific sensor over a given time range.
      description: This endpoint allows you to fetch sensor data for a specified sensor, filtered by a timestamp range, offset, and limit.
      parameters:
        - name: sensorName
          in: query
          description: The name of the sensor for which to retrieve data.
          required: true
          type: string
        - name: fromTimestamp
          in: query
          description: The starting timestamp (Unixtimestamp UTC without offset in milliseconds) for the data retrieval.
          required: false
          type: integer
        - name: toTimestamp
          in: query
          description: The ending timestamp (Unixtimestamp UTC without offset in milliseconds) for the data retrieval.
          required: false
          type: integer
        - name: offset
          in: query
          description: The number of records to skip before starting to return records.
          required: false
          type: integer
        - name: limit
          in: query
          description: The maximum number of records to return.
          required: false
          type: integer
      responses:
        '200':
          description: A list of sensor data points.
          schema:
            type: object
            properties:
              esp32_system_uptime_millis:
                type: array
                items:
                  type: object
                  properties:
                    t:
                      type: integer
                      description: Timestamp in milliseconds.
                    v:
                      type: number
                      description: Value of the sensor at the given timestamp.
            example:
              esp32_system_uptime_millis:
                - t: 1715004847
                  v: 60314.00
                - t: 1715004907
                  v: 120238.00
                - t: 1715004967
                  v: 180268.00
                - t: 1715005027
                  v: 240273.00
        
        '400':
          description: An error occurred. Bad request paramaters.
        
        '503':
          description: The server is busy while another sensor data request is already being processed. Try again.
)=====";