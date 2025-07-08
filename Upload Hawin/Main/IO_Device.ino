// ======================== Pin PWM dan motor =========================
// Pin Kontrol Motor: PWM = Kecepatan Putaran, In1 & In2 = Arah Putaran
const int pwmPin = 18, in1Pin = 16, in2Pin = 17;

// Konfigurasi PWM: Channel, Frekuensi (Hz), Resolusi (bit)
const int pwmChannel = 0, pwmFreq = 5000, pwmResolution = 8;

// Nilai minimum PWM yang masih efektif menggerakkan Motor
const float pwmMinEffective = 60;


// =========================== Pin Encoder ============================
// Pin input yang dipakai untuk sensor Magnetic Encoder (A & B)
const byte encoderPinA = 34, encoderPinB = 35;

// Menyimpan nilai hitungan dari sensor Magnetic Encoder
long ticksCount;


// ========================== Potensiometer ===========================
// Pin input ADC yang dipakai untuk Potensiometer
const int potPin = 36;

// Nilai toleransi perubahan Potensiometer agar tidak terlalu sensitif
const int POT_DEADBAND = 20;

// Nilai terakhir yang dibaca dari Potensiometer
int lastPotValue = -1;

// Waktu terakhir pembacaan Potensiometer
unsigned long lastPotReadTime = 0;

// Jeda waktu antar pembacaan Potensiometer (ms)
const int POT_READ_INTERVAL = 100;

// Menyimpan nilai analog yang dibaca dari Potensiometer
int potValue;


// ============================== Tombol ==============================
// Pin Tombol ON/OFF dan Arah Putaran Motor
const int onOffButtonPin = 4, directionButtonPin = 5;

// Status terakhir Tombol (untuk deteksi perubahan)
bool lastOnOffState = HIGH, lastDirState = HIGH;

// Menyimpan status tombol ON/OFF Motor saat ini
bool currentOnOff;

// Menyimpan status tombol Arah Putaran Motor saat ini
bool currentDir;


// ===================== Status Motor & Kontrol =======================
// Status ON/OFF Motor
bool motorEnabled = false;

// Status Arah Putaran Motor
bool directionForward = true;

// RPM target yang diatur dari Potensiometer
float setpointRPM = 0.0;

// RPM aktual dari pembacaan sensor Magnetic Encoder
float input = 0.0;


// =============================== LCD ================================
// Waktu terakhir update LCD
unsigned long lastDisplayTime = 0;

// Interval update tampilan LCD (ms)
const unsigned long displayInterval = 200;


// Fungsi untuk menginisialisasi semua I/O (seperti pin Magnetic Encoder, Motor DC, Tombol, Potensiometer, dan LCD),
// Termasuk pemilihan nilai Baud Rate dan pengaturan lainnya
void initIO() {
  
  // Memulai komunikasi serial dengan Baud Rate 115200 untuk debugging
  Serial.begin(115200);
  
  // Set pin Arah Putaran Motor sebagai Output
  pinMode(in1Pin, OUTPUT); pinMode(in2Pin, OUTPUT);
  
  // Setup PWM: Channel, Frekuensi, Resolusi
  ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  
  // Hubungkan PWM ke pin Motor
  ledcAttachPin(pwmPin, pwmChannel);

  // Inisialisasi Encoder dengan mode Full Quadrature
  encoder.attachFullQuad(encoderPinB, encoderPinA);

  // Set pin Potensiometer sebagai Input
  pinMode(potPin, INPUT);
  
  // Set Tombol ON/OFF dengan pull-up internal
  pinMode(onOffButtonPin, INPUT_PULLUP);
  
  // Set Tombol Arah dengan pull-up internal
  pinMode(directionButtonPin, INPUT_PULLUP);

  // Inisialisasi LCD I2C
  lcd.init();
  
  // Nyalakan backlight LCD
  lcd.backlight();
  
  // Tampilkan Splash Screen awal
  startingLCD();

  // Inisialisasi waktu pembacaan Potensiometer
  lastPotReadTime = millis();
  
  // Secara Default Motor di set: 0 (OFF)
  stopMotor();
}


