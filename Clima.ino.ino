#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>

const char* ssid = "internetdesktop";
const char* password = "19097893";

// Chave da OpenWeatherMap
const String apiKey = "f6543c6dcf2d82f7d4f0c5b963b03b12";
const String cidade = "Jana%C3%BAba";
const String estado = "BR";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -10800;
const int daylightOffset_sec = 0;

WiFiServer server(80);

String obterTemperatura() {
  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/weather?q=" + cidade + "," + estado + "&appid=" + apiKey + "&units=metric";

  http.begin(url);
  int httpCode = http.GET();
  String resultado = "Erro";

  if (httpCode == 200) {
    String payload = http.getString();
    int index = payload.indexOf("\"temp\":");
    if (index != -1) {
      int end = payload.indexOf(",", index);
      resultado = payload.substring(index + 7, end) + " °C";
    }
  }

  http.end();
  return resultado;
}

String obterDataHora() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "Erro ao obter hora";
  char buffer[64];
  strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
  return String(buffer);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConectado! IP: " + WiFi.localIP().toString());

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    while (client.connected()) {
      if (client.available()) {
        String requisicao = client.readStringUntil('\r');
        client.read(); // remove \n

        if (requisicao.indexOf("GET") >= 0) {
          String temperatura = obterTemperatura();
          String dataHora = obterDataHora();

          String html = "<html><head><meta charset='UTF-8'><title>Clima</title></head><body>";
          html += "<h1>Clima - Janaúba/MG</h1>";
          html += "<p><strong>Temperatura:</strong> " + temperatura + "</p>";
          html += "<p><strong>Data e Hora:</strong> " + dataHora + "</p>";
          html += "</body></html>";

          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          client.println(html);
        }
        break;
      }
    }
    client.stop();
  }
}
