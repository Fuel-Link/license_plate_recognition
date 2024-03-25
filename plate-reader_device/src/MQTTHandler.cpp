#include <MQTTHandler.h>

MQTTHandler::MQTTHandler() : mqttClient(espClient) {}

MQTTHandler::~MQTTHandler() {}

void MQTTHandler::connect_wifi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.printf("Connecting to %s", WIFI_SSID);
    Serial.println();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
}

boolean MQTTHandler::connected_to_wifi(){
    return WiFi.isConnected();
}

 void MQTTHandler::connect_mqtt(){
    if (!WiFi.isConnected()) {
        Serial.println("Error: WiFi not connected");
        return;
    }

    mqttClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
    mqttClient.setCallback(message_callback);

    Serial.printf("Connecting to MQTT broker at %s:%d", MQTT_SERVER_IP, MQTT_SERVER_PORT);
    Serial.println();

    while (!mqttClient.connected()) {
        if (mqttClient.connect(THING_ID)) {
            Serial.println("Connected to MQTT broker");
        } else {
            Serial.print("Failed to connect to MQTT broker, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" Retrying in 5 seconds");
            delay(5000);
        }
    }

    // channels to Subscribe
    mqttClient.subscribe(IN_TOPIC);
}

boolean MQTTHandler::connected_to_mqtt(){
    return mqttClient.connected();
}

bool MQTTHandler::publish_image(uint8_t *data, uint32_t size) {
    if (!WiFi.isConnected()) {
        Serial.println("Error: WiFi not connected");
        return false;
    }

    if (!mqttClient.connected()) {
        Serial.println("Error: MQTT broker not connected");
        return false;
    }

    JsonDocument doc;
    doc["thingId"] = THING_ID;
    doc["size"] = size;

    // Convert image data to base64-encoded string
    char *imageData = (char *)malloc(size);
    base64::encode(data, size, imageData);
    doc["image"] = imageData;

    uint32_t thingIdSize = strlen(THING_ID) + 1;
    uint32_t sizeSize = sizeof(size);
    uint32_t json_size = thingIdSize + sizeSize + size +1;
    psram.allocate(json_size);
    char* ptr = (char*)psram.get_mem_ptr();

    PSRAMStream psramStream(ptr);
    if(serializeJson(doc, psramStream) == 0){
        Serial.println("Error: Failed to serialize JSON object");
        return false;
    }

    boolean status = mqttClient.publish(IMAGE_TOPIC, ptr, psram.get_mem_size());

    // clear PSRAM memory
    psram.destroy();

    free(imageData);

    if(!status){
        Serial.println("Error: Failed to publish image to MQTT broker");
        return false;
    }

    return true;
}