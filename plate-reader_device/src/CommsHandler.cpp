#include <CommsHandler.h>

CommsHandler::CommsHandler() 
    : mqttClient(espClient), 
      apiServer(API_SERVER_IP, API_SERVER_PORT) {}

CommsHandler::~CommsHandler() {}

/**
 * This function should be in a shared folder with both this and main.cpp files
*/
void CommsHandler::connect_wifi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.printf("Connecting to %s", WIFI_SSID);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print(" - Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    initialize_ntp_client();
}

boolean CommsHandler::connected_to_wifi(){
    return WiFi.isConnected();
}

 void CommsHandler::connect_mqtt(){
    if (!WiFi.isConnected()) {
        Serial.println("Error: WiFi not connected");
        return;
    }

    mqttClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
    mqttClient.setCallback(mqtt_message_callback);

    Serial.printf("Connecting to MQTT broker at %s:%d", MQTT_SERVER_IP, MQTT_SERVER_PORT);
    Serial.println();

    while (!mqttClient.connected()) {
        if (!mqttClient.connect(THING_ID)) {
            Serial.print("Failed to connect to MQTT broker, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" Retrying in 5 seconds");
            delay(5000);
        }
    }

    Serial.println(" - Connected to MQTT broker");
    Serial.println();

    // channels to Subscribe
    mqttClient.subscribe(IN_TOPIC);
}

boolean CommsHandler::connected_to_mqtt(){
    return mqttClient.connected();
}

void CommsHandler::start_api_server(){
    // In this case, something like this app.use("/image", &image_process), could be used
        // to route the request properly, however, since it's a simple API, we will group
        // all the requests in a single processing function
    app.use(&process_api_request);

    // Start the API server
    apiServer.begin();
    Serial.printf("API server started on port %d, serving WebHooks: ", API_SERVER_PORT);
    Serial.println();
    Serial.println(" - http://" + WiFi.localIP().toString() + ":" + String(API_SERVER_PORT) + String(IMAGE_URL_PATH));
    Serial.println(" - http://" + WiFi.localIP().toString() + ":" + String(API_SERVER_PORT) + String(JSON_URL_PATH));
    Serial.println(" - http://" + WiFi.localIP().toString() + ":" + String(API_SERVER_PORT) + String(DOCS_URL_PATH));
    Serial.println();
}

void CommsHandler::listen_to_api_clients(){
    WiFiClient client = apiServer.available();

    if (client.connected()) {
        app.process(&client);
        client.stop();
    }
}

long CommsHandler::get_time_in_ms(timeval& currentTime){
    String timeStr = NTP.getTimeStr(currentTime); // Get the time string
    long hours = timeStr.substring(0, 2).toInt(); // Extract hours
    long minutes = timeStr.substring(3, 5).toInt(); // Extract minutes
    long seconds = timeStr.substring(6, 8).toInt(); // Extract seconds
    String millisecondsStr = timeStr.substring(9, 12);
    long milliseconds = millisecondsStr.toInt(); // Convert milliseconds string to integer
    long totalTimeMs = (hours * 3600 + minutes * 60 + seconds) * 1000 + milliseconds;
    return totalTimeMs;
}

String CommsHandler::get_time_string(timeval& currentTime){
    char buffer[30];
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S%z", localtime(&currentTime.tv_sec));
    return String(buffer);
}

int CommsHandler::get_time_year(timeval& currentTime){
    return localtime(&currentTime.tv_sec)->tm_year + 1900;
}

void CommsHandler::process_sync_event (NTPEvent_t ntpEvent) {
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

void CommsHandler::initialize_ntp_client(){
    NTP.onNTPSyncEvent ([this] (NTPEvent_t event) {
        this->ntpEvent = event;
        this->syncEventTriggered = true;
        process_sync_event (ntpEvent);
    });

    NTP.setTimeZone (NTP_TIMEZONE);
    NTP.setInterval (600);
    NTP.setNTPTimeout (NTP_TIMEOUT);
    // NTP.setMinSyncAccuracy (5000);
    // NTP.settimeSyncThreshold (3000);
    NTP.begin (NTP_SERVER);

    // Wait for the response from NTP server
    Serial.println("Initializing NTP client: ");
    Serial.print(" - Fetching current time");
    timeval currentTime;
    do{
        Serial.print(".");
        delay(2500);
        gettimeofday(&currentTime, NULL);
    }while(!syncEventTriggered || get_time_year(currentTime) == 1970);

    Serial.print(" - NTP client initialized with time: "); Serial.println(NTP.getTimeDateStringUs());
    Serial.println();

}

bool CommsHandler::publish_image(timeval& imageTime, String imageURL) {
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
    doc["timestamp"] = get_time_string(imageTime);
    doc["imageId"] = get_time_in_ms(imageTime);
    doc["url"] = imageURL;

    String serializedDoc;
    if(serializeJson(doc, serializedDoc) == 0){
        Serial.println("Error: Failed to serialize JSON object");
        return false;
    }

    if(!mqttClient.publish(IMAGE_TOPIC, serializedDoc.c_str())){
        Serial.println("Error: Failed to publish image to MQTT broker");
        return false;
    }

    return true;
}