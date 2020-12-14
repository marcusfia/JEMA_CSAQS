/**** RETURN CLIENT ID - USED AS DB PARTITION KEY ********/
String transmit_createClientID() 
{
  return AWS_IOT_CORE_CLIENT_ID;
}


void BuildJSONPayload()
{


  /* get current UTC time */
  timeClient.update();
  int timeStamp = timeClient.getEpochTime();

  SIV = 8;
  
  delay(100);
  
  //Clear the string buffer
  payload = "";
  /* Concatenate payload variables into String payload */
  payload = "{";
  payload += "\"clientID\":";    payload += "\""; payload += db_device_ID;
  payload += "\"";               payload += ",";
  
  payload += "\"dateTime\":";    payload += timeStamp      ; payload += ",";
  
  /*                                                         ADC (POWER)    */
  payload += "\"vcc\":";         payload += vcc            ; payload += ",";
  
  /*                                                         ADC (NOx)      */
  payload += "\"nox\":";         payload += noxSensorRead  ; payload += ",";

  //modified payload for MARCUS hardcoded gps (lat: 416545630, lon: -915309680, altitude: 200000mm) and ERIC (lat: 417419460, lon: -916102450, altitude: 238000mm)
  /*                                                         UBLOX-GPS      */
  payload += "\"latitude\":";    payload += "ADD MARCUS or ERIC lat here"    ; payload += ",";
  payload += "\"longitude\":";   payload += "ADD MARCUS or ERIC lon here"   ; payload += ",";
  payload += "\"altitude\":";    payload += "ADD MARCUS or ERIC altitude here"           ; payload += ",";
  payload += "\"SIV\":";         payload += SIV            ; payload += ",";

  /*                                                         BME680         */
  payload += "\"temp\":";        payload += tempStr        ; payload += ",";
  payload += "\"humidity\":";    payload += humidityStr    ; payload += ",";
  payload += "\"pressure\":";    payload += pressureStr    ; payload += ",";
  payload += "\"voc\":";         payload += gasStr         ; payload += ",";
                     
  /*                                                         MAX30105       */
  payload += "\"red\":";         payload += red            ; payload += ",";
  payload += "\"green\":";       payload += green          ; payload += ",";
  payload += "\"IR\":";          payload += IR             ; 
     
  payload += "}"; 
       
}
