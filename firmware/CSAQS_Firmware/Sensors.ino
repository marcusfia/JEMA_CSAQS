
void InitializeBME680()
{
  bme680Enabled = false;
  if(BME680.begin(I2C_STANDARD_MODE)) // Start BME680 using I2C, use first device found
  {
    if(DEBUG_OUT) Serial.println("BME680 initialized successfully, I2C address = 0x" + String(BME680.getI2CAddress(), HEX));  
    BME680.setOversampling(TemperatureSensor,Oversample16); // Use enumerated type values
    BME680.setOversampling(HumiditySensor,   Oversample8); // Use enumerated type values
    BME680.setOversampling(PressureSensor,   Oversample16); // Use enumerated type values
    BME680.setIIRFilter(IIR4); // Use enumerated type values
    BME680.setGas(320,150); //  for 150 milliseconds
    bme680Enabled = true;
  }
  else if(DEBUG_OUT) Serial.println("Unable to initialize BME 680.");
  delay(1000);
}


void InitialzieMAX30105()
{
  max30105Enabled = false;
  if(particleSensor.begin() == true)
  {    
    particleSensor.setup(); //Configure sensor. Use 6.4mA for LED drive
    if(DEBUG_OUT) Serial.println("MAX30105 initialized successfully. getRevisionID() = " + String(particleSensor.getRevisionID()) + ", getPartID() = " + String(particleSensor.readPartID()));
    max30105Enabled = true;
  }
  else if(DEBUG_OUT) Serial.println("Unable to initialize MAX30105.");
  delay(1000);
}


void InitializeNEOM8U()
{
  gpsEnabled = false;
  if(ubloxGPSObj.begin() == true) //Connect to the Ublox module using Wire port
  {
    delay(1000);
    ubloxGPSObj.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
    if(DEBUG_OUT)
    {
      delay(1000);
      Serial.println("u-blox NEO-M8U initialzied successfully.  u-blox.IsConnected() = " + String(ubloxGPSObj.isConnected(100)));
      Serial.println("u-blox version = " + String(ubloxGPSObj.getProtocolVersionHigh()) + "." + String(ubloxGPSObj.getProtocolVersionLow()));
    }
    gpsEnabled = true;
  }
  else if(DEBUG_OUT) Serial.println("unable to initialize u-blox NEO-M8U.");
}


void PollPowerSupply()
{
  digitalWrite(MUX_SELECT, PWR_SUPPLY);
  delay(100);  //allow time for signal to settle
  read_A0 = SampleAnalogInput();
  vcc = ((float)read_A0 * (voltage_divider_scalar / ADC_resolution));
  if(DEBUG_OUT)
  {
    Serial.println("Power Supply A0 value: " + String(read_A0));
    Serial.println("Power Supply Voltage:  " + String(vcc));
  }
}


void PollNOxSensor()
{
  digitalWrite(MUX_SELECT, NOX_SENSOR);
  delay(100);  //allow time for signal to settle
  read_A0 = SampleAnalogInput();
  noxSensorRead = ((float)read_A0 * (NOx_voltage_divider_scalar / ADC_resolution));
  if(DEBUG_OUT)
  {
    Serial.println("NOx A0 value:       " + String(read_A0));
    Serial.println("NOx Sensor Voltage: " + String(noxSensorRead));
  }
}


void PollBME680()
{
  tempStr     = "";
  humidityStr = "";
  pressureStr = "";
  gasStr      = "";
  
  if(bme680Enabled)
  { 
    BME680.getSensorData(temp, humidity, pressure, gas);
    tempStr     = String(((float)temp)/100);
    humidityStr = String(((float)humidity)/1000);
    pressureStr = String(((float)pressure)/100);
    gasStr      = String((float)gas);
  }
  if(DEBUG_OUT)
  {
    Serial.println("temp:     " + tempStr);
    Serial.println("humidity: " + humidityStr);
    Serial.println("pressure: " + pressureStr);
    Serial.println("gas:      " + gasStr);
  }
}


void PollMAX30105()
{
  red   = "";
  green = "";
  IR    = "";
  if(max30105Enabled)
  {
    red   = String(particleSensor.getRed());
    green = String(particleSensor.getGreen());
    IR    = String(particleSensor.getIR());
  }
  if(DEBUG_OUT)
  {
     Serial.println("red:   " + red);
     Serial.println("green: " + green);
     Serial.println("IR:    " + IR);
  }
}


void PollNEOM8U()
{
  latitude  = "";
  longitude = "";
  altitude  = "";
  SIV = 0;
  if(gpsEnabled)
  {
    latitude  = String(ubloxGPSObj.getLatitude());
    delay(100);
    longitude = String(ubloxGPSObj.getLongitude());
    delay(100);
    altitude  = String(ubloxGPSObj.getAltitude());
    delay(100);
    SIV  = ubloxGPSObj.getSIV();
  }
  if(DEBUG_OUT)
  {
    Serial.println("latitude:  " + latitude);
    Serial.println("longitude: " + longitude);
    Serial.println("altitude:  " + altitude);
    Serial.println("SIV:       " + String(SIV));
  }
}
