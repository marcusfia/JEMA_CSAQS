//Initial wifi connection with stored credentials
void SetupWifiConnection() 
{
  delay(10);
  // We start by connecting to a WiFi network
  espClient.setBufferSizes(512, 512);
  //Connects to the Wifi newtwork that is saved in Memory or goes to AP mode
  wifiManager.autoConnect("Jema");
  
  if(DEBUG_OUT) Serial.println("WiFi connected. IP address: " + WiFi.localIP());

  timeClient.begin();
  while(!timeClient.update())
  {
    timeClient.forceUpdate();
  }
  espClient.setX509Time(timeClient.getEpochTime());
}
