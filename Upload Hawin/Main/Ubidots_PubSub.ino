// Variabel static untuk menyimpan waktu terakhir data dikirim
static unsigned long lastSend = 0;

// Interval waktu antar publish data dalam milidetik (5 detik)
const unsigned long interval = 5000;

// Array untuk menampung label variabel Ubidots (misal: rpm, switch1, switch2)
String variables[3] = {VARIABLE_LABEL1, VARIABLE_LABEL2, VARIABLE_LABEL3};

// Array untuk menyimpan nilai dari masing-masing variabel
int values[3] = {0, 0, 0};

// Mode debug untuk Publish dan Subscribe
bool debugPubSubMode = false;

// Fungsi untuk mengaktifkan / mematikan debug Publish dan Subscribe
void debugSerialPubSub(bool enable) {
  debugPubSubMode = enable;
}

// Fungsi untuk mencetak pesan debug Publish dan Subscribe tanpa newline
void debugPubSubPrint(const String& msg) {
  if (debugPubSubMode) Serial.print(msg);
}

// Fungsi untuk mencetak pesan debug Publish dan Subscribe dengan newline
void debugPubSubPrintln(const String& msg) {
  if (debugPubSubMode) Serial.println(msg);
}

// Menandai apakah data RPM sudah diterima atau belum
bool rpmDataReceived = false;

// Variabel penanda apakah data perlu dipublikasikan (dikirim) ke server MQTT Ubidots
bool needPublish = false;


// Fungsi untuk menyimpan nilai berdasarkan label
void addUbidots(String var, int val) {
  
  // Loop mengecek semua label
  for (int i = 0; i < 3; i++) {
    
    // Jika label cocok, maka:
    if (var == variables[i]) {
      
      // Simpan nilai ke array values sesuai indeks
      values[i] = val;
      
      // Hentikan loop jika sudah ketemu
      break;
    }
  }
}


// Fungsi untuk mengirim data secara berkala ke platform Ubidots melalui MQTT
void publishUbidots() {

  // Ambil waktu sekarang (dalam milidetik sejak ESP32 menyala)
  unsigned long currentTime = millis();

  // Cek apakah sudah waktunya mengirim data lagi
  if (currentTime - lastSend >= interval) {
    
    // Update waktu terakhir mengirim data
    lastSend = currentTime;

    // Buat dokumen JSON statis untuk payload data (ukuran 128 byte)
    StaticJsonDocument<128> DOC;

    // Isi JSON dengan data dari variabel dan nilai yang sudah ada
    for (int i = 0; i < 3; i++) {
      
      // variables adalah array variabel (topik), dan values ini adalah nilainya
      DOC[variables[i]] = values[i];
    }

    // Buffer untuk menampung string JSON hasil serialisasi
    char PAYLOAD[128];

    // Serialisasi JSON ke string (payload siap dikirim)
    serializeJson(DOC, PAYLOAD);

    // Buat topic MQTT berdasarkan device label
    String TOPIC = "/v1.6/devices/" + String(DEVICE_LABEL);

    // Kirim data JSON ke topic MQTT Ubidots
    client.publish(TOPIC.c_str(), PAYLOAD);

    // Tampilkan info debug terkait proses publish MQTT
    debugPubSubPrintln("\n\n==========================================================");
    debugPubSubPrintln("PUBLISH MQTT");
    debugPubSubPrintln("==========================================================");
    debugPubSubPrintln("Topic: " + TOPIC);
    debugPubSubPrintln("Json Data: " + String(PAYLOAD));
    debugPubSubPrint("\n\n");
  }
}


