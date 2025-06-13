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

// Menyimpan status apakah sudah pernah mencoba koneksi atau belum
bool hasTriedConnecting = false;

// Menyimpan waktu terakhir kali pengecekan koneksi WiFi
unsigned long lastWiFiCheckTime = 0;

// Interval pengecekan koneksi WiFi (dalam milidetik)
const unsigned long wifiCheckInterval = 1000;


// Fungsi untuk menghubungkan ESP32 ke jaringan WiFi
void connectWiFi() {
  
  // Jika ESP32 belum terhubung ke jaringan WiFi, maka:
  if (WiFi.status() != WL_CONNECTED) {
    
    // Mengatur mode WiFi sebagai Station (bukan Access Point)
    WiFi.mode(WIFI_STA);
    
    // Memulai proses koneksi ke WiFi dengan SSID dan password yang telah didefinisikan
    WiFi.begin(WIFI_NAME, WIFI_PASSWORD);

    // Cetak informasi awal ke Serial Monitor
    debugConnPrintln("==========================================================");
    debugConnPrintln("KONFIGURASI WI-FI");
    debugConnPrintln("==========================================================");
    debugConnPrint("Menghubungkan ke Wi-Fi");

    // Selama belum terhubung dan masih dalam batas percobaan, maka:
    while (WiFi.status() != WL_CONNECTED && retry < maxRetry) {
      
      // Tunggu selama 500 milidetik
      delay(500);
      
      // Cetak -> titik (.), sebagai indikator proses koneksi sedang berlangsung
      debugConnPrint(".");
      
      // Tambah hitungan percobaan koneksi
      retry++;
    }

    // Jika berhasil terhubung, maka:
    if (WiFi.status() == WL_CONNECTED) {
      
      // Cetak informasi berhasil ke Serial Monitor
      debugConnPrintln("\n\n================ STATUS KONFIGURASI WI-FI ================");
      debugConnPrint("Berhasil tersambung ke Wi-Fi: ");
      
      // Cetak nama WiFi yang diperoleh dari router ke Serial Monitor
      debugConnPrintln(WIFI_NAME);
      
      // Cetak alamat IP lokal yang diperoleh dari router ke Serial Monitor
      debugConnPrint("IP Lokal: ");
      debugConnPrintln(WiFi.localIP().toString());
      
      // Cek apakah alamat IP yang diperoleh, jika IP nya adalah "0.0.0.0" berarti belum mendapatkan IP yang valid dari router
      if (WiFi.localIP().toString() == "0.0.0.0") {
        
        // Jika IP tidak valid, beri peringatan ke Serial Monitor bahwa koneksi perlu diulang
        debugConnPrintln("!!! IP tidak valid. Coba ulang koneksi...");
      }
      
      // Tambahkan baris kosong di Serial Monitor untuk memperjelas pemisahan log
      debugConnPrintln("\n");
      
      // Mengatur agar ESP32 secara otomatis dapat menyambungkan kembali jika koneksi terputus
      WiFi.setAutoReconnect(true);
      
      // Menyimpan kredensial WiFi secara permanen ke memori Flash ESP32
      WiFi.persistent(true);
    
      // Menandai bahwa ESP32 saat ini sudah berhasil terhubung ke jaringan WiFi
      wasConnected = true;

      // Menandai bahwa ESP32 sudah pernah mencoba melakukan koneksi WiFi setidaknya sekali
      hasTriedConnecting = true;
    }
    
    // Jika gagal terhubung, maka:
    else {
      
      // Cetak pesan gagal dan juga informasi tambahan ke Serial Monitor
      debugConnPrintln("\n\n================ STATUS KONFIGURASI WI-FI ================");
      debugConnPrintln("!!! GAGAL TERHUBUNG KE WI-FI !!!");
      debugConnPrintln("Silakan periksa SSID dan Password WiFi Anda.");
      debugConnPrint("ESP32 akan mencoba lagi nanti.\n\n\n");
    }
  }
}


// Fungsi untuk memeriksa status koneksi WiFi secara berkala dan mencoba reconnect jika terputus
void checkWiFiConnection() {

  // Ambil waktu saat ini dalam milidetik sejak perangkat mulai menyala
  unsigned long currentTime = millis();

  // Cek apakah waktu sejak pengecekan terakhir sudah melewati interval yang ditentukan
  if (currentTime - lastWiFiCheckTime >= wifiCheckInterval) {
    
    // Update waktu pengecekan terakhir menjadi waktu saat ini
    lastWiFiCheckTime = currentTime;

    // Cek status koneksi WiFi, jika tidak terhubung, maka:
    if (WiFi.status() != WL_CONNECTED) {
      
      // Jika sebelumnya ESP32 sudah terhubung, berarti koneksi baru saja terputus, maka:
      if (wasConnected) {
        
        // Cetak pesan bahwa koneksi WiFi terputus dan ESP32 akan mencoba menyambung kembali
        debugConnPrintln("================ STATUS KONFIGURASI WI-FI ================");
        debugConnPrintln("Wi-Fi terputus!\nMencoba menghubungkan kembali...\n\n");
        
        // Tandai status koneksi bahwa sudah tidak terhubung lagi
        wasConnected = false;
      }
      
      // Panggil fungsi untuk menyambungkan kembali ke jaringan WiFi
      connectWiFi();
    }
  }
}