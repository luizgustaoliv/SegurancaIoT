#include <WiFi.h>

// ===== WIFI =====
const char* ssid     = "nome-rede";
const char* password = "senha-rede";

WiFiServer server(80);

// GPIOs
const int gpio26 = 26;
const int gpio27 = 27;

bool estado26 = false;  // false = desligado
bool estado27 = false;

unsigned long currentTime = 0;
unsigned long previousTime = 0;
const long timeoutTime = 2000;

// Gera HTML
String buildHTML() {
  String s = "<!DOCTYPE html><html><head>";
  s += "<meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'>";
  s += "<title>ESP32 Web Server</title>";
  s += "<style>body{font-family:Arial;text-align:center;margin-top:40px;}";
  s += ".card{border:1px solid #ddd;border-radius:10px;padding:16px;margin:8px;display:inline-block;}";
  s += ".btn{padding:10px 20px;font-size:18px;border:none;border-radius:6px;cursor:pointer;margin:4px;}";
  s += ".on{background:#2ecc71;color:#fff;}.off{background:#e74c3c;color:#fff;}";
  s += "</style></head><body>";
  s += "<h1>ESP32 Web Server</h1>";

  // GPIO 26
  s += "<div class='card'><h2>GPIO 26</h2>";
  s += "<p>Estado: <b>";
  s += (estado26 ? "on" : "off");
  s += "</b></p>";
  if (!estado26)
    s += "<p><a href='/26/on'><button class='btn on'>Ligar</button></a></p>";
  else
    s += "<p><a href='/26/off'><button class='btn off'>Desligar</button></a></p>";
  s += "</div>";

  // GPIO 27
  s += "<div class='card'><h2>GPIO 27</h2>";
  s += "<p>Estado: <b>";
  s += (estado27 ? "on" : "off");
  s += "</b></p>";
  if (!estado27)
    s += "<p><a href='/27/on'><button class='btn on'>Ligar</button></a></p>";
  else
    s += "<p><a href='/27/off'><button class='btn off'>Desligar</button></a></p>";
  s += "</div>";

  s += "</body></html>";
  return s;
}

void aplicaEstadoSaidas() {
  // Se seu LED estiver ligado NO GND (modo normal), use isto:
  digitalWrite(gpio26, estado26 ? HIGH : LOW);
  digitalWrite(gpio27, estado27 ? HIGH : LOW);

  // SE o LED do 27 for ligado NO 3V3 (ativo em LOW), comente as duas linhas acima
  // e use ESTA linha somente para o 27:
  // digitalWrite(gpio27, estado27 ? LOW : HIGH);
}

void setup() {
  Serial.begin(115200);

  pinMode(gpio26, OUTPUT);
  pinMode(gpio27, OUTPUT);

  estado26 = false;
  estado27 = false;
  aplicaEstadoSaidas();

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (!client) return;

  Serial.println("Novo cliente");
  String header = "";
  String currentLine = "";
  currentTime = millis();
  previousTime = currentTime;

  while (client.connected() && (currentTime - previousTime <= timeoutTime)) {
    currentTime = millis();
    if (client.available()) {
      char c = client.read();
      header += c;

      if (c == '\n') {
        if (currentLine.length() == 0) {
          // ---- ROTAS ----
          if (header.indexOf("GET /26/on")  >= 0) estado26 = true;
          if (header.indexOf("GET /26/off") >= 0) estado26 = false;
          if (header.indexOf("GET /27/on")  >= 0) estado27 = true;
          if (header.indexOf("GET /27/off") >= 0) estado27 = false;

          aplicaEstadoSaidas();

          // ---- RESPOSTA ----
          String html = buildHTML();
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          client.print(html);
          break;
        } else {
          currentLine = "";
        }
      } else if (c != '\r') {
        currentLine += c;
      }
    }
  }

  client.stop();
  Serial.println("Cliente desconectado\n");
}