// Fungsi untuk melakukan subscribe ke beberapa variabel (topik) yang telah dibuat di Ubidots
void subscribeUbidots() {
  
  // Subscribe ke variabel (topik) RPM (untuk monitoring RPM Motor)
  client.subscribe(("/v1.6/devices/" + String(DEVICE_LABEL) + "/" + String(VARIABLE_LABEL1) + "/lv").c_str());
  
  // Subscribe ke variabel (topik) switch1 (untuk ON/OFF Motor)
  client.subscribe(("/v1.6/devices/" + String(DEVICE_LABEL) + "/" + String(VARIABLE_LABEL2) + "/lv").c_str());
  
  // Subscribe ke variabel (topik) switch2 (untuk Arah Rotasi Motor)
  client.subscribe(("/v1.6/devices/" + String(DEVICE_LABEL) + "/" + String(VARIABLE_LABEL3) + "/lv").c_str());
}


// Fungsi callback: mengambil data kembali dari Ubidots untuk keperluan tertentu (Subscribe)
void callback(char* TOPIC, byte* PAYLOAD, unsigned int length) {
  
  // Tampilkan header debug saat menerima pesan Subscribe MQTT
  debugPubSubPrintln("==========================================================");
  debugPubSubPrintln("SUBSCRIBE MQTT");
  debugPubSubPrintln("==========================================================");
  
  // Tampilkan variabel (topik) yang diterima dari Ubidots
  debugPubSubPrintln("Topic: " + String(TOPIC));

  // Inisialisasi string kosong untuk menampung payload pesan
  String message = "";

  // Loop untuk mengkonversi payload byte menjadi string karakter
  for (unsigned int i = 0; i < length; i++) {
    message += (char)PAYLOAD[i];
  }

  // Konversi pesan string ke nilai float
  float value = message.toFloat();

  // Simpan topik sebagai string untuk mempermudah pengecekan
  String topicStr = String(TOPIC);

  // Cek apakah topik saat ini mengandung VARIABLE_LABEL1 (RPM), jika iya maka:
  if (topicStr.indexOf(VARIABLE_LABEL1) >= 0) {

    // Tandai bahwa data RPM sudah diterima
    rpmDataReceived = true;

    // Tampilkan nilai RPM yang diterima
    debugPubSubPrint("RPM: ");
    debugPubSubPrintln(String((int)value));
  }
  
  // Cek apakah topik saat ini mengandung VARIABLE_LABEL2 (ON/OFF), jika iya maka:
  else if (topicStr.indexOf(VARIABLE_LABEL2) >= 0) {

    // Tampilkan pesan: "Status Aktivasi Motor:"
    debugPubSubPrint("Status Aktivasi Motor: ");
    
    // Atur status motor ini berdasarkan nilai yang diterima (1 = ON, 0 = OFF)
    motorEnabled = ((int)value == 1);
    debugPubSubPrintln(motorEnabled ? "Menyala" : "Mati");

    // Kirim data Status ON/OFF Motor pada topik yang sudah dibuat di platform Ubidots
    addUbidots(VARIABLE_LABEL2, motorEnabled ? 1 : 0);

    // Tandai bahwa data perlu dipublikasikan
    needPublish = true;
  }
  
  // Cek apakah topik saat ini mengandung VARIABLE_LABEL3 (Maju/Mundur), jika iya maka:
  else if (topicStr.indexOf(VARIABLE_LABEL3) >= 0) {

    // Tampilkan pesan: "Arah Rotasi Motor:"
    debugPubSubPrint("Arah Rotasi Motor: ");

    // Atur Arah Motor ini berdasarkan nilai yang diterima (0 = Maju, 1 = Mundur)
    directionForward = ((int)value == 0);
    debugPubSubPrintln(directionForward ? "Maju" : "Mundur");

    // Kirim data Status Arah Putaran Motor pada topik yang sudah dibuat di platform Ubidots
    addUbidots(VARIABLE_LABEL3, directionForward ? 0 : 1);

    // Tandai bahwa data perlu dipublikasikan
    needPublish = true;
  }

  // Jika ada data baru yang perlu dikirim, panggil fungsi Publish dan Reset Flag
  if (needPublish) {
    
    // Memanggil fungsi publishUbidots() untuk mengirim data sensor atau status yang
    // telah disiapkan ke platform Ubidots melalui protokol MQTT
    publishUbidots();

    // Reset setelah data berhasil dipublikasikan
    needPublish = false;
  }
}