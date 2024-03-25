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

    initialize_ntp_client();
    Serial.print("NTP client initialized with time: "); Serial.println(NTP.getTimeDateStringUs());
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

long MQTTHandler::get_time_in_ms(timeval& currentTime){
    String timeStr = NTP.getTimeStr(currentTime); // Get the time string
    long hours = timeStr.substring(0, 2).toInt(); // Extract hours
    long minutes = timeStr.substring(3, 5).toInt(); // Extract minutes
    long seconds = timeStr.substring(6, 8).toInt(); // Extract seconds
    String millisecondsStr = timeStr.substring(9, 12);
    long milliseconds = millisecondsStr.toInt(); // Convert milliseconds string to integer
    long totalTimeMs = (hours * 3600 + minutes * 60 + seconds) * 1000 + milliseconds;
    return totalTimeMs;
}

String MQTTHandler::get_time_string(timeval& currentTime){
    char buffer[30];
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S%z", localtime(&currentTime.tv_sec));
    return String(buffer);
}

void MQTTHandler::process_sync_event (NTPEvent_t ntpEvent) {
    switch (ntpEvent.event) {
        case timeSyncd:
        case partlySync:
        case syncNotNeeded:
        case accuracyError:
            Serial.printf ("[NTP-event] %s", NTP.ntpEvent2str (ntpEvent));
            Serial.println();
            break;
        default:
            break;
    }
    syncEventTriggered = true;
}

void MQTTHandler::initialize_ntp_client(){
    NTP.onNTPSyncEvent ([this] (NTPEvent_t event) {
        this->ntpEvent = event;
        this->syncEventTriggered = true;
        process_sync_event (ntpEvent);
    });

    NTP.setTimeZone (TZ_Europe_London);
    NTP.setInterval (600);
    NTP.setNTPTimeout (NTP_TIMEOUT);
    // NTP.setMinSyncAccuracy (5000);
    // NTP.settimeSyncThreshold (3000);
    NTP.begin (NTP_SERVER);

    while(!syncEventTriggered)
        delay(1000);

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

    timeval currentTime; // 8 bytes
    gettimeofday (&currentTime, NULL);
    doc["timestamp"] = get_time_string(currentTime);

    String auxJson;
    if(serializeJson(doc, auxJson) == 0){
        Serial.println("Error: Failed to serialize JSON object");
        return false;
    }

    // remove last '}' from JSON object and add ', "image":'
    auxJson.remove(auxJson.length() - 1);
    auxJson += ",\"image\":";

    // add the base64 encoded image data
    
    char *imageData = (char *) malloc(sizeof(uint8_t) * size);
    if(imageData == NULL){
        Serial.println("Error: Failed to allocate memory for image data");
        return false;
    }

    base64::encode(data, size, imageData);
    Serial.println("Image data encoded to base64");
    auxJson.concat((const char *)imageData, size);
    auxJson += "\"}";

    Serial.println("Image data added to JSON object");

    boolean status = mqttClient.publish(IMAGE_TOPIC, auxJson.c_str());

    free(imageData);
   
    /*
    // Convert image data to base64-encoded string
    char *imageData = (char *)malloc(sizeof(char)*size);
    if(imageData == NULL){
        Serial.println("Error: Failed to allocate memory for image data");
        return false;
    }

    // print available size in heap in bytes
    Serial.print("Available heap size after allocation: ");
    Serial.println(ESP.getFreeHeap());

    base64::encode(data, size, imageData);
    Serial.println("Image data encoded to base64");

    // print to the user the last 20 bytes of image data
    Serial.print("Last 20 bytes of image data: ");
    for(int i = size - 20; i < size; i++){
        Serial.print(data[i], HEX);
    }
    Serial.println();

    Serial.print("Available heap size after image transfer: ");
    Serial.println(ESP.getFreeHeap());

    doc["image"] = imageData;
    Serial.println("Image data added to JSON object");

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

    */

    if(!status){
        Serial.println("Error: Failed to publish image to MQTT broker");
        return false;
    }

    return true;
}