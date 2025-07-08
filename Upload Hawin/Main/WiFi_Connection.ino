// Mode debug untuk koneksi WiFi dan IoT
bool debugConnMode = false;

// Fungsi untuk mengaktifkan / mematikan debug WiFi dan IoT
void debugSerialConn(bool enable) {
  debugConnMode = enable;
}

// Fungsi untuk mencetak pesan debug WiFi dan IoT tanpa newline
void debugConnPrint(const String& msg) {
  if (debugConnMode) Serial.print(msg);
}

// Fungsi untuk mencetak pesan debug WiFi dan IoT dengan newline
void debugConnPrintln(const String& msg) {
  if (debugConnMode) Serial.println(msg);
}

// Hitungan percobaan koneksi
int retry = 0;

// Maksimal percobaan (20 x 500ms = 10 detik)
const int maxRetry = 20;

// Menyimpan status apakah sebelumnya berhasil terkoneksi ke WiFi atau tidak
bool wasConnected = false;

// Menandakan apakah proses koneksi WiFi sedang berlangsung
bool isWiFiConnecting = false;

// Menyimpan status apakah sudah pernah mencoba koneksi atau belum
bool hasTriedConnecting = false;

// Menyimpan waktu terakhir kali pengecekan koneksi WiFi
unsigned long lastWiFiCheckTime = 0;

// Interval pengecekan koneksi WiFi (dalam milidetik)
const unsigned long wifiCheckInterval = 1000;

// Interval waktu antar percobaan koneksi WiFi (dalam milidetik)
const unsigned long wifiRetryInterval = 500;

// Menyimpan waktu (dalam milidetik) saat terakhir kali mencoba koneksi WiFi
unsigned long lastWiFiRetryTime = 0;


// Fungsi untuk menghubungkan ESP32 ke jaringan WiFi
void connectWiFi() {
  
  // Jika ESP32 belum dalam proses koneksi dan belum terhubung ke WiFi, maka:
  if (!isWiFiConnecting && WiFi.status() != WL_CONNECTED) {
    
    // Mengatur mode WiFi sebagai Station (bukan Access Point)
    WiFi.mode(WIFI_STA);
    
    // Memulai koneksi ke WiFi dengan SSID dan password yang telah didefinisikan
    WiFi.begin(WIFI_NAME, WIFI_PASSWORD);

    // Cetak informasi awal ke Serial Monitor
    debugConnPrintln("==========================================================");
    debugConnPrintln("KONFIGURASI WI-FI");
    debugConnPrintln("==========================================================");
    debugConnPrint("Menghubungkan ke Wi-Fi");

    // Mengatur ulang jumlah percobaan koneksi
    retry = 0;

    // Mencatat waktu saat pertama kali mencoba koneksi (digunakan untuk retry interval)
    lastWiFiRetryTime = millis();

    // Menandai bahwa ESP32 sedang mencoba terhubung ke WiFi (untuk mencegah pemanggilan ulang)
    isWiFiConnecting = true;

    // Menandai bahwa ESP32 telah mencoba menghubungkan setidaknya sekali
    hasTriedConnecting = true;
  }
}


// Fungsi untuk memeriksa status koneksi WiFi secara berkala dan mencoba reconnect jika terputus
void checkWiFiConnection() {

  // Ambil waktu saat ini dalam milidetik sejak perangkat mulai menyala
  unsigned long currentTime = millis();

  // Jika ESP32 sudah berhasil terhubung ke WiFi, maka:
  if (WiFi.status() == WL_CONNECTED) {

    // Jika sebelumnya belum pernah terhubung (status awal atau reconnect berhasil), maka:
    if (!wasConnected) {

      // Cetak informasi berhasil ke Serial Monitor
      debugConnPrintln("\n\n================ STATUS KONFIGURASI WI-FI ================");
      debugConnPrint("Berhasil tersambung ke Wi-Fi: ");

      // Cetak nama WiFi yang diperoleh dari router ke Serial Monitor
      debugConnPrintln(WIFI_NAME);

      // Cetak alamat IP lokal yang diperoleh dari router ke Serial Monitor
      debugConnPrint("IP Lokal: ");
      debugConnPrintln(WiFi.localIP().toString());

      // Jika IP tidak valid, maka:
      if (WiFi.localIP().toString() == "0.0.0.0") {

        // Cetak peringatan ke Serial Monitor bahwa koneksi perlu diulang
        debugConnPrintln("!!! IP tidak valid. Coba ulang koneksi...");
      }

      // Mengatur agar ESP32 secara otomatis dapat menyambungkan kembali jika koneksi terputus
      WiFi.setAutoReconnect(true);

      // Menyimpan kredensial WiFi secara permanen ke memori Flash ESP32
      WiFi.persistent(true);

      // Tambahkan baris kosong di Serial Monitor untuk memperjelas pemisahan log
      debugConnPrintln("\n");
    }

    // Menandai bahwa ESP32 saat ini sudah berhasil terhubung ke jaringan WiFi
    wasConnected = true;

    // Menandai bahwa proses koneksi WiFi telah selesai (berhasil atau gagal)
    isWiFiConnecting = false;

    // Mengatur ulang jumlah percobaan koneksi ke nol untuk persiapan retry berikutnya
    retry = 0;

    // Keluar dari fungsi karena koneksi sudah berhasil
    return;
  }

  // Jika sedang dalam proses koneksi dan sudah waktunya melakukan retry, maka:
  if (isWiFiConnecting && currentTime - lastWiFiRetryTime >= wifiRetryInterval) {

    // Menyimpan waktu saat ini sebagai patokan untuk menentukan jeda antar percobaan koneksi WiFi berikutnya
    lastWiFiRetryTime = currentTime;

    // Tambah hitungan percobaan koneksi
    retry++;

    // Tampilkan indikator proses retry
    debugConnPrint(".");

    // Jika jumlah retry sudah mencapai batas maksimum, maka:
    if (retry >= maxRetry) {

      // Cetak pesan gagal dan juga informasi tambahan ke Serial Monitor
      debugConnPrintln("\n\n================ STATUS KONFIGURASI WI-FI ================");
      debugConnPrintln("!!! GAGAL TERHUBUNG KE WI-FI !!!");
      debugConnPrintln("Silakan periksa SSID dan Password WiFi Anda.");
      debugConnPrint("ESP32 akan mencoba lagi nanti.\n\n\n");

      // Reset status koneksi agar bisa dicoba ulang nanti
      isWiFiConnecting = false; retry = 0;
    }
  }

  // Jika tidak sedang mencoba koneksi, tidak terhubung, dan sudah waktunya cek koneksi, maka:
  if (!isWiFiConnecting && WiFi.status() != WL_CONNECTED && currentTime - lastWiFiCheckTime >= 1000) {

    // Menyimpan waktu saat ini sebagai patokan untuk menentukan jeda antar percobaan koneksi WiFi berikutnya
    lastWiFiCheckTime = currentTime;

    // Jika sebelumnya ESP32 sudah terhubung, berarti koneksi baru saja terputus, maka:
    if (wasConnected) {

      // Cetak pesan bahwa koneksi WiFi terputus dan ESP32 akan mencoba menyambung kembali
      debugConnPrintln("================ STATUS KONFIGURASI WI-FI ================");
      debugConnPrintln("Wi-Fi terputus!\nMencoba menghubungkan kembali...\n\n");

      // Tandai status koneksi bahwa sudah tidak terhubung lagi
      wasConnected = false;
    }

    // Mulai proses koneksi kembali
    connectWiFi();
  }
}