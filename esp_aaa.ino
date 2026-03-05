#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_timer.h"
#include "sensors.h"
#include <WebSocketsServer.h>
#include <Wire.h>
#include <ArduinoJson.h>

#define FLASH_GPIO_NUM 4
#define PART_BOUNDARY "123456789000000000000987654321"

const char* ssid = "KHAI-Satellite 1";
//const char* ssid = "KHAI-Satellite 2";

const char* password = "12345678";

#define CAMERA_MODEL_AI_THINKER
#if defined(CAMERA_MODEL_AI_THINKER)
  #define PWDN_GPIO_NUM     32
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27
  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22
#else
  #error "Camera model not selected"
#endif

static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static const char* index_html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>ESP32-CAM Stream</title>
    <style>
        body {
            margin: 0;
            padding: 0;
            background: #000;
            font-family: 'Arial', sans-serif;
            color: #fff;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            overflow-x: hidden;
        }
        .stars {
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: url('data:image/svg+xml,<svg xmlns="http://www.w3.org/2000/svg" width="100" height="100"><circle cx="10" cy="10" r="1" fill="white"><animate attributeName="opacity" values="0;1;0" dur="2s" repeatCount="indefinite" begin="0s"/></circle><circle cx="50" cy="20" r="1" fill="white"><animate attributeName="opacity" values="0;1;0" dur="1.5s" repeatCount="indefinite" begin="0.7s"/></circle><circle cx="80" cy="40" r="1" fill="white"><animate attributeName="opacity" values="0;1;0" dur="2.5s" repeatCount="indefinite" begin="1.2s"/></circle><circle cx="30" cy="60" r="1" fill="white"><animate attributeName="opacity" values="0;1;0" dur="1.8s" repeatCount="indefinite" begin="0.4s"/></circle></svg>') repeat;
            z-index: -1;
        }
        .main-table {
            width: 100%;
            max-width: 1200px;
            border-collapse: collapse;
            background: rgba(0, 0, 50, 0.7);
            border: 2px solid #00f;
        }
        .main-table td, .main-table th {
            padding: 10px;
            border: 1px solid #00f;
            text-align: center;
            vertical-align: middle;
        }
        .main-table th {
            background: rgba(0, 0, 100, 0.9);
            font-size: 2em;
            text-shadow: 0 0 10px #00f;
        }
        .sensors-table {
            width: 100%;
            border-collapse: collapse;
            background: rgba(0, 0, 50, 0.7);
            border: 2px solid #00f;
        }
        .sensors-table th, .sensors-table td {
            padding: 8px;
            border: 1px solid #00f;
            text-align: left;
            font-size: 1.1em;
        }
        .sensors-table th {
            background: rgba(0, 0, 100, 0.9);
        }
        .video {
            position: relative;
            width: 100%;
            max-width: 640px;
        }
        .video img {
            width: 100%;
            border: 3px solid #fff;
            box-shadow: 0 0 15px rgba(255, 255, 255, 0.5);
        }
        .time {
            position: absolute;
            bottom: 10px;
            left: 50%;
            transform: translateX(-50%);
            font-size: 1.5em;
            color: #fff;
            text-shadow: 2px 2px 4px #000, -2px -2px 4px #000, 2px -2px 4px #000, -2px 2px 4px #000;
            background: rgba(0, 0, 0, 0.5);
            padding: 5px 10px;
            border-radius: 5px;
            white-space: nowrap;
        }
        .light-cross {
            position: absolute;
            top: 10px;
            right: 10px;
            width: 48px; /* 120 / 2.5 */
            height: 48px; /* 120 / 2.5 */
            box-shadow: 0 0 5px rgba(255, 255, 255, 0.7); /* Тень для видимости */
        }
        .light-bar {
            position: absolute;
            background: #333;
        }
        .light-fill {
            background: #ff0;
            transition: all 0.5s ease;
        }
        #ph1-bar { /* Влево */
            width: 20px; /* 50 / 2.5 */
            height: 10px; /* 20 / 2 */
            top: 19px; /* 48 / 2 - 10 / 2 */
            left: 0;
        }
        #ph1-fill {
            height: 100%;
            position: absolute;
            right: 0;
        }
        #ph2-bar { /* Вниз (назад) */
            width: 10px; /* 20 / 2 */
            height: 20px; /* 50 / 2.5 */
            bottom: 0;
            left: 19px; /* 48 / 2 - 10 / 2 */
        }
        #ph2-fill {
            width: 100%;
            position: absolute;
            top: 0;
        }
        #ph3-bar { /* Вправо */
            width: 20px; /* 50 / 2.5 */
            height: 10px; /* 20 / 2 */
            top: 19px; /* 48 / 2 - 10 / 2 */
            right: 0;
        }
        #ph3-fill {
            height: 100%;
            position: absolute;
            left: 0;
        }
        #ph4-bar { /* Вверх (прямо) */
            width: 10px; /* 20 / 2 */
            height: 20px; /* 50 / 2.5 */
            top: 0;
            left: 19px; /* 48 / 2 - 10 / 2 */
        }
        #ph4-fill {
            width: 100%;
            position: absolute;
            bottom: 0;
        }
    </style>
