#include <SoftwareSerial.h>
 
#define DEBUG false
#define rkey 1019 

 
SoftwareSerial esp8266(2,3); // make RX Arduino line is pin 2, make TX Arduino line is pin 3.
                             // This means that you need to connect the TX line from the esp to the Arduino's pin 2
                             // and the RX line from the esp to the Arduino's pin 3

char pin[] = "pin=";    
char ipd[] = "+IPD,";
char key[] = "key=";
void setup()
{
  Serial.begin(9600);
  esp8266.begin(9600); // your esp's baud rate might be different
  
  pinMode(11,OUTPUT);
  digitalWrite(11,LOW);
  
  pinMode(12,OUTPUT);
  digitalWrite(12,LOW);
  /*
   pinMode(10,OUTPUT);
  digitalWrite(10,LOW);
  */ 
 
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
 
 // digitalWrite(11, HIGH);
  sendCommand("AT+RST\r\n",2000,DEBUG); // reset module
  sendCommand("AT+CWMODE=3\r\n",1000,DEBUG); // configure as access point
  sendCommand("AT+CWJAP=\"projectfi\",\"akhila123\"\r\n",3000,DEBUG);
  
  delay(10000);
  
  sendCommand("AT+CIFSR\r\n",1000,DEBUG); // get ip address
  sendCommand("AT+CIPMUX=1\r\n",1000,DEBUG); // configure for multiple connections
  sendCommand("AT+CIPSERVER=1,80\r\n",1000,DEBUG); // turn on server on port 80
  
  Serial.println("Server Ready");
}
 
void loop()
{
    if(esp8266.available()) // check if the esp is sending a message 
    {
        if(esp8266.find(ipd))
        {
            delay(2000); // wait for the serial buffer to fill up (read all the serial data)
                        // get the connection id so that we can then disconnect
            int connectionId = esp8266.read()-48; // subtract 48 because the read() function returns 
                                           // the ASCII decimal value and 0 (the first decimal number) starts at 48
          
            esp8266.find(pin); // advance cursor to "pin="
            int pinNumber = 0; 
            int publicKey = 0;  
            int a = 0;
            int i = 0, j = 0;
            int num[10];

            a = (esp8266).read();
     
            do
            {
                num[i] = a - 48;
                i++;
                a = (esp8266).read();            
            
            }while(a>=48 && a<=57);
            i = i-1;
            while(i>=0) 
            {
                a = num[i] * findPower(10,j);;
                pinNumber+= a;
                j++;
                i--;
        
            }
          Serial.print("Encrypted Data: ");
          Serial.println(pinNumber);

          (esp8266).read();
          (esp8266).read();
          (esp8266).read();
          (esp8266).read();
          a = 0;
          i = 0;
          j = 0; 

          a = (esp8266).read();

          do
          {
              num[i] = a - 48;  
              i++;
              a = (esp8266).read();
          }while(a>=48 && a<=57);

          i = i-1;
          while(i>=0) 
          {
                a = num[i] * findPower(10,j);;
                publicKey+= a;
                j++;
                i--;
        
          }

          Serial.print("Public Key: ");
          Serial.println(publicKey);
            
          pinNumber = decryptMessage(pinNumber, publicKey);

          Serial.print("Decrypted Data: ");
          Serial.println(pinNumber);
     
     
      
     
     
        if(pinNumber>13 || pinNumber<11)
            pinNumber = 10;
        
        digitalWrite(pinNumber, !digitalRead(pinNumber)); // toggle pin    
     
     // build string that is send back to device that is requesting pin toggle
        String content;
        content = "Pin ";
        content += pinNumber;
        content += " is ";

        if(pinNumber<11 || pinNumber>13)
            content = "ERROR";      
        else 
        {
            if(digitalRead(pinNumber))
            {
              content += "ON";
            }
            else
            {
              content += "OFF";
            }
        }  
     
        sendHTTPResponse(connectionId,content);
     
     // make close command
        String closeCommand = "AT+CIPCLOSE="; 
        closeCommand+=connectionId; // append connection id
        closeCommand+="\r\n";
        sendCommand(closeCommand,1000,DEBUG); // close connection
      }
    }
}


int decryptMessage(int data, int publicKey) 
{

    int i;
    unsigned long temp = 0;
    int mod;
    unsigned long val;
    int power;
    int num;
    
    mod = publicKey;
    val = data;
    num = data;
    power = rkey;

    /*Serial.println(mod);
    Serial.println(val);
    Serial.println(power);
    */
    for(i = 1; i<power; i++) {
        
        temp = num * val; 
        val = temp % mod;
       /*if(i%100 == 0){
        Serial.println(temp);
        Serial.println(val);
        
       }*/
    }
    return val; 
}


int findPower(int base, int exp) 
{
    int i;
    int val = 1;
    for(i = 0; i<exp; i++) 
    {
        val = val * base;
    }
    return val;
}

 
/*
* Name: sendData
* Description: Function used to send data to ESP8266.
* Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
* Returns: The response from the esp8266 (if there is a reponse)
*/
String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    
    int dataSize = command.length();
    char data[dataSize];
    command.toCharArray(data,dataSize);
           
    esp8266.write(data,dataSize); // send the read character to the esp8266
    if(debug)
    {
      Serial.println("\r\n====== HTTP Response From Arduino ======");
      Serial.write(data,dataSize);
      Serial.println("\r\n========================================");
    }
    
    long int time = millis();
    
    while( (time+timeout) > millis())
    {
      while(esp8266.available())
      {
        
        // The esp has data so display its output to the serial window 
        char c = esp8266.read(); // read the next character.
        response+=c;
      }  
    }
    
    if(debug)
    {
      Serial.print(response);
    }
    
    return response;
}
 
/*
* Name: sendHTTPResponse
* Description: Function that sends HTTP 200, HTML UTF-8 response
*/
void sendHTTPResponse(int connectionId, String content)
{
     
     // build HTTP response
     String httpResponse;
     String httpHeader;
     // HTTP Header
     httpHeader = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n"; 
     httpHeader += "Content-Length: ";
     httpHeader += content.length();
     httpHeader += "\r\n";
     httpHeader +="Connection: close\r\n\r\n";
     httpResponse = httpHeader + content + " "; // There is a bug in this code: the last character of "content" is not sent, I cheated by adding this extra space
     sendCIPData(connectionId,httpResponse);
}
 
/*
* Name: sendCIPDATA
* Description: sends a CIPSEND=<connectionId>,<data> command
*
*/
void sendCIPData(int connectionId, String data)
{
   String cipSend = "AT+CIPSEND=";
   cipSend += connectionId;
   cipSend += ",";
   cipSend +=data.length();
   cipSend +="\r\n";
   sendCommand(cipSend,1000,DEBUG);
   sendData(data,1000,DEBUG);
}
 
/*
* Name: sendCommand
* Description: Function used to send data to ESP8266.
* Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
* Returns: The response from the esp8266 (if there is a reponse)
*/
String sendCommand(String command, const int timeout, boolean debug)
{
    String response = "";
           
    esp8266.print(command); // send the read character to the esp8266
    
    long int time = millis();
    
    while( (time+timeout) > millis())
    {
      while(esp8266.available())
      {
        
        // The esp has data so display its output to the serial window 
        char c = esp8266.read(); // read the next character.
        response+=c;
      }  
    }
    
    if(debug)
    {
      Serial.print(response);
    }
    
    return response;
}
 

