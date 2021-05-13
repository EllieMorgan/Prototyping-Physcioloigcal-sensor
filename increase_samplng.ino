#include <ArduinoBLE.h>
#include <SparkFun_Bio_Sensor_Hub_Library.h>
#include <Wire.h>

// Reset pin, MFIO pin
int resPin = A2;
int mfioPin = A1;

// Possible widths: 69, 118, 215, 411us
int width = 411;
// Possible samples: 50, 100, 200, 400, 800, 1000, 1600, 3200 samples/second
// Not every sample amount is possible with every width; check out our hookup
// guide for more information.
int samples = 100;
int pulseWidthVal;
int sampleVal;
int Sample_number = 10;
int HR_Average = 0;
int loop_count = 0;
int Ox_sample = 0;
int Ox_Average = 0;
int run_once = 1;


// Takes address, reset pin, and MFIO pin.
SparkFun_Bio_Sensor_Hub bioHub(resPin, mfioPin);

bioData body;
// ^^^^^^^^^
// What's this!? This is a type (like "int", "byte", "long") unique to the SparkFun
// Pulse Oximeter and Heart Rate Monitor. Unlike those other types it holds
// specific information on the LED count values of the sensor and ALSO the
// biometric data: heart rate, oxygen levels, and confidence. "bioLedData" is
// actually a specific kind of type, known as a "struct". I chose the name
// "body" but you could use another variable name like "blood", "readings",
// "ledBody" or whatever. Using the variable in the following way gives the
// following data:
// body.irLed      - Infrared LED counts.
// body.redLed     - Red LED counts.
// body.heartrate  - Heartrate
// body.confidence - Confidence in the heartrate value
// body.oxygen     - Blood oxygen level
// body.status     - Has a finger been sensed?


BLEService customService("1101");
BLEUnsignedIntCharacteristic spo2("2101", BLERead | BLENotify);

unsigned long last_time;


void setup() {
  Serial.begin(115200);


  Wire.begin();
  int result = bioHub.begin();
  if (result == 0) // Zero errors!
    Serial.println("Sensor started!");

  Serial.println("Configuring Sensor....");
  int error = bioHub.configSensorBpm(MODE_ONE); // Configure Sensor and BPM mode , MODE_TWO also available
  if (error == 0) { // Zero errors.
    Serial.println("Sensor configured.");
  }
  else {
    Serial.println("Error configuring sensor.");
    Serial.print("Error: ");
    Serial.println(error);
  }

  // Set pulse width.
  error = bioHub.setPulseWidth(width);
  if (error == 0) { // Zero errors.
    Serial.println("Pulse Width Set.");
  }
  else {
    Serial.println("Could not set Pulse Width.");
    Serial.print("Error: ");
    Serial.println(error);
  }

  // Check that the pulse width was set.
  pulseWidthVal = bioHub.readPulseWidth();
  Serial.print("Pulse Width: ");
  Serial.println(pulseWidthVal);

  // Set sample rate per second. Remember that not every sample rate is
  // available with every pulse width. Check hookup guide for more information.
  error = bioHub.setSampleRate(samples);
  if (error == 0) { // Zero errors.
    Serial.println("Sample Rate Set.");
  }
  else {
    Serial.println("Could not set Sample Rate!");
    Serial.print("Error: ");
    Serial.println(error);
  }

  // Check sample rate.
  sampleVal = bioHub.readSampleRate();
  Serial.print("Sample rate is set to: ");
  Serial.println(sampleVal)

  // Data lags a bit behind the sensor, if you're finger is on the sensor when
  // it's being configured this delay will give some time for the data to catch
  // up.
  Serial.println("Loading up the buffer with data....");
  delay(4000);




  pinMode(LED_BUILTIN, OUTPUT);

  if (!BLE.begin()) {
    Serial.println("BLE failed to Initiate");
    delay(500);
    while (1);
  }

  BLE.setLocalName("Arduino oxygen sensor");
  BLE.setAdvertisedService(customService);
  customService.addCharacteristic(spo2);
  BLE.addService(customService);


  BLE.advertise();

  Serial.println("Bluetooth device is now active, waiting for connections...");
  digitalWrite(25, LOW);
}


void loop() {

  BLEDevice central = BLE.central();
  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    //digitalWrite(LED_BUILTIN, HIGH);

    while (central.connected()) {



        body = bioHub.readSensorBpm();
        //while (body.status != 3) {
          body = bioHub.readSensorBpm();
          delay(50);
        
        HR_Average = 0;
        Ox_Average = 0;
        HR_Average = HR_Average + body.heartRate;
        Ox_Average = Ox_Average + body.oxygen;
        spo2.writeValue(HR_Average);
        spo2.writeValue(Ox_Average);
        run_once = 0;


        // Below not needed as not connected to PC
        //    Serial.print("Heartrate: ");
        //    Serial.println(body.heartRate);
        //    Serial.print("Confidence: ");
        //    Serial.println(body.confidence);
        //    Serial.print("Blood Oxygen: ");
        //    Serial.println(body.oxygen);
        //     Serial.print("Status: ");
        //    Serial.println(body.status);
        //    // Slow it down or your heart rate will go up trying to keep up
        //    // with the flow of numbers
        //
        //    delay(250);
       
      }
    }
  

  //digitalWrite(LED_BUILTIN, LOW);
  Serial.print("Disconnected from central: ");
  Serial.println(central.address());
}
