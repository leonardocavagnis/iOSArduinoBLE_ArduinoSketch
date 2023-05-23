/*
  iOSArduinoBLE

  Board: Arduino Nano 33 BLE Sense

  by Leonardo Cavagnis
*/

#include <ArduinoBLE.h>
#include <Arduino_HTS221.h>

// Services and characteristics
BLEService ledService("cd48409a-f3cc-11ed-a05b-0242ac120003");
BLEByteCharacteristic ledstatusCharacteristic("cd48409b-f3cc-11ed-a05b-0242ac120003", BLEWrite);

BLEService sensorService("d888a9c2-f3cc-11ed-a05b-0242ac120003");
BLEByteCharacteristic temperatureCharacteristic("d888a9c3-f3cc-11ed-a05b-0242ac120003", BLERead | BLENotify);

int temperature = 0;
unsigned long startMillis = 0;
unsigned long currentMillis = 0;

void setup() {
  Serial.begin(115200);

  // LED pin initialization
  pinMode(LED_BUILTIN, OUTPUT);

  // Humidity/Temperature sensor initialization
  if (!HTS.begin()) {
    Serial.println("Failed to initialize humidity temperature sensor!");
    while (1);
  }

  // BLE initialization
  if (!BLE.begin()) {
    Serial.println("Failed to initialize BLE module!");
    while (1);
  }

  // set advertised local name and service UUID
  BLE.setLocalName("iOSArduinoBoard");
  BLE.setAdvertisedService(ledService);

  // add the characteristics to the services
  ledService.addCharacteristic(ledstatusCharacteristic);
  sensorService.addCharacteristic(temperatureCharacteristic);

  // add services to BLE stack
  BLE.addService(ledService);
  BLE.addService(sensorService);

  // set read request handler for temperature characteristic
  temperatureCharacteristic.setEventHandler(BLERead, temperatureCharacteristicRead);

  // start advertising
  BLE.advertise();

  startMillis = millis();
  Serial.println("Start advertising");
}

void loop() {  
  // listen for BLE centrals to connect
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral
  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    while (central.connected()) {
      currentMillis = millis();
      if (currentMillis - startMillis >= 1000) {
        // read temperature value every 1 second
        temperature = (int) HTS.readTemperature();
        // update temperature value in temperature characteristic
        temperatureCharacteristic.writeValue(temperature);
        startMillis = currentMillis;  
      }
      
      // check LedStatus characteristic write
      if (ledstatusCharacteristic.written()) {
        if (ledstatusCharacteristic.value()) {
          digitalWrite(LED_BUILTIN, HIGH);
        } else {
          digitalWrite(LED_BUILTIN, LOW);
        }
      }
    }
  }
}

/* Read request handler for temperatureCharacteristic */
void temperatureCharacteristicRead(BLEDevice central, BLECharacteristic characteristic) {
  temperatureCharacteristic.writeValue(temperature);
}