// Fungsi untuk menangani Input dari Tombol (misal Tombol ON/OFF dan Arah Putaran Motor)
void handleButtons() {
  
  // Baca status Tombol ON/OFF
  currentOnOff = digitalRead(onOffButtonPin);
  
  // Jika Tombol ditekan (transisi HIGH ke LOW)
  if (currentOnOff == LOW && lastOnOffState == HIGH) {
    
    // Toggle status Motor ON/OFF
    motorEnabled = !motorEnabled;
    
    // Jika dimatikan, maka :
    if (!motorEnabled) {
      // Hentikan Motor dan Update LCD
      stopMotor(); updateLCD(setpointRPM, input, "OFF");
    }
    
    // Kirim data status Motor ON/OFF ke variabel (topik) Ubidots (1 jika ON, 0 jika OFF)
    addUbidots(VARIABLE_LABEL2, motorEnabled ? 1 : 0);
    
    // Publikasikan data status Motor ON/OFF tersebut
    publishUbidots();
    
    // Debounce Delay
    delay(200);
  }
  
  // Update status Tombol ON/OFF terakhir
  lastOnOffState = currentOnOff;

  // Baca status Tombol Arah Putaran
  currentDir = digitalRead(directionButtonPin);
  
  // Jika ditekan (transisi HIGH ke LOW)
  if (currentDir == LOW && lastDirState == HIGH) {
    
    // Toggle Arah Putaran Motor
    directionForward = !directionForward;
   
    // Kirim data status Arah Putaran Motor ke variabel (topik) Ubidots (0 untuk maju, 1 untuk mundur misalnya)
    addUbidots(VARIABLE_LABEL3, directionForward ? 0 : 1);
    
    // Publikasikan data status Arah Putaran Motor tersebut
    publishUbidots();
    
    // Debounce Delay
    delay(200);
  }
  
  // Update status tombol Arah Putaran terakhir
  lastDirState = currentDir;
}


// Fungsi untuk membaca nilai Potensiometer (untuk mengatur setpoint RPM)
void handlePotentiometer() {
  
  // Menyimpan waktu saat ini (dalam milidetik) sejak ESP32 pertama kali dinyalakan
  unsigned long currentTime = millis();
  
  // Cek interval pembacaan
  if (currentTime - lastPotReadTime >= POT_READ_INTERVAL) {
    
    // Update waktu terakhir pembacaan
    lastPotReadTime = currentTime;
    
    // Baca nilai analog Potensiometer
    potValue = analogRead(potPin);
    
    // Jika perubahan cukup signifikan
    if (abs(potValue - lastPotValue) > POT_DEADBAND) {
      
      // Mapping ke rentang RPM
      setpointRPM = map(potValue, 0, 4095, 0, 492);
      
      // Simpan nilai terakhir
      lastPotValue = potValue;
    }
  }
}


// Fungsi untuk membaca jumlah pulsa dari sensor Magnetic Encoder sejak pembacaan terakhir dan meresetnya
long readAndResetEncoderTicks() {
  
  // Ambil nilai hitungan dari sensor Magnetic Encoder
  ticksCount = encoder.getCount();
  
  // Reset nilai sensor
  encoder.setCount(0);
  
  // Kembalikan nilai sebelumnya
  return ticksCount;
}


// Fungsi untuk mengatur nilai PWM untuk mengontrol kecepatan Motor dan Arah Putarannya
void setMotorPWM(float pwmVal, bool forward) {
  
  // Set Arah Motor: Maju/Mundur
  digitalWrite(in1Pin, forward ? HIGH : LOW);
  digitalWrite(in2Pin, forward ? LOW : HIGH);
  
  // Tulis nilai PWM ke Motor
  ledcWrite(pwmChannel, (int)pwmVal);
}


// Fungsi untuk menghentikan Motor dengan mengatur PWM ke nol
void stopMotor() {
  
  // PWM = 0 (Motor berhenti)
  ledcWrite(pwmChannel, 0);
  
  // Matikan kedua pin Arah
  digitalWrite(in1Pin, LOW); digitalWrite(in2Pin, LOW);
}


