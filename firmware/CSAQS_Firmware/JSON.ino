/**** RETURN ADDRESS CLIENT ID ********/
String transmit_createClientID() 
{
  return AWS_IOT_CORE_CLIENT_ID;
}


void BuildJSONPayload()
{


  /* get current UTC time */
  timeClient.update();
 
  int timeStamp = timeClient.getEpochTime();
  
  /* set expirey time for object to 600 seconds after object capture time for testing TTL use in DB */
  int TTL = timeStamp + 30; 
  
  delay(100);
  
  //Clear the string buffer
  payload = "";
  /* Concatenate payload variables into String payload */
  payload = "{";
  payload += "\"clientID\":";    payload += "\""; payload += db_device_ID;
  payload += "\"";               payload += ",";
  payload += "\"timeID\":";    payload += timeStamp      ; payload += ",";
  
  /*                                                         ADC (POWER)    */
  payload += "\"vcc\":";         payload += vcc            ; payload += ",";

  
  /*                                                         ADC (NOx)      */
  payload += "\"nox\":";         payload += noxSensorRead  ; payload += ",";
  
  /*                                                         UBLOX-GPS      */
  payload += "\"latitude\":";    payload += latitude       ; payload += ",";
  payload += "\"longitude\":";   payload += longitude      ; payload += ",";
  payload += "\"altitude\":";    payload += altitude       ; payload += ",";
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
