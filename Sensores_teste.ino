#include <Wire.h>
#include "SparkFun_SCD30_Arduino_Library.h"
#include "SparkFun_SCD4x_Arduino_Library.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_CCS811.h"
#include "Adafruit_SGP30.h"
#include <Arduino.h>
#include <pas-co2-serial-ino.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////
// Quem liga onde:
// Alimentação 3.3V  -> CCS811 e PAS CO2
// Alimentação 5.0V  -> SCD 30, SCD 41 e SGP 30
// Alimentação 12.0V -> PAS CO2 com terra comum a todos (saida vermelha vai no 12V e saida preta da fonte vai no GND da plaquinha, tds os nomes estão nos fios embaixo).
// 1 Barramento p/ SCL pra todos
// 1 Barramento p/ SDA pra todos
// 1 Barramento Terra/GND pra todos
/////////////////////////////////////////////////////////////////////////////////////////////

SCD30 airSensor;              //SCD30
SCD4x mySensor;               //SCD41
Adafruit_CCS811 ccs;          //CCS811
Adafruit_SGP30 sgp;           //SGP30

#define I2C_FREQ_HZ 400000    // PASCO2
PASCO2SerialIno cotwo;        //PASCO2

int16_t co2ppm; //PASCO2
Error_t err; //PASCO2


//SGP30
uint32_t getAbsoluteHumidity(float temperature, float humidity)
{
  // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
  const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
  const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
  return absoluteHumidityScaled;
}

bool flag = true;
byte count = 0;
int j = 0;
int counter = 0;


void setup()
{
  delay(10000);

  //SCD30
  Wire.beginTransmission(97);
  Serial.begin(115200);
  Wire.begin();

  if (airSensor.begin() == false)
  {
    Serial.println("Failed to start sensor! Please check your wiring. SCD30");
    flag = false;
  }

  //SCD41
  if (mySensor.begin() == false)
  {
    Serial.println(F("Failed to start sensor! Please check your wiring. SCD41"));
  }

  //CCS811
  if (!ccs.begin())
  {
//    Serial.println("Failed to start sensor! Please check your wiring. CCS811");
  }

  //SGP30
  if (!sgp.begin())
  {
    Serial.println("Failed to start sensor! Please check your wiring. SGP30");
  }

  delay(500);

  Wire.begin(40);
  Wire.setClock(I2C_FREQ_HZ);

  // PASCO2
  err = cotwo.begin();
  if (XENSIV_PASCO2_OK != err)
  {
    Serial.print("initialization error: ");
    Serial.println(err);
    Serial.println("Failed to start sensor! Please check your wiring. PASCO2");
  }

  Serial.println("Sensor,CO2(ppm),Temperatura (C),umidade (%),TVOC,Data");
  delay(500);
}

void loop()
{
  /////////////////////////////////////////////////////////////////////////////////
  //SCD30
  if (flag)
  {
    if (airSensor.dataAvailable())
    {
      float fTempSCD30 = airSensor.getTemperature();
      float fCo2SCD30 = airSensor.getCO2();
      float fUmidadeSCD30 = airSensor.getHumidity();

      Serial.println( (String) "SCD30, " + fCo2SCD30 + "," + fTempSCD30 + "," + fUmidadeSCD30 + "," + "TVOC" );
    }
  }

  /////////////////////////////////////////////////////////////////////////////////
  //SCD41
  {
    Wire.beginTransmission(98);
  }
  if (mySensor.readMeasurement())
  {
    float fCo2SCD41 = mySensor.getCO2();
    float fTempSCD41 = mySensor.getTemperature();
    float fUmidadeSCD41 = mySensor.getHumidity();

    Serial.println( (String) "SCD41, " + fCo2SCD41 + "," + fTempSCD41 + "," + fUmidadeSCD41 + "," + "TVOC" );
  }
  delay(100);

  /////////////////////////////////////////////////////////////////////////////////
  //CCS811
  if (ccs.available())
  {
    if (!ccs.readData()) //inicia a leitura dos dados
    {
      float fCo2CCS811 = ccs.geteCO2(); // definir uma variável para a função
      float fTvocCCS811 = ccs.getTVOC(); // definir uma variável para a função

      Serial.println( (String) "CCS811, " + fCo2CCS811 + "," + "Temperatura" + "," + "Umidade" + "," + fTvocCCS811 );
    }
    delay(100);
  }

  /////////////////////////////////////////////////////////////////////////////////
  //SGP30
  //if (! sgp.IAQmeasure())
  //{ delay(5000);
  //  Serial.println("Measurement failed. SGP30");
  //  return;

  // }
  // if (! sgp.IAQmeasureRaw())
  // { delay(5000);
  //   Serial.println("Raw Measurement failed. SGP30");
  //   return;
  //  }

  if (sgp.IAQmeasure()) {

    float fCo2SGP30 = sgp.eCO2;
    float fTvocSGP30 = sgp.TVOC;

    Serial.println( (String) "SGP30, " + fCo2SGP30 + "," + "Temperatura" + "," + "Umidade" + "," + fTvocSGP30);

  }
  //delay(2000);

  counter++;
  if (counter == 30)
  {
    counter = 0;

    uint16_t TVOC_base, eCO2_base;
    if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base))
    {
      // Serial.println("Failed to get baseline readings. SGP30");
      return;
    }
  }

  /////////////////////////////////////////////////////////////////////////////////
  //  PASCO2
  err = cotwo.startMeasure();
  if (XENSIV_PASCO2_OK != err)
  {
    Serial.print("error: ");
  }
  //Serial.println(err);

  delay(2000);
  co2ppm = 0;

  do
  {
    err = cotwo.getCO2(co2ppm);
    if (XENSIV_PASCO2_OK != err)
    {
      Serial.print("error: A ");
      Serial.println(err);
      break;
    }
  }
  while (0 == co2ppm); {
    Serial.println( (String) "PASCO2, " + (co2ppm) + "," + "Temperatura" + "," + "Umidade" + "," + "TVOC" );
  }
  delay(5000);
}
