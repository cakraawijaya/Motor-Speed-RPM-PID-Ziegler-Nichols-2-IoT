// Menyimpan waktu (dalam milidetik) saat terakhir kali mencoba reconnect ke server
unsigned long lastReconnectAttempt = 0;

// Status koneksi
bool isConnecting = false;

// Interval waktu minimum antar upaya reconnect, yaitu setiap 5000 ms (5 detik)
const unsigned long reconnectInterval = 5000; // 5 detik


// Fungsi untuk menginisialisasi koneksi ke server Ubidots
void connectUbidots() {
  
  // Mengatur alamat dan port server MQTT (Ubidots)
  client.setServer(MQTT_SERVER, MQTT_PORT);

  // Menetapkan fungsi callback yang akan dipanggil ketika pesan MQTT diterima
  client.setCallback(callback);

  // Menyimpan waktu saat ini sebagai waktu terakhir mencoba reconnect (digunakan untuk interval reconnect)
  lastReconnectAttempt = millis();

  // Menandakan bahwa proses koneksi telah selesai dicoba (berhasil atau gagal), sehingga tidak terus-menerus reconnect
  isConnecting = false;
}


// Fungsi untuk memeriksa koneksi MQTT dan menjaganya tetap aktif
void checkUbidotsConnection() {
  
  // Jika client MQTT belum terhubung ke server, maka :
  if (!client.connected()) {
    
    // Panggil fungsi reconnect() untuk mencoba menyambung kembali ke server MQTT
    reconnect();
  }
  
  // Fungsi loop MQTT yang wajib dipanggil secara berkala agar koneksi tetap aktif dan dapat menerima pesan masuk
  client.loop();
}


// Fungsi untuk menyambungkan ulang koneksi MQTT ke server Ubidots
void reconnect() {

  // Ambil waktu saat ini (dalam milidetik sejak ESP32 menyala)
  unsigned long currentTime = millis();

  // Hanya mencoba reconnect jika:
  // 1. Belum terhubung ke MQTT broker
  // 2. Sudah lewat interval reconnect yang ditentukan
  if (!client.connected() && (currentTime - lastReconnectAttempt >= reconnectInterval)) {

    // Perbarui waktu terakhir mencoba reconnect
    lastReconnectAttempt = currentTime;

    // Tandai bahwa proses koneksi sedang berlangsung
    isConnecting = true;

    // Cetak informasi awal ke Serial Monitor
    debugConnPrintln("==========================================================");
    debugConnPrintln("KONFIGURASI IOT PLATFORM: UBIDOTS");
    debugConnPrintln("==========================================================");
    debugConnPrintln("Menghubungkan ke Ubidots.....");

    // Jika berhasil terhubung, maka:
    if (client.connect(CLIENT_ID.c_str(), UBIDOTS_TOKEN, UBIDOTS_PASSWORD)) {
      
      // Coba sambungkan ke server MQTT Ubidots
      debugConnPrintln("\n=============== STATUS KONFIGURASI UBIDOTS ===============");
      debugConnPrintln("Berhasil tersambung ke IoT Platform: Ubidots");

      // Subscribe ke topik MQTT tertentu, agar bisa menerima pesan dari server Ubidots
      subscribeUbidots();
      debugConnPrintln("\n");

      // Tandai bahwa proses koneksi sudah selesai
      isConnecting = false;
    }

    // Jika gagal terkoneksi, maka:
    else {

      // Tampilkan pesan gagal
      debugConnPrintln("\n=============== STATUS KONFIGURASI UBIDOTS ===============");
      debugConnPrint("Gagal, Kode Error = ");
      debugConnPrintln(String(client.state()));

      // Tampilkan penjelasan berdasarkan kode Error MQTT
      switch (client.state()) {
        case -2:
          debugConnPrintln("• WiFi / Hotspot Anda tidak aktif !!!");
          debugConnPrintln("• Atau alamat Server MQTT yang Anda masukkan salah !!!");
          break;
          
        case -4:
          debugConnPrintln("• Tidak ada akses internet / paket data !!!");
          break;
          
        case 5:
          debugConnPrintln("• Token Ubidots yang Anda masukkan salah !!!");
          break;
          
        default:
          debugConnPrintln("• Gagal terkoneksi karena alasan lain.");
          break;
      }

      // Informasikan akan mencoba ulang dalam 5 detik
      debugConnPrintln("• Coba lagi 5 detik.\n\n");

      // Tandai bahwa proses koneksi sudah selesai (meski gagal)
      isConnecting = false;
    }
  }
}