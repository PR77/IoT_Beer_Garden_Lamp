#include <Arduino.h>
#include <NeoPixelBus.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <FS.h>
#include <WebSocketsServer.h>

//=============================================================================
// Hostname, SSID and Password for Wifi router
//=============================================================================
#define HOSTNAME "IoTBeerGarden"
const char* ssid = "...";
const char* password = "...";

//=============================================================================
// Global constants for WS2182 interface
//=============================================================================
const uint16_t PixelCount = 1;
const uint8_t PixelPin = 2;

//=============================================================================
// Global objects for Wifi and ESP specifics
//=============================================================================
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
WebSocketsServer webSocket(81);
File fsUploadFile;

//=============================================================================
// Global objects for application specifics
//=============================================================================
NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart1800KbpsMethod> strip(PixelCount, PixelPin);

RgbColor red(255, 0, 0);
RgbColor green(0, 255, 0);
RgbColor blue(0, 0, 255);

//==============================================================
// Function prototypes
//==============================================================
bool loadFromSpiffs(String path);
void handleRoot(void);
void handleFileList(void);
void handleFileUpload(void);
void handleWebRequests(void);

//==============================================================
// Helper functions
//==============================================================
bool loadFromSpiffs(String path) {
    String dataType = "text/plain";
    bool fileTransferStatus = false;

    // If a folder is requested, send the default index.html
    if (path.endsWith("/"))
        path += "index.htm";
    else if (path.endsWith(".html")) dataType = "text/html";
    else if (path.endsWith(".htm")) dataType = "text/html";
    else if (path.endsWith(".css")) dataType = "text/css";
    else if (path.endsWith(".js")) dataType = "application/javascript";
    else if (path.endsWith(".png")) dataType = "image/png";
    else if (path.endsWith(".gif")) dataType = "image/gif";
    else if (path.endsWith(".jpg")) dataType = "image/jpeg";
    else if (path.endsWith(".ico")) dataType = "image/x-icon";
    else if (path.endsWith(".xml")) dataType = "text/xml";
    else if (path.endsWith(".pdf")) dataType = "application/pdf";
    else if (path.endsWith(".zip")) dataType = "application/zip";

    File dataFile = SPIFFS.open(path.c_str(), "r");

    if (!dataFile) {
        Serial.println(String("Failed to open file: ") + path);
        return fileTransferStatus;
    }
    else {
        if (httpServer.streamFile(dataFile, dataType) == dataFile.size()) {
            Serial.println(String("Sent file: ") + path);
            fileTransferStatus = true;
        }
    }

    dataFile.close();
    return fileTransferStatus;
}

//==============================================================
// Webserver event handlers
//==============================================================
void handleRoot(void) {
    httpServer.sendHeader("Location", "/index.html", true);

    // Redirect to our index.html web page
    httpServer.send(302, "text/plain", "");
}

void handleFileList(void) {
    // curl -X GET 192.168.2.116/list

    String path = "/";
    String directoryList = "[";
    Dir directoryEntry = SPIFFS.openDir(path);

    while (directoryEntry.next()) {
        File fileElement = directoryEntry.openFile("r");

        if (directoryList != "[")
            directoryList += ", ";

        directoryList += String(fileElement.name()).substring(1);
        fileElement.close();
    }

    directoryList += "]";
    httpServer.send(200, "text/plain", directoryList);
    Serial.println(directoryList);
}

void handleFileUpload(void) {
    // curl -X POST -F "file=@SomeFile.EXT" 192.168.2.116/upload

    HTTPUpload& upload = httpServer.upload();

    Serial.println("handleFileUpload Entry: " + String(upload.status));

    if (upload.status == UPLOAD_FILE_START) {
        String filename = upload.filename;

        if (!filename.startsWith("/"))
            filename = "/" + filename;

        Serial.println("handleFileUpload Name: " + filename);

        fsUploadFile = SPIFFS.open(filename, "w");

    } else if (upload.status == UPLOAD_FILE_WRITE) {

        if(fsUploadFile)
            fsUploadFile.write(upload.buf, upload.currentSize);

    } else if (upload.status == UPLOAD_FILE_END) {

        if(fsUploadFile)
            fsUploadFile.close();

        Serial.print("handleFileUpload Size: " + String(upload.totalSize));
    }
}

