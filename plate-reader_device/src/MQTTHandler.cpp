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

    wifiConnected = true;
}

 void MQTTHandler::connect_mqtt(){
    if (!wifiConnected) {
        Serial.println("Error: WiFi not connected");
        return;
    }

    mqttClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
    mqttClient.setCallback(message_callback);

    Serial.printf("Connecting to MQTT broker at %s:%d", MQTT_SERVER_IP, MQTT_SERVER_PORT);
    Serial.println();

    while (!mqttClient.connected()) {
        if (mqttClient.connect("ESP32Client")) {
            Serial.println("Connected to MQTT broker");
        } else {
            Serial.print("Failed to connect to MQTT broker, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" Retrying in 5 seconds");
            delay(5000);
        }
    }

    // channels to Subscribe
    mqttClient.subscribe(IMAGE_CHANNEL.c_str());
}

bool MQTTHandler::publish_image(uint8_t *data, uint32_t size) {
    if (!wifiConnected) {
        Serial.println("Error: WiFi not connected");
        return false;
    }

    return true;
}

void MQTTHandler::message_callback(char* topic, byte* payload, unsigned int length){

}