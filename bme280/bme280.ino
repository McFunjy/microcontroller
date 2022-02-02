/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

// Подключаем библиотеку Wi-Fi
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;

// Замените своими данными
const char* ssid     = ""; //Имя сети
const char* password = ""; //Пароль

// Указываем, что сервер будет использовать 80 порт
WiFiServer server(80);

// Переменная для хранения HTTP запроса
String header;

// Текущее время 
unsigned long currentTime = millis();
// Переменная для сохранения времени подключения пользователя
unsigned long previousTime = 0; 
// Определяем задержку в миллисекундах
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  bool status;

  // настройки по умолчанию 
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  // Подключаемся к Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Выводим IP-адрес и запускаем веб-сервер
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Ждем подключения пользователя

  if (client) {                             // Если есть подключение,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // выводим сообщение в монитор порта
    String currentLine = "";                // создаем строку для хранения входящих данных
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // выполняем программу, пока пользователь подключен
      currentTime = millis();
      if (client.available()) {             // проверяем, есть ли входящее сообщение
        char c = client.read();             // читаем и
        Serial.write(c);                    // выводим в монитор порта
        header += c;
        if (c == '\n') {                    // если входящее сообщение – переход на новую строку (пустая строка)
          // то считаем это концом HTTP запроса и выдаем ответ
          if (currentLine.length() == 0) {
            // заголовок всегда начинается с ответа (например, HTTP/1.1 200 OK)
            // добавляем тип файла ответа:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Выводим HTML-страницу
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // Добавляем стили CSS 
            client.println("<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial;}");
            client.println("table { border-collapse: collapse; width:35%; margin-left:auto; margin-right:auto; }");
            client.println("th { padding: 12px; background-color: #0043af; color: white; }");
            client.println("tr { border: 1px solid #ddd; padding: 12px; }");
            client.println("tr:hover { background-color: #bcbcbc; }");
            client.println("td { border: none; padding: 12px; }");
            client.println(".sensor { color:white; font-weight: bold; background-color: #bcbcbc; padding: 1px; }");
            
            // Заголовок веб-страницы
            client.println("</style></head><body><h1>ESP32 with BME280</h1>");
            client.println("<table><tr><th>MEASUREMENT</th><th>VALUE</th></tr>");
            client.println("<tr><td>Temp. Celsius</td><td><span class=\"sensor\">");
            client.println(bme.readTemperature());
            client.println(" *C</span></td></tr>");  
            client.println("<tr><td>Temp. Fahrenheit</td><td><span class=\"sensor\">");
            client.println(1.8 * bme.readTemperature() + 32);
            client.println(" *F</span></td></tr>");       
            client.println("<tr><td>Pressure</td><td><span class=\"sensor\">");
            client.println(bme.readPressure() / 100.0F);
            client.println(" hPa</span></td></tr>");
            client.println("<tr><td>Approx. Altitude</td><td><span class=\"sensor\">");
            client.println(bme.readAltitude(SEALEVELPRESSURE_HPA));
            client.println(" m</span></td></tr>"); 
            client.println("<tr><td>Humidity</td><td><span class=\"sensor\">");
            client.println(bme.readHumidity());
            client.println(" %</span></td></tr>"); 
            client.println("</body></html>");
            
            // Ответ HTTP также заканчивается пустой строкой
            client.println();
            // Прерываем выполнение программы
            break;
          } else { // если у нас есть новый запрос, очищаем строку
            currentLine = "";
          }
        } else if (c != '\r') {  // но, если отправляемая строка не пустая
          currentLine += c;      // добавляем ее в конец строки
        }
      }
    }
    // Очищаем заголовок
    header = "";
    // Сбрасываем соединение
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
