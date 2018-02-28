#include <SocketIOClient.h>
#include <ArduinoJson.h>

#define ORIGIN F("Origin: ArduinoSocketIOClient\r\n")
#define ECHO(m) Serial.println(m)
#define DATA_BUFFER_LEN 700
#define MAX_BUFFER_SIZE DATA_BUFFER_LEN - 2

WiFiClient internets;

const char* ssid = "Tang-4";
const char* password = "!@#$%^&*o9";
char databuffer[DATA_BUFFER_LEN];
char *dataptr;
String host = "viblo-posts.lc";
String token;
String authToken;
String cookie;
int port = 80;

String identifyCode = "be-ca-1";
String devicePassword = "12344321";
SocketIOClient socket;

bool waitForInput(void) {
    unsigned long now = millis();

    while (!internets.available() && ((millis() - now) < 30000UL)) {
        ;
    }
    return internets.available();
}

void sendRequest(String method, String host, int port, String path, String data) {
    char hostname[128];
    host.toCharArray(hostname, MAX_HOSTNAME_LEN);
    if (!internets.connect(hostname, port)) {
        ECHO(F("Connect failed"));
        return;
    }

    String request = "";
    request += method;
    request += F(" /");
    request += path;
    request += F(" HTTP/1.1\r\n");
    request += F("Host: ");
    request += hostname;
    request += F("\r\n");
    request += F("Accept: application/json\r\n");
    request += F("X-CSRF-TOKEN: ");
    request += token;
    request += F("\r\n");
    request += F("Cookie: ");
    request += cookie;
    request += F("\r\n");
    request += ORIGIN;
    if (data.length() > 0) {
        request += F("Content-Length: ");
        request += data.length();
        request += F("\r\n");
    }
    request += F("Content-Type: application/json\r\n");
    request += F("Connection: keep-alive\r\n\r\n");
    if (data.length() > 0) {
        request += data;
        request += F("\r\n");
    }

    ECHO(F("\r\n[sendRequest] Send request........................."));
    ECHO(request);
    ECHO(F("[sendRequest] .........................send request done\r\n"));

    internets.print(request);
    if (!waitForInput()) {
        ECHO(F("[sendRequest] Time out"));
    }
}

void stopConnect() {
    while (internets.available()) {
        readLine();
    }

    internets.stop();
    delay(100);
    ECHO(F("[stopConnect] Connect was stopped"));
}

void setCookie(String data) {
    int a = data.indexOf(";");
    String newCookie = data.substring(12, a);
    String newCookieKey = newCookie.substring(0, newCookie.indexOf("="));
    int oldCookieStart = cookie.indexOf(newCookieKey);
    if (oldCookieStart >= 0) {
        int oldCookieEnd = cookie.indexOf(";", oldCookieStart);
        String newCookies = cookie.substring(0, oldCookieStart);
        newCookies += newCookie;
        newCookies += cookie.substring(oldCookieEnd);
        cookie = newCookies;
    } else {
        cookie += newCookie;
        cookie += ";";
    }
    ECHO("[setCookie] cookie: ");
    ECHO(cookie);
}

void readLine() {
    dataptr = databuffer;
    while (dataptr < &databuffer[DATA_BUFFER_LEN - 2]) {
        if (!internets.available() && !internets.available()) {
            break;
        }
        char c = internets.read();
        if (c == '\r') {
            break;
        } else if (c != '\n') {
            *dataptr++ = c;
        }
    }
    *dataptr = 0;
}

void eatHeader() {
    while (internets.available()) { // consume the header
        readLine();
        if (strlen(databuffer) == 0) {
            break;
        }
        String data = databuffer;
        if (data.indexOf("Set-Cookie:") >=0) {
            setCookie(data);
        }
    }
}

void setupNetwork() {
   WiFi.begin(ssid, password);
   uint8_t i = 0;
   Serial.print("Connect to wifi ");
   Serial.println(ssid);
   while (WiFi.status() != WL_CONNECTED && i++ < 20) {
        Serial.print(".");
        delay(500);
   }
   if (i == 21) {
       while (1) delay(500);
   }
}

void getSession() {
    StaticJsonBuffer<DATA_BUFFER_LEN> jsonBuffer;
    String path = "device/session";
    sendRequest(String("GET"), host, port, path, String(""));
    eatHeader();
    String data = "";
    while (internets.available()) {
        readLine();
        data += String(databuffer);
    }
    Serial.println("data");
    Serial.println(data);
    JsonObject& root = jsonBuffer.parse(data);
    if (!root.success()) {
        Serial.println("Parse json failed");
        return;
    }
    int status = root["status"];
    String newToken = root["token"];
    token = newToken;
    if (status == 1) {
        Serial.println(F("User is login"));
    } else {
        Serial.println(F("User is not login"));
    }
    stopConnect();
}

void login() {
    StaticJsonBuffer<DATA_BUFFER_LEN> jsonBuffer;
    String path = "device/login";
    String data = F("{\"identify_code\":\"");
    data += identifyCode;
    data += "\",\"password\":\"";
    data += devicePassword;
    data += "\"}";

    Serial.print(F("Data login: "));
    Serial.println(data);
    sendRequest(String("POST"), host, port, path, data);
    eatHeader();
    data = "";
    while (internets.available()) {
        readLine();
        data += String(databuffer);
    }
    JsonObject& root = jsonBuffer.parse(data);
    if (!root.success()) {
        Serial.println("Parse json failed");
        return;
    }
    int status = root["status"];
    String newAuthToken = root["auth_token"];
    authToken = newAuthToken;
    stopConnect();
}

void setup() {
    Serial.begin(115200);
    setupNetwork();
    getSession();
    login();
}

void loop() {
    delay(2000);
    getSession();
}




















