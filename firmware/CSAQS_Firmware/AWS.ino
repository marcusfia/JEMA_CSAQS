
//Load in AWS certificates to permit usage of AWS network
void LoadAWSCertificate()
{
  Serial.print("Heap: "); Serial.println(ESP.getFreeHeap());

  /* Load AWS certificate key from SPIFFS*/
  File cert = SPIFFS.open("/cert.der", "r"); 
  String a = "Failed to open cert file";
  String b = "Successfully opened cert file";
  String c = (!cert) ? a : b;
  if(DEBUG_OUT) Serial.println(c);

  delay(100);
  
  // succesfully loaded?
  a = "cert loaded";
  b = "vert not loaded";
  c = (espClient.loadCertificate(cert)) ? a : b;
  if(DEBUG_OUT) Serial.println(c);
  
  /* Load AWS private key from SPIFFS */
  File private_key = SPIFFS.open("/private.der", "r"); 
  a = "private key not loaded";
  b = "private key loaded";
  c = (!private_key) ? a : b;
  if(DEBUG_OUT) Serial.println(c);

  delay(100);

  // succesfully loaded?
  a = "private key loaded";
  b = "private key not loaded";
  c = (espClient.loadPrivateKey(private_key)) ? a : b;
  if(DEBUG_OUT) Serial.println(c);

  /* Load AWS Cert Authority file from SPIFFS */
  File ca = SPIFFS.open("/ca.der", "r"); 
  a = "Cert Authority file not loaded";
  b = "Cert Authority file loaded";
  c = (!ca) ? a : b;
  if(DEBUG_OUT) Serial.println(c);

  delay(100);

  // succesfully loaded?
  a = "Cert Authority loaded";
  b = "Cert Authority not loaded";
  c = (espClient.loadCACert(ca)) ? a : b;
  if(DEBUG_OUT)
  {
    Serial.println(c);
    Serial.print("Heap: "); Serial.println(ESP.getFreeHeap());
  }
}