// Fungsi untuk menampilkan animasi pembuka di LCD
void startingLCD() {

  // Bersihkan layar LCD dari semua karakter sebelumnya
  lcd.clear();
  
  // Scroll teks "Project Skripsi" dari kanan ke kiri di baris pertama
  scrollTextFromRight("Project Skripsi", 0);
  
  // Scroll teks "PID-IoT" dari kanan ke kiri di baris kedua
  scrollTextFromRight("PID-IoT", 1);
  
  // Bersihkan layar LCD lagi setelah animasi scroll selesai
  lcd.clear();
  
  // Set posisi kursor pada kolom ke-2, baris ke-1
  lcd.setCursor(1, 0);
  
  // Tampilkan teks "Project Skripsi" pada posisi kursor
  lcd.print("Project Skripsi");
  
  // Set posisi kursor pada kolom ke-6, baris ke-2
  lcd.setCursor(5, 1);
  
  // Tampilkan teks "PID-IoT" pada posisi kursor
  lcd.print("PID-IoT");
  
  // Tahan tampilan selama 5 detik agar dapat dibaca
  delay(5000);
  
  // Panggil fungsi loadingLCD() untuk menampilkan animasi loading setelah Splash Screen
  loadingLCD();
}


// Fungsi untuk menampilkan animasi loading pada LCD
void loadingLCD() {
  
  // Bersihkan layar LCD dari karakter sebelumnya
  lcd.clear();
  
  // Atur posisi kursor pada kolom ke-1, baris ke-1
  lcd.setCursor(0, 0);
  
  // Tampilkan teks "Loading" di posisi kursor
  lcd.print("Loading");

  // Loop sebanyak 4 kali untuk menampilkan titik loading
  for (int i = 0; i < 4; i++) {
  
    // Tunggu selama 0.5 detik sebelum menampilkan titik berikutnya
    delay(500);
    
    // Cetak satu titik (.) secara berurutan untuk animasi loading
    lcd.print(".");
  }
  
  // Setelah titik keempat, tunggu selama 1 detik sebelum melanjutkan
  delay(1000);
}


// Fungsi untuk scroll teks dari kanan ke kiri di LCD pada baris tertentu
void scrollTextFromRight(String text, int row) {

  // Loop pergeseran dari 0 sampai 16 kolom (panjang LCD 16 karakter)
  for (int i = 0; i <= 16; i++) {
  
    // Bersihkan layar LCD sebelum menampilkan teks baru pada posisi berbeda
    lcd.clear();
    
    // Atur posisi kursor mulai dari kolom paling kanan (16) ke kiri secara bertahap
    lcd.setCursor(16 - i, row);
    
    // Tampilkan teks pada posisi kursor saat ini
    lcd.print(text);
    
    // Tunggu selama 200 ms untuk efek animasi scroll yang halus
    delay(200);

    // Jika baris ke-0, dan sudah scroll 16 kali untuk teks "Project Skripsi", hentikan loop
    if ((row == 0 && i == 16 - 0) && text == "Project Skripsi") break;
    
    // Jika baris ke-1, dan sudah scroll 11 kali untuk teks "PID-IoT", hentikan loop
    if ((row == 1 && i == 16 - 5) && text == "PID-IoT") break;
  }
}


// untuk menampilkan informasi mengenai status Motor pada layar LCD
void updateLCD(float setpoint, float actual, String status) {

  // Cek interval update LCD
  if ((millis() - lastDisplayTime) >= displayInterval) {
  
    // Update waktu terakhir
    lastDisplayTime = millis();

    // Label baris ke-1
    lcd.setCursor(0, 0); lcd.print("Set Point :     ");
    
    // Tampilkan nilai SetPoint
    lcd.setCursor(12, 0); lcd.print((int)setpoint); lcd.print("    ");

    // Bersihkan baris ke-2
    lcd.setCursor(0, 1); lcd.print("                ");
    lcd.setCursor(0, 1);
    
    // Jika status RUNNING, maka:
    if (status == "RUNNING") {
    
      // Cetak Label kontrol PID pada layar LCD
      lcd.print("C.PID RPM : ");
      
      // Tampilkan nilai aktual dari RPM Motor
      lcd.setCursor(12, 1); lcd.print((int)actual);
    }
    
    // Jika selain itu, maka:
    else {
      
      // Tampilkan status lain (OFF/dll)
      lcd.print("Status : "); lcd.print(status);
    }
  }
}


// Fungsi untuk mencetak data Plotter PID di Serial Plotter
void printSerialPlot(float setpoint, float rpm, float pidOutput) {
  
  // Tampilkan SetPoint
  debugPidPlotterPrint("Setpoint:"); debugPidPlotterPrint(String(setpoint));
  
  // Tampilkan RPM aktual
  debugPidPlotterPrint(",RPM:"); debugPidPlotterPrint(String(abs(rpm)));
  
  // Tampilkan Output PID
  debugPidPlotterPrint(",Output:"); debugPidPlotterPrintln(String(pidOutput));
}