// Mengecek apakah makro CONNECTION_H sudah pernah didefinisikan sebelumnya
#ifndef CONNECTION_H

// Jika belum, maka definisikan CONNECTION_H agar isi file ini tidak disertakan lebih dari sekali saat kompilasi
#define CONNECTION_H

// Mengimpor library untuk koneksi WiFi
#include <WiFi.h>

// Mengimpor library untuk komunikasi MQTT
#include <PubSubClient.h>

// Mengimpor library untuk membuat dan menguraikan data JSON
#include <ArduinoJson.h>

// Membuat objek WiFiClient untuk komunikasi TCP (digunakan oleh MQTT client)
WiFiClient espClient;

// Membuat objek PubSubClient untuk komunikasi MQTT, menggunakan objek WiFiClient sebagai transport layer
PubSubClient client(espClient);

// Mendefinisikan nama SSID WiFi yang akan digunakan untuk koneksi
#define WIFI_NAME "YOUR_WIFI_NAME"

// Mendefinisikan password dari jaringan WiFi tersebut
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Alamat server MQTT dari platform Ubidots (digunakan untuk koneksi)
#define MQTT_SERVER "industrial.api.ubidots.com"

// Port MQTT menggunakan yang non-TLS (koneksi tanpa enkripsi)
#define MQTT_PORT 1883

// Token autentikasi milik pengguna untuk akses akun Ubidots (gunakan token akun pribadi Anda)
#define UBIDOTS_TOKEN "YOUR_UBIDOTS_TOKEN"

// Password MQTT untuk Ubidots tidak diperlukan, cukup token sebagai username
#define UBIDOTS_PASSWORD ""

// Label perangkat yang akan digunakan di Ubidots
#define DEVICE_LABEL "esp32_motor_dc"

// Label variabel (topik) untuk menampung data RPM motor di Ubidots
#define VARIABLE_LABEL1 "rpm"

// Label variabel (topik) untuk menampung data ON/OFF (switch1) di Ubidots
#define VARIABLE_LABEL2 "switch1"

// Label variabel (topik) untuk menampung data arah putaran motor (switch2) di Ubidots
#define VARIABLE_LABEL3 "switch2"

// Membuat ID client MQTT yang unik menggunakan awalan "esp32-" dan angka acak
String CLIENT_ID = "esp32-" + String(random(1000));

// Penutup directive CONNECTION_H agar tidak terjadi redefinisi saat include berulang kali
#endif