</head>
<body>
    <div class="stars"></div>
    <table class="main-table">
        <tr><th colspan="2">KHAI Satellite v1.2</th></tr>
        <tr>
            <td style="width: 50%;">
                <table class="sensors-table">
                    <tr><th>Parameter</th><th>Value</th></tr>
                    <tr><td>Temperature</td><td><span id="temp">Loading...</span> °C</td></tr>
                    <tr><td>Pressure</td><td><span id="pressure">Loading...</span> hPa</td></tr>
                    <tr><td>Humidity</td><td><span id="humidity">Loading...</span> %</td></tr>
                    <tr><td>Accel X</td><td><span id="ax">Loading...</span> g</td></tr>
                    <tr><td>Accel Y</td><td><span id="ay">Loading...</span> g</td></tr>
                    <tr><td>Accel Z</td><td><span id="az">Loading...</span> g</td></tr>
                    <tr><td>Gyro X</td><td><span id="gx">Loading...</span> °/s</td></tr>
                    <tr><td>Gyro Y</td><td><span id="gy">Loading...</span> °/s</td></tr>
                    <tr><td>Gyro Z</td><td><span id="gz">Loading...</span> °/s</td></tr>
                    <!--
                    <tr><td>Mag X</td><td><span id="mx">Loading...</span> µT</td></tr>
                    <tr><td>Mag Y</td><td><span id="my">Loading...</span> µT</td></tr>
                    <tr><td>Mag Z</td><td><span id="mz">Loading...</span> µT</td></tr>
                    -->
                </table>
            </td>
            <td style="width: 50%;">
                <div class="video">
                    <img src="/stream">
                    <!--
                    <div class="time">Time: <span id="time">Loading...</span></div>
                    -->
                    <div class="light-cross">
                        <div id="ph1-bar" class="light-bar"><div id="ph1-fill" class="light-fill"></div></div>
                        <div id="ph2-bar" class="light-bar"><div id="ph2-fill" class="light-fill"></div></div>
                        <div id="ph3-bar" class="light-bar"><div id="ph3-fill" class="light-fill"></div></div>
                        <div id="ph4-bar" class="light-bar"><div id="ph4-fill" class="light-fill"></div></div>
                    </div>
                </div>
            </td>
        </tr>
    </table>
    <script>
        const elements = {
            temp: document.getElementById('temp'),
            pressure: document.getElementById('pressure'),
            humidity: document.getElementById('humidity'),
            ax: document.getElementById('ax'),
            ay: document.getElementById('ay'),
            az: document.getElementById('az'),
            gx: document.getElementById('gx'),
            gy: document.getElementById('gy'),
            gz: document.getElementById('gz'),
            /*
            mx: document.getElementById('mx'),
            my: document.getElementById('my'),
            mz: document.getElementById('mz'),
            time: document.getElementById('time'),
            */
            ph1: document.getElementById('ph1-fill'),
            ph2: document.getElementById('ph2-fill'),
            ph3: document.getElementById('ph3-fill'),
            ph4: document.getElementById('ph4-fill')
        };
        const ws = new WebSocket('ws://' + window.location.hostname + ':81/');
        ws.onmessage = function(event) {
            console.log('Received data:', event.data);
            const data = JSON.parse(event.data);
            elements.temp.innerText = data.temp;
            elements.pressure.innerText = data.pressure;
            elements.humidity.innerText = data.humidity;
            elements.ax.innerText = data.ax;
            elements.ay.innerText = data.ay;
            elements.az.innerText = data.az;
            elements.gx.innerText = data.gx;
            elements.gy.innerText = data.gy;
            elements.gz.innerText = data.gz;
            /*
            elements.mx.innerText = data.mx;
            elements.my.innerText = data.my;
            elements.mz.innerText = data.mz;
            elements.time.innerText = data.time.replace(/\.| /g, '-');
            */

            const ph = [Math.min(parseFloat(data.ph1), 1100), Math.min(parseFloat(data.ph2), 1100), 
                        Math.min(parseFloat(data.ph3), 1100), Math.min(parseFloat(data.ph4), 1100)];
            const maxPh = 1100;
            elements.ph1.style.width = ((maxPh - ph[0]) / maxPh * 100) + '%';  // Влево
            elements.ph2.style.height = ((maxPh - ph[1]) / maxPh * 100) + '%'; // Вниз
            elements.ph3.style.width = ((maxPh - ph[2]) / maxPh * 100) + '%';  // Вправо
            elements.ph4.style.height = ((maxPh - ph[3]) / maxPh * 100) + '%'; // Вверх
        };
        ws.onerror = function() {
            console.log('WebSocket error');
            for (let key in elements) if (elements[key].innerText) elements[key].innerText = 'Error';
        };
        ws.onclose = function() {
            console.log('WebSocket closed');
            for (let key in elements) if (elements[key].innerText) elements[key].innerText = 'Disconnected';
        };
    </script>
