// Mengimpor file header untuk koneksi WiFi dan Ubidots
#include "Connection.h"


// Fungsi setup akan dijalankan sekali saat board ESP32 dinyalakan
void setup() {
  
  // Menyalakan atau mematikan log debug untuk koneksi Wi-Fi & IoT (true = aktif, false = nonaktif)
  debugSerialConn(true);

  // Menyalakan atau mematikan log debug untuk aktivitas Publish & Subscribe (true = aktif, false = nonaktif)
  debugSerialPubSub(true);
  
  // Menyambungkan ESP32 ke jaringan WiFi
  connectWiFi();
  
  // Menyambungkan ESP32 ke platform Ubidots melalui protokol MQTT
  connectUbidots();
}


// Fungsi loop akan dijalankan berulang kali (secara terus menerus)
void loop() {
  
  // Mengecek secara berkala apakah ESP32 masih terhubung ke WiFi, dan reconnect jika terputus
  checkWiFiConnection();
  
  // Mengecek secara berkala apakah ESP32 masih terhubung ke Ubidots, dan reconnect jika terputus
  checkUbidotsConnection();
  
  // Mengirim semua data ke Ubidots melalui protokol MQTT
  publishUbidots();
}