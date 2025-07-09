[![Open Source Love](https://badges.frapsoft.com/os/v1/open-source.svg?style=flat)](https://github.com/ellerbrock/open-source-badges/)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg?logo=github&color=%23F7DF1E)](https://opensource.org/licenses/MIT)
![GitHub last commit](https://img.shields.io/github/last-commit/cakraawijaya/Motor-Speed-RPM-PID-Ziegler-Nichols-2-IoT?logo=Codeforces&logoColor=white&color=%23F7DF1E)
![Project](https://img.shields.io/badge/Project-ESP32-light.svg?style=flat&logo=espressif&logoColor=white&color=%23F7DF1E)
![Type](https://img.shields.io/badge/Type-Undergraduate%20Thesis-light.svg?style=flat&logo=gitbook&logoColor=white&color=%23F7DF1E)

# Motor-Speed-RPM-PID-Ziegler-Nichols-2-IoT
<strong>Undergraduate Thesis Project Documentation (S1) - Informatics UPN Veteran Jatim</strong><br><br>
In the industrial sector, DC conveyor motors are commonly used to move materials efficiently. Maintaining a stable speed is essential to ensure product quality and smooth production. Previous research faced issues with microcontroller selection and suboptimal PID implementation. Remote control was underutilized, and system integration was not fully synchronized. These shortcomings affected the flexibility and reliability of the conveyor. This project aims to improve speed control with IoT integration. The ESP32 microcontroller manages ON/OFF functions, rotation direction, and RPM setpoint. Ubidots serves as the remote monitoring and control platform. The Ziegler-Nichols PID method is applied to stabilize motor speed. The project was developed over one year and is expected to enhance the efficiency and reliability of small-scale industrial automation.

<br><br>

## Project Requirements
| Part | Description |
| --- | --- |
| Development Board | DOIT ESP32 DEVKIT V1 |
| Code Editor | Arduino IDE |
| Driver | CP210X USB Driver |
| IoT Platform | Ubidots |
| Communications Protocol | • Inter Integrated Circuit (I2C)<br>• Message Queuing Telemetry Transport (MQTT) |
| IoT Architecture | 3 Layer |
| Programming Language | C/C++ |
| Arduino Library | • WiFi (default)<br>• PubSubClient<br>• LiquidCrystal_I2C<br>• ArduinoJson<br>• ESP32Encoder |
| Actuators | • Motor DC JGA25-370 (x1) |
| Sensor | • Encoder Sensor (x1)<br>• Potentiometer (x1) |
| Display | LCD I2C (x1) |
| Other Components | • USB type C - USB type A cable (x1)<br>• Jumper cable (1 set)<br>• Female DC power adapter (x1)<br>• Push button 12 x 12 mm (x2)<br>• Motor driver L298N (x1)<br>• Adaptor 12V 2A (x1)<br>• Adaptor 5V 2A (x1)<br>• Breadboard (x1)<br>• Plywood 50 x 50 cm (x2)<br>• Stainless Steel Concrete 30 cm (x1)<br>• 1/2 Inch Pipe 25 cm (x1)<br>• Oscar fabric 50 x 137 cm (x1)<br>• Spicer bolts (1 set)<br>• Bolts plus (1 set)<br>• Nuts (1 set)<br>• L Bracket (1 set)<br>• PVC Electrical insulation (1 set)<br>• Sandpaper G-180 1 m (x1) |

<br><br>

## Download & Install 
1. Arduino IDE

   <table><tr><td width="810">

   ```
   https://www.arduino.cc/en/software
   ```

   </td></tr></table><br>

2. CP210X USB Driver

   <table><tr><td width="810">

   ```
   https://bit.ly/CP210X_USBdriver
   ```

   </td></tr></table>

<br><br>

## Project Designs
<table>
<tr>
<th width="840">Infrastructure</th>
</tr>
<tr>
<td><img src="Assets/Documentation/Diagram/Infrastructure.jpg" alt="infrastructure"></td>
</tr>
</table>
<table>
<tr>
<th width="420">Block Diagram</th>
<th width="420">Pictorial Diagram</th>
</tr>
<tr>
<td><img src="Assets/Documentation/Diagram/Block Diagram.jpg" alt="block-diagram"></td>
<td><img src="Assets/Documentation/Diagram/Pictorial Diagram.jpg" alt="pictorial-diagram"></td>
</tr>
</table>
<table>
<tr>
<th width="840" colspan="2">Prototype Design</th>
</tr>
<tr>
<td width="420"><img src="Assets/Documentation/Diagram/Prototype Design 1.jpg" alt="prototype-design-1"></td>
<td width="420"><img src="Assets/Documentation/Diagram/Prototype Design 2.jpg" alt="prototype-design-2"></td>
</tr>
</table>
<table>
<tr>
<th width="840" colspan="3">Conveyor System Blueprint</th>
</tr>
<tr>
<td width="280"><img src="Assets/Documentation/Diagram/Blueprint 1.jpg" alt="blueprint-1"></td>
<td width="280"><img src="Assets/Documentation/Diagram/Blueprint 2.jpg" alt="blueprint-2"></td>
<td width="280"><img src="Assets/Documentation/Diagram/Blueprint 3.jpg" alt="blueprint-3"></td>
</tr>
</table>
<table>
<tr>
<th width="840">Wiring</th>
</tr>
<tr>
<td><img src="Assets/Documentation/Table/Device Wiring.jpg" alt="wiring"></td>
</tr>
</table>

<br><br>

## CPR Calibration
<table><tr><td width="840">

```ino
// Library to read Magnetic/Optical Encoder with ESP32
#include <ESP32Encoder.h>

// Channel A of the encoder is connected to GPIO pin 34
#define encoderA 34

// Channel B of the encoder is connected to GPIO pin 35
#define encoderB 35

// Encoder object from the ESP32Encoder library
ESP32Encoder encoder;

// Encoder initial count (when starting calibration)
long startEncoderCount = 0;      

// Previous encoder count (to calculate delta)
long lastEncoderCount = 0;

// Difference between current and previous count
long deltaEncoderCount = 0;

// Count per Revolution (number of counts in one full revolution)
float CPR = 0;

// Pulse per Revolution (number of pulses of 1 channel in one revolution)
float PPR = 0;

// Gearbox ratio between motor and output shaft
float gearRatio = 0;

// PPR internal encoder (usually 11, depending on motor specifications)
const float encoderPPR_Internal = 11.0;

// Total estimated output shaft rotation
float totalOutputRotation = 0;

// Output shaft rotation target for calibration (default: 1 full rotation)
int outputRotationTarget = 1;

// Status of whether the calibration has been completed
bool calibrationDone = false;


// Function to display the guide on the Serial Monitor
void showInstructions() {
  Serial.println("================================================");
  Serial.println("                 CPR CALIBRATION                ");
  Serial.println("================================================");
  Serial.println("Steps:");
  Serial.println("1. Make sure the motor and encoder are connected.");
  Serial.println("2. Turn the OUTPUT shaft clockwise.");
  Serial.println("3. Rotate 1x full (360 degrees) steadily.");
  Serial.println("4. Wait for the calibration result to appear.");
  Serial.println("------------------------------------------------");
}


// The setup function will be executed once when the ESP32 board is powered on
void setup() {
  
  // Initialize Serial communication with baudrate 115200
  Serial.begin(115200);

  // Connect encoder with Full Quadrature method (4x resolution)
  encoder.attachFullQuad(encoderB, encoderA);

  // Reset encoder count to 0
  encoder.clearCount();

  // Save the initial count as a reference
  lastEncoderCount = encoder.getCount();

  // Also save as initial calibration value
  startEncoderCount = lastEncoderCount;

  while (!Serial) {
    ; // Wait for the Serial Monitor to be ready
  }

  // Delay to ensure Serial Monitor is completely ready
  delay(5000);
  
  // Display calibration instructions to user
  showInstructions();
}


// The loop function will be executed repeatedly (continuously)
void loop() {
  
  // If the calibration is complete, stop the loop (do nothing)
  if (calibrationDone) return;

  // Read the current encoder count value
  long currentCount = encoder.getCount();
  
  // Calculate the change from the last reading
  deltaEncoderCount = currentCount - lastEncoderCount;

  // Estimation of output rotation increment, based on encoder count change
  // Only if forward rotation occurs
  if (deltaEncoderCount > 0) {

    // Add to total output revolutions (500 is just an initial estimate)
    totalOutputRotation += deltaEncoderCount / 500.0;

    // If the total output rotation has reached the target (e.g. 1 full rotation)
    if (totalOutputRotation >= outputRotationTarget) {
      
      // Mark calibration complete
      calibrationDone = true;

      // Total encoder count for 1 revolution
      long totalCountsInOneRotation = currentCount - startEncoderCount;

      // Count Per Revolution
      CPR = (float)totalCountsInOneRotation;

      // Counts per pulse (due to Full Quad, divided by 4)
      PPR = CPR / 4.0;

      // Estimated gearbox ratio
      gearRatio = CPR / (encoderPPR_Internal * 4.0);

      // Round up results for easier reading
      int PPR_rounded = round(PPR);
      int CPR_rounded = round(CPR);
      int gearRatio_rounded = round(gearRatio);

      // Display calibration results
      Serial.println();
      Serial.println("================ CALIBRATION RESULT ===============");
      Serial.print("PPR (Pulse/Revolusi)   =  "); Serial.println(PPR_rounded);
      Serial.print("CPR (Count/Revolusi)   =  "); Serial.println(CPR_rounded);
      Serial.print("Gear Ratio (Motor:Out) =  1:"); Serial.println(gearRatio_rounded);
      Serial.println("================================================");
      Serial.println("✅ Calibration completed. Use the above values.");
    }
  }

  // Last encoder count update
  lastEncoderCount = currentCount;
}
```

</td></tr></table><br><br>

## Arduino IDE Setup
1. Open the ``` Arduino IDE ``` first, then open the project by clicking ``` File ``` -> ``` Open ``` : 

   <table><tr><td width="810">
   
      ``` Main.ino ```

   </td></tr></table><br>
   
2. Fill in the ``` Additional Board Manager URLs ``` in Arduino IDE

   <table><tr><td width="810">
      
      Click ``` File ``` -> ``` Preferences ``` -> enter the ``` Boards Manager Url ``` by copying the following link :
      
      ```
      https://dl.espressif.com/dl/package_esp32_index.json
      ```

   </td></tr></table><br>
   
3. ``` Board Setup ``` in Arduino IDE

   <table>
      <tr><th width="810">

      How to setup the ``` DOIT ESP32 DEVKIT V1 ``` board
            
      </th></tr>
      <tr><td>
      
      • Click ``` Tools ``` -> ``` Board ``` -> ``` Boards Manager ``` -> Install ``` esp32 ```. 
      
      • Then selecting a Board by clicking: ``` Tools ``` -> ``` Board ``` -> ``` ESP32 Arduino ``` -> ``` DOIT ESP32 DEVKIT V1 ```.

   </td></tr></table><br>
   
4. ``` Change the Board Speed ``` in Arduino IDE

   <table><tr><td width="810">
      
      Click ``` Tools ``` -> ``` Upload Speed ``` -> ``` 115200 ```

   </td></tr></table><br>
   
5. ``` Install Library ``` in Arduino IDE

   <table><tr><td width="810">
      
      Download all the library zip files. Then paste it in the: ``` C:\Users\Computer_Username\Documents\Arduino\libraries ```

   </td></tr></table><br>

6. ``` Port Setup ``` in Arduino IDE

   <table><tr><td width="810">
      
      Click ``` Port ``` -> Choose according to your device port ``` (you can see in device manager) ```

   </td></tr></table><br>

7. Change the ``` WiFi Name ```, ``` WiFi Password ```, and so on according to what you are currently using.<br><br>

8. Before uploading the program please click: ``` Verify ```.<br><br>

9. If there is no error in the program code, then please click: ``` Upload ```.<br><br>
    
10. Some things you need to do when using the ``` ESP32 board ``` :

    <table><tr><td width="810">
       
       • If ``` ESP32 board ``` cannot process ``` Source Code ``` totally -> Press ``` EN (RST) ``` button -> ``` Restart ```.

       • If ```ESP32 board ``` cannot process ``` Source Code ``` automatically then :<br>

      - When information: ``` Uploading... ``` has appeared -> immediately press and hold the ``` BOOT ``` button.<br>

      - When information: ``` Writing at .... (%) ``` has appeared -> release the ``` BOOT ``` button.

      • If message: ``` Done Uploading ``` has appeared -> ``` The previously entered program can already be operated ```.

      • Do not press the ``` BOOT ``` and ``` EN ``` buttons at the same time as this may switch to ``` Upload Firmware ``` mode.

    </td></tr></table><br>

11. If there is still a problem when uploading the program, then try checking the ``` driver ``` / ``` port ``` / ``` others ``` section.

<br><br>

## Ubidots Setup
Coming Soon...

<br><br>

## Get Started
1. Download and extract this repository.<br><br>
   
2. Make sure you have the necessary electronic components.<br><br>
   
3. Make sure your components are designed according to the diagram.<br><br>
   
4. Configure your device according to the settings above.<br><br>

5. Please enjoy [Done].

<br><br>

## Highlights
<table>
<tr>
<th width="840" colspan="2">Product</th>
</tr>
<tr>
<td width="420"><img src="Assets/Documentation/Experiment/Product 1.jpg" alt="product-1"></td>
<td width="420"><img src="Assets/Documentation/Experiment/Product 2.jpg" alt="product-2"></td>
</tr>
</table>
<table>
<tr>
<th width="840" colspan="3">Wi-Fi Connectivity</th>
</tr>
<tr>
<td width="280"><img src="Assets/Documentation/Experiment/Wi-Fi Connectivity 1.jpg" alt="wifi-connectivity-1"></td>
<td width="280"><img src="Assets/Documentation/Experiment/Wi-Fi Connectivity 2.jpg" alt="wifi-connectivity-2"></td>
<td width="280"><img src="Assets/Documentation/Experiment/Wi-Fi Connectivity 3.jpg" alt="wifi-connectivity-3"></td>
</tr>
</table>
<table>
<tr>
<th width="840" colspan="4">IoT Connectivity</th>
</tr>
<tr>
<td width="210"><img src="Assets/Documentation/Experiment/IoT Connectivity 1.jpg" alt="iot-connectivity-1"></td>
<td width="210"><img src="Assets/Documentation/Experiment/IoT Connectivity 2.jpg" alt="iot-connectivity-2"></td>
<td width="210"><img src="Assets/Documentation/Experiment/IoT Connectivity 3.jpg" alt="iot-connectivity-3"></td>
<td width="210"><img src="Assets/Documentation/Experiment/IoT Connectivity 4.jpg" alt="iot-connectivity-4"></td>
</tr>
</table>
<table>
<tr>
<th width="840" colspan="4">Publish-Subscribe MQTT</th>
</tr>
<tr>
<td width="210"><img src="Assets/Documentation/Experiment/Publish-Subscribe MQTT 1.jpg" alt="pubsub-mqtt-1"></td>
<td width="210"><img src="Assets/Documentation/Experiment/Publish-Subscribe MQTT 2.jpg" alt="pubsub-mqtt-2"></td>
<td width="210"><img src="Assets/Documentation/Experiment/Publish-Subscribe MQTT 3.jpg" alt="pubsub-mqtt-3"></td>
<td width="210"><img src="Assets/Documentation/Experiment/Publish-Subscribe MQTT 4.jpg" alt="pubsub-mqtt-4"></td>
</tr>
</table>
<table>
<tr>
<th width="840" colspan="5">LCD View</th>
</tr>
<tr>
<td width="168"><img src="Assets/Documentation/Experiment/LCD View 1.jpg" alt="lcd-view-1"></td>
<td width="168"><img src="Assets/Documentation/Experiment/LCD View 2.jpg" alt="lcd-view-2"></td>
<td width="168"><img src="Assets/Documentation/Experiment/LCD View 3.jpg" alt="lcd-view-3"></td>
<td width="168"><img src="Assets/Documentation/Experiment/LCD View 4.jpg" alt="lcd-view-4"></td>
<td width="168"><img src="Assets/Documentation/Experiment/LCD View 5.jpg" alt="lcd-view-5"></td>
</tr>
</table>
<table>
<tr>
<th width="420">Serial Monitor</th>
<th width="420">Serial Plotter</th>
</tr>
<tr>
<td><img src="Assets/Documentation/Experiment/Serial Monitor.jpg" alt="serial-monitor"></td>
<td><img src="Assets/Documentation/Experiment/Serial Plotter.jpg" alt="serial-plotter"></td>
</tr>
</table>
<table>
<tr>
<th width="420">Ubidots Controls and Indicators</th>
<th width="420">Ubidots Line Chart</th>
</tr>
<tr>
<td><img src="Assets/Documentation/Experiment/Ubidots Controls and Indicators.jpg" alt="control-indicators"></td>
<td><img src="Assets/Documentation/Experiment/Ubidots Line Chart.jpg" alt="line-chart"></td>
</tr>
</table>

<br><br>
<strong>More information:</strong><br>
<table><tr><td width="840">
   • Undergraduate Thesis : <a href="https://repository.upnjatim.ac.id/38675"><u>Access 1</u></a> or <a href="https://github.com/cakraawijaya/Motor-Speed-RPM-PID-Ziegler-Nichols-2-IoT/tree/master/Assets/Documentation/Report"><u>Access 2</u></a><br><br>
   • Journals : <a href="https://journal.citradharma.org/index.php/literasinusantara/article/view/1593"><u>Click Here</u></a>
</td></tr></table>

<br><br>

## Appreciation
If this work is useful to you, then support this work as a form of appreciation to the author by clicking the ``` ⭐Star ``` button at the top of the repository.

<br><br>

## Disclaimer
This application is the result of the hard work of my colleague named Hawin, not the result of plagiarism from other people's research or work, except those related to third-party services which include: libraries, frameworks, and so on. In this project I only act as a supervisor. The publication of this work has obtained permission from the parties concerned in accordance with what was agreed at the beginning for the development of science.

<br><br>

## LICENSE
MIT License - Copyright © 2025 - Moch Hawin Hamami & Devan C. M. Wijaya, S.Kom

Permission is hereby granted without charge to any person obtaining a copy of this software and the software-related documentation files to deal in them without restriction, including without limitation the right to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons receiving the Software to be furnished therewith on the following terms:

The above copyright notice and this permission notice must accompany all copies or substantial portions of the Software.

IN ANY EVENT, THE AUTHOR OR COPYRIGHT HOLDER HEREIN RETAINS FULL OWNERSHIP RIGHTS. THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, THEREFORE IF ANY DAMAGE, LOSS, OR OTHERWISE ARISES FROM THE USE OR OTHER DEALINGS IN THE SOFTWARE, THE AUTHOR OR COPYRIGHT HOLDER SHALL NOT BE LIABLE, AS THE USE OF THE SOFTWARE IS NOT COMPELLED AT ALL, SO THE RISK IS YOUR OWN.