</body>
</html>
)rawliteral";

httpd_handle_t stream_httpd = NULL;
WebSocketsServer webSocket = WebSocketsServer(81);
volatile int active_clients = 0;

static esp_err_t stream_handler(httpd_req_t *req) {
    if (active_clients >= 4) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Too many clients");
        return ESP_FAIL;
    }

    active_clients++;
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    char part_buf[64];

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK) {
        Serial.println("Stream: Ошибка установки типа ответа");
        active_clients--;
        return res;
    }

    while (true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Stream: Ошибка захвата изображения");
            res = ESP_FAIL;
            break;
        }

        if (fb->format == PIXFORMAT_JPEG) {
            size_t frame_size = fb->len;
            size_t hlen = snprintf(part_buf, sizeof(part_buf), _STREAM_PART, frame_size);
            res = httpd_resp_send_chunk(req, part_buf, hlen);
            if (res != ESP_OK) {
                Serial.println("Stream: Ошибка отправки заголовка");
                esp_camera_fb_return(fb);
                break;
            }

            res = httpd_resp_send_chunk(req, (const char *)fb->buf, frame_size);
            if (res != ESP_OK) {
                Serial.println("Stream: Ошибка отправки данных");
                esp_camera_fb_return(fb);
                break;
            }

            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
            if (res != ESP_OK) {
                Serial.println("Stream: Ошибка отправки границы");
                esp_camera_fb_return(fb);
                break;
            }
        }

        esp_camera_fb_return(fb);
        fb = NULL;
        delay(61);
    }

    if (fb) esp_camera_fb_return(fb);
    active_clients--;
    return res;
}

static esp_err_t index_handler(httpd_req_t *req) {
    Serial.println("Index: Запрос главной страницы");
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Type", "text/html; charset=UTF-8");
    httpd_resp_send(req, index_html, strlen(index_html));
    return ESP_OK;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("WebSocket: Клиент #%u отключился\n", num);
            break;
        case WStype_CONNECTED:
            Serial.printf("WebSocket: Клиент #%u подключился\n", num);
            break;
    }
}

