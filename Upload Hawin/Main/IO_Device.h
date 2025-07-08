// Mengecek apakah makro IO_DEVICE_H sudah pernah didefinisikan sebelumnya
#ifndef IO_DEVICE_H

// Jika belum, maka definisikan IO_DEVICE_H agar isi file ini tidak disertakan lebih dari sekali saat kompilasi
#define IO_DEVICE_H

// Mengimpor library khusus untuk board ESP32 untuk membaca Magnetic/Optical Encoder yang ada pada Motor DC
#include <ESP32Encoder.h>

// Mengimpor library untuk mengendalikan LCD I2C (Liquid Crystal Display berbasis I2C)
#include <LiquidCrystal_I2C.h>

// Objek encoder dari library ESP32Encoder
ESP32Encoder encoder;

// Objek LCD I2C dengan alamat: 0x27 dan ukuran: 16x2
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Penutup directive IO_DEVICE_H agar tidak terjadi redefinisi saat include berulang kali
#endif