// Mengimpor file header untuk koneksi WiFi dan Ubidots
#include "Connection.h"

// Mengimpor file header untuk inisialisasi dan kontrol perangkat I/O, seperti :
// Magnetic Encoder, Motor DC, Tombol, Potensiometer, dan LCD
#include "IO_Device.h"


// Fungsi setup akan dijalankan sekali saat board ESP32 dinyalakan
void setup() {
  
  // Menginisialisasi perangkat I/O seperti: Magnetic Encoder, Motor DC, Tombol, Potensiometer, dan LCD
  initIO();
  
  // Menyalakan atau mematikan log debug untuk koneksi Wi-Fi & IoT (true = aktif, false = nonaktif)
  debugSerialConn(true);

  // Menyalakan atau mematikan log debug untuk aktivitas Publish & Subscribe (true = aktif, false = nonaktif)
  debugSerialPubSub(true);

  // Menyalakan atau mematikan log debug untuk PID Plotter (true = aktif, false = nonaktif)
  debugSerialPidPlotter(false);
  
  // Menyambungkan ESP32 ke jaringan WiFi
  connectWiFi();
  
  // Menyambungkan ESP32 ke platform Ubidots melalui protokol MQTT
  connectUbidots();
  
  // Menginisialisasi parameter dan konfigurasi awal kontrol PID
  initPID();
}


// Fungsi loop akan dijalankan berulang kali (secara terus menerus)
void loop() {
  
  // Mengecek secara berkala apakah ESP32 masih terhubung ke WiFi, dan reconnect jika terputus
  checkWiFiConnection();
  
  // Mengecek secara berkala apakah ESP32 masih terhubung ke Ubidots, dan reconnect jika terputus
  checkUbidotsConnection();

  // Membaca nilai potensiometer dan menggunakannya sebagai input
  handlePotentiometer();
  
  // Mengecek status tombol (ON/OFF + Arah) dan melakukan aksi sesuai inputan user
  handleButtons();

  // Mengirim semua data ke Ubidots melalui protokol MQTT
  publishUbidots();
  
  // Memperbarui hitungan PID dan mengatur kecepatan motor berdasarkan error dan output PID
  updatePid();
}