
//Will take the existing file and add a payload too it
void AppendToFile(){

    //Opens The file
    File file = SPIFFS.open("/storage.txt", "a");
    //Checks to see if the file is opened
    if(!file){
        if(DEBUG_OUT) Serial.println("Error opening File");
        return;
    }
    //Checks to see if the file is empty or not
    if(file.size() == 0){
    //if it is empty adds the payload to the file to be sent later
    if(file.println(payload)){
        if(DEBUG_OUT)Serial.println("File was appended");
    } else {
       if(DEBUG_OUT) Serial.println("File append failed");
    }
    }else{
      //if the file was not empty adds  a ~ and then adds the next payload to the file
      String tempPayload = "~" + payload;
      if(file.println(tempPayload)){
       if(DEBUG_OUT) Serial.println("File was appended");
       } else {
       if(DEBUG_OUT) Serial.println("File append failed");
       }
    }
    file.close(); 
}
//opens the file and erases it
void eraseFile(){
  File file = SPIFFS.open("/storage.txt", "w");
  if (!file) {
    if(DEBUG_OUT)Serial.println("Error opening file for writing");
    return;
  }
  if(DEBUG_OUT)file.print("");
  file.close();

}
//Opens the File and reads it if it is empty closes the file if it has payload inside it is will send them to AWS
void fileRead(){
  File file = SPIFFS.open("/storage.txt", "r");
  if(DEBUG_OUT) Serial.println("reading File");
  if (!file) {
    if(DEBUG_OUT) Serial.println("Failed to open file for reading");
    return;
  }
  if(file.size() == 0){
     if(DEBUG_OUT) Serial.println("File is Empty");
     file.close();
    return;
  }
  if(DEBUG_OUT) Serial.println(file.size());
  
 String a = "";
 while (file.available()) {
   char currentChar =  (char)file.read();
   if(currentChar == '~') {
      Serial.print(a);
      Serial.println("");
      sendPayloadFromFile(a);
      delay(6000);
      a = "";  
   }
   else{
      a += currentChar;
   }
  }
  if(a != ""){
    Serial.print(a);
    sendPayloadFromFile(a);
    delay(6000);
    a = "";
  }
  file.close();
  eraseFile();
}



//transforms a string to a payload which can be sent 
void sendPayloadFromFile(String filePayload)
{
   filePayload.toCharArray(msg, MQTT_PAYLOAD_SIZE);
   if(!DEBUG_MODE)
   {
     // publish msg to AWS topic:
     int payLoadLength = filePayload.length();
     int result = client.publish(AWS_IOT_CORE_PUBLISH_TOPIC, msg);
     if(DEBUG_OUT) Serial.println("Client id = " + db_device_ID + ", Payload size = " + String(payLoadLength) + ",client.publish = " + String(result) + "\r\n");
   }
   else if(DEBUG_OUT) Serial.println("*DEBUG_MODE ACTIVE* No Payload Sent!\r\n");
   memset(msg, 0, sizeof(msg));   //empty msg buffer
}