void startCameraServer() {
    Serial.println("Запуск сервера...");

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.max_open_sockets = 2;
    config.lru_purge_enable = true;
    config.stack_size = 32768;

    httpd_uri_t index_uri = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = index_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t stream_uri = {
        .uri       = "/stream",
        .method    = HTTP_GET,
        .handler   = stream_handler,
        .user_ctx  = NULL
    };

    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &index_uri);
        httpd_register_uri_handler(stream_httpd, &stream_uri);
        Serial.println("HTTP сервер запущен на порту 80");
    } else {
        Serial.println("Ошибка запуска HTTP сервера на порту 80");
    }

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("WebSocket сервер запущен на порту 81");
}

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    Serial.begin(115200);
    Serial.println("Запуск системы...");

    pinMode(FLASH_GPIO_NUM, OUTPUT);
    digitalWrite(FLASH_GPIO_NUM, HIGH);
    delay(1000);
    digitalWrite(FLASH_GPIO_NUM, LOW);

    Wire.begin(15, 13); // SDA=15, SCL=13
    initSensors();

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_VGA; // 640x480
    config.jpeg_quality = 8; // Качество
    config.fb_count = 2;

    Serial.println("Инициализация камеры...");
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Ошибка инициализации камеры: 0x%x\n", err);
        return;
    } else {
        Serial.println("Камера успешно инициализирована");
    }

    sensor_t * s = esp_camera_sensor_get();
    if (s) {
        s->set_vflip(s, 1); // Вертикальный поворот
        s->set_hmirror(s, 1); // Горизонтальное зеркало
        s->set_brightness(s, 1); // Яркость +1
        s->set_contrast(s, 1);   // Контраст +1
        s->set_sharpness(s, 1);  // Резкость +1
        s->set_saturation(s, 1); // Насыщенность +1
        Serial.println("Камера повёрнута на 180 градусов, улучшены параметры изображения");
    } else {
        Serial.println("Ошибка: Не удалось получить сенсор камеры");
    }

    Serial.println("Запуск WiFi...");
    WiFi.softAP(ssid, password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("IP адрес точки доступа: ");
    Serial.println(myIP);

    startCameraServer();
    Serial.println("Доступ: http://192.168.4.1/");
}
void loop() {
    static unsigned long last_update = 0;
    unsigned long current_time = millis();

    webSocket.loop();

    if (current_time - last_update >= 500) {
        pointer_of_sensors *sensors = get_sensors_data();
        if (sensors) {
            StaticJsonDocument<512> doc;
            if (sensors->bme_) {
                doc["temp"] = String(sensors->bme_->temperature, 2);
                doc["pressure"] = String(sensors->bme_->pressure, 2);
                doc["humidity"] = String(sensors->bme_->humidity, 2);
            } else {
                doc["temp"] = "N/A";
                doc["pressure"] = "N/A";
                doc["humidity"] = "N/A";
            }
            if (sensors->mpu_) {
                doc["ax"] = String(sensors->mpu_->aX, 2);
                doc["ay"] = String(sensors->mpu_->aY, 2);
                doc["az"] = String(sensors->mpu_->aZ, 2);
                doc["gx"] = String(sensors->mpu_->gX, 2);
                doc["gy"] = String(sensors->mpu_->gY, 2);
                doc["gz"] = String(sensors->mpu_->gZ, 2);
            } else {
                doc["ax"] = "N/A";
                doc["ay"] = "N/A";
                doc["az"] = "N/A";
                doc["gx"] = "N/A";
                doc["gy"] = "N/A";
                doc["gz"] = "N/A";
            }
            if (sensors->ads_) {
                doc["ph1"] = String(sensors->ads_->ph1, 0);
                doc["ph2"] = String(sensors->ads_->ph2, 0);
                doc["ph3"] = String(sensors->ads_->ph3, 0);
                doc["ph4"] = String(sensors->ads_->ph4, 0);
            } else {
                doc["ph1"] = "N/A";
                doc["ph2"] = "N/A";
                doc["ph3"] = "N/A";
                doc["ph4"] = "N/A";
            }
            if (sensors->rtc_) {
                char time_str[20];
                snprintf(time_str, sizeof(time_str), "%04d.%02d.%02d %02d:%02d:%02d",
                         sensors->rtc_->year_, sensors->rtc_->month_, sensors->rtc_->day_,
                         sensors->rtc_->hour_, sensors->rtc_->minute_, sensors->rtc_->second_);
                doc["time"] = String(time_str);
            } else {
                doc["time"] = "N/A";
            }

            String json;
            serializeJson(doc, json);
            webSocket.broadcastTXT(json);
        } else {
            webSocket.broadcastTXT("{\"error\":\"Sensor data unavailable\"}");
            Serial.println("WebSocket: Ошибка получения данных датчиков");
        }
        last_update = current_time;
    }

    if (current_time - last_update >= 5000) {
        pointer_of_sensors *sensors = get_sensors_data();
        if (sensors) {
            Serial.printf("Free Heap: %u bytes, System Uptime: %u ms\n", ESP.getFreeHeap(), current_time);
            print_sensors_data(sensors);
        }
    }


delay(100);
return;

}
