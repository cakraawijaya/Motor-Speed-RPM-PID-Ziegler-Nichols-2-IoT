// Count Per Revolution (CPR): 124 Pulses Per Revolution (PPR) × 4 (Quadrature Encoding)
const float CPR = 496.0;

// Interval waktu sampling PID dalam milidetik (50 ms)
const unsigned long sampleTime = 50;

// Mode kontrol PID (1 = P saja, 2 = PID lengkap)
int controlMode = 2;

// Parameter Kuasi-Osilasi (Ultimate Gain dan Periode Osilasi) untuk Tuning Ziegler-Nichols
float Ku = 12.0, Tu = 0.3;

// Parameter kontrol PID (Proporsional, Integral, Derivatif)
float Kp = 7.2, Ki = 48, Kd = 0.27;

// Waktu pengukuran terakhir untuk interval sampling
unsigned long lastTime = 0;

// Menyimpan jumlah pulsa yang diterima oleh sensor Magnetic Encoder
long ticks;

// Menyimpan jumlah pulsa yang diterima per detik dari hasil perhitungan
float ticksPerSecond;

// Sebuah flag boolean yang menunjukkan apakah motor sedang dalam keadaan aktif atau beroperasi
bool motorActive;

// Variabel Output PWM, Input Terakhir (RPM), Integral, Error, dan Derivative
float output = 0, lastInput = 0, integral = 0, error = 0, derivative = 0;

// Mode debug untuk PID Plotter dalam keadaan mati (OFF)
bool debugPidPlotterMode = false;

// Fungsi untuk mengaktifkan / mematikan debug PID Plotter
void debugSerialPidPlotter(bool enable) {
  debugPidPlotterMode = enable;
}

// Fungsi untuk mencetak pesan debug PID Plotter tanpa newline
void debugPidPlotterPrint(const String& msg) {
  if (debugPidPlotterMode) Serial.print(msg);
}

// Fungsi untuk mencetak pesan debug PID Plotter dengan newline
void debugPidPlotterPrintln(const String& msg) {
  if (debugPidPlotterMode) Serial.println(msg);
}


// Fungsi inisialisasi PID dan Setting parameter sesuai mode kontrol dan Metode Tuning
void initPID() {

  // Jika mode PID lengkap dan Parameter Tuning valid, maka:
  if (controlMode == 2 && Ku > 0 && Tu > 0) {
  
    // Hitung Kp, Ki, Kd dengan aturan Ziegler-Nichols
    Kp = 0.6 * Ku;
    Ki = 2.0 * Kp / Tu;
    Kd = Kp * Tu / 8.0;
  }
  
  // Jika mode kontrol P saja, maka:
  else if (controlMode == 1) {
    
    // Matikan integral dan derivatif
    Ki = 0.0; Kd = 0.0;
  }
  
  // Simpan waktu mulai pengukuran
  lastTime = millis();
}


// Fungsi utama untuk mengatur dan mengendalikan kecepatan motor secara otomatis menggunakan algoritma PID
void updatePid() {

  // Ambil waktu saat ini
  unsigned long currentTime = millis();
  
  // Abaikan jika interval sampling belum tercapai
  if (currentTime - lastTime < sampleTime) return;
  
  // Update waktu pengukuran terakhir
  lastTime = currentTime;

  // Baca jumlah pulsa Encoder sejak terakhir dan Reset penghitung
  ticks = readAndResetEncoderTicks();

  // Hitung pulsa per detik (sesuaikan dengan sampleTime)
  ticksPerSecond = abs(ticks * (1000.0 / sampleTime));
  
  // Konversi pulsa per detik ke RPM (Rotasi per menit)
  input = abs((float(ticksPerSecond) / CPR) * 60.0);
  
  // Jika Arah Motor mundur, jadikan Input negatif menjadi positif
  if (!directionForward) input *= abs(-1);

  // Motor dianggap aktif jika Enabled dan SetPoint RPM cukup tinggi
  motorActive = motorEnabled && setpointRPM > 5.0;
  
  // Jika Motor tidak aktif, maka:
  if (!motorActive) {
    
    // Matikan motor
    stopMotor();
    
    // Tampilkan status STANDBY atau OFF di LCD
    updateLCD(setpointRPM, input, motorEnabled ? "STANDBY" : "OFF");
    
    // Reset Output PWM
    output = 0;
    
    // Cetak data ke Serial Plotter
    printSerialPlot(setpointRPM, input, output);
    
    // Keluar dari fungsi update
    return;
  }

  // Hitung error (selisih SetPoint dengan Input RPM aktual)
  error = setpointRPM - input;
  
  // Jika mode P saja, maka:
  if (controlMode == 1) {
    
    // Output hanya Proporsional — tanpa integral (I) dan tanpa derivatif (D)
    output = Kp * error;
  }

  // Jika lainnya, maka:
  else {
    
    // Hitung akumulasi Integral error dengan konversi waktu ke detik
    integral += error * (sampleTime / 1000.0);
    
    // Reset Integral jika Arah error berbalik (Anti-Windup)
    if ((error * (setpointRPM - lastInput)) < 0) integral = 0;
    
    // Hitung turunan (Derivative) berdasarkan perubahan Input RPM
    derivative = (input - lastInput) / (sampleTime / 1000.0);
    
    // Hitung Output PID lengkap (P + I - D)
    output = Kp * error + Ki * integral - Kd * derivative;
    
    // Simpan Input saat ini untuk perhitungan Derivative berikutnya
    lastInput = input;
  }

  // Batasi Output PWM antara 0 - 255 (range PWM)
  output = constrain(output, 0, 255);
  
  // Jika Output terlalu kecil, naikan ke nilai minimal efektif PWM
  if (output > 0 && output < pwmMinEffective) output = pwmMinEffective;

  // Kirim data RPM ke variabel (topik) Ubidots
  addUbidots(VARIABLE_LABEL1, input);
  
  // Publikasikan data RPM tersebut
  publishUbidots();

  // Atur PWM Motor dengan menggunakan parameter Output dan Arah Putaran Motor
  setMotorPWM(output, directionForward);
  
  // Update tampilan LCD dengan status RUNNING
  updateLCD(setpointRPM, input, "RUNNING");
  
  // Cetak data ke Serial Plotter
  printSerialPlot(setpointRPM, input, output);
}