void handleWebRequests(void) {
    String message = "File Not Detected\n\n";

    if (loadFromSpiffs(httpServer.uri()))
        return;

    message += "URI: ";
    message += httpServer.uri();
    message += "\nMethod: ";
    message += (httpServer.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += httpServer.args();
    message += "\n";

    for (uint8_t i = 0; i < httpServer.args(); i++) {
        message += " NAME:" + httpServer.argName(i);
        message += "\n VALUE:" + httpServer.arg(i) + "\n";
    }

    httpServer.send(404, "text/plain", message);
    Serial.println(message);
}

//==============================================================
// Websocket event handler
//==============================================================
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            // if the websocket is disconnected
            Serial.printf("[%u] Disconnected!\n", num);
        break;

        case WStype_CONNECTED: {
            // if a new websocket connection is established
            HtmlColor colourArgument;
            char htmlColourString[10];

            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

            colourArgument = HtmlColor(strip.GetPixelColor(0));

            if (colourArgument.ToNumericalString(htmlColourString, sizeof(htmlColourString))) {
                Serial.printf("Sending colourArgument: %s\n", htmlColourString);
                webSocket.sendTXT(num, htmlColourString);
            }
        }
        break;

        case WStype_TEXT: {
            // if new text data is received
            String colourArgument = (const char *)payload;
            Serial.printf("[%u] get Text: %s\n", num, payload);

            if (colourArgument.substring(0, 1) == "#") {
                HtmlColor pixelColour = HtmlColor();
                pixelColour.Parse<HtmlColorNames>(colourArgument);

                strip.SetPixelColor(0, pixelColour);
                strip.Show();
            }
        }
        break;

        case WStype_ERROR:
            Serial.printf("Error [%u] , %s\n", num, payload);
        break;

        case WStype_BIN: {
            Serial.printf("[%u] get binary length: %u\r\n", num, length);
        }
        break;

        default:
            Serial.printf("Invalid WStype [%d]\r\n", type);
        break;
    }
}

//==============================================================
// Setup function
//==============================================================
void setup() {
    Serial.begin(115200);
    Serial.println();

    // this resets all the neopixels to an off state
    strip.Begin();
    strip.SetPixelColor(0, red);
    strip.Show();

    //Initialize File System
    SPIFFS.begin();
    Serial.println("File System Initialised...");

    //Connect to your WiFi router
    WiFi.begin(ssid, password);
    WiFi.hostname(HOSTNAME);

    MDNS.begin(HOSTNAME);
    httpUpdater.setup(&httpServer);
    httpServer.begin();

    webSocket.begin();

    MDNS.addService("http", "tcp", 80);
    Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in browser\n", HOSTNAME);

    // Wait for connection
    Serial.printf("Connecting to WiFi %s...\n", ssid);
    strip.SetPixelColor(0, blue);
    strip.Show();
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(200);
    }

    // If connection successful change LED colour to GREEN
    strip.SetPixelColor(0, green);
    strip.Show();

    // If connection successful show IP address in serial monitor
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Assign server helper functions
    httpServer.on("/", handleRoot);
    httpServer.on("/list", handleFileList);
    httpServer.on("/upload", HTTP_POST, []() {
        httpServer.send(200, "text/plain", "{\"success\":1}");
    }, handleFileUpload);
    httpServer.on("/format", HTTP_POST, []() {
        SPIFFS.format();
        httpServer.send(200, "text/plain", "{\"success\":1}");
    });
    httpServer.onNotFound(handleWebRequests); //Set setver all paths are not found so we can handle as per URI

    webSocket.onEvent(webSocketEvent);
}

//==============================================================
// Loop function
//==============================================================
void loop() {
    httpServer.handleClient();
    webSocket.loop();
}
