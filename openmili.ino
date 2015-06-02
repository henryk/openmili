#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <printf.h>

#include "PL1167_nRF24.h"
#include "MiLightRadio.h"

#define CE_PIN 9
#define CSN_PIN 10

RF24 radio(CE_PIN, CSN_PIN);
PL1167_nRF24 prf(radio);
MiLightRadio mlr(prf);

void setup()
{
  Serial.begin(115200);
  printf_begin();
  delay(1000);
  Serial.println("OpenMiLight receiver starting");
  mlr.begin();
}


void loop()
{
  static int dupesPrinted = 0;
  if (mlr.available()) {
    printf("\n");
    uint8_t packet[7];
    size_t packet_length = sizeof(packet);
    mlr.read(packet, packet_length);

    for (int i = 0; i < packet_length; i++) {
      printf("%02X ", packet[i]);
    }
    
    printf("\n");
    
    printf("ID %02X%02X;", packet[1], packet[2]);
    
    if(packet[4] & 0b00000111) 
      printf("Group %d;", packet[4] & 0b00000111);
    else
      printf("All;");
    
    byte button = packet[5] & 0x0F;
    
    if(button == 0x0F) 
      printf("Color %02X;", packet[3]);
    else if(button == 0x0E) {
      byte brightnes = (packet[4] & 0b11111000) >> 3;
      
      if(brightnes <= 18) {
        brightnes -= 16;
        brightnes = brightnes * -1;
      }
      else{
        brightnes -= 47;
        brightnes = brightnes * -1;
      }
      
      if(brightnes < 0)
        brightnes = 0;
      else if(brightnes > 25)
        brightnes = 25;
      
      printf("Brightnes %d;", brightnes);
    }
    else if(button == 0x0D) 
      printf("Disco %d;", packet[0] & 0x0F);
    else if(button == 0x0C) 
      printf("Speed -;");
    else if(button == 0x0B) 
      printf("Speed +;");
    else if(button == 0x0A) 
      printf("Group 4 off;");
    else if(button == 0x09) 
      printf("Group 4 on;");
    else if(button == 0x08) 
      printf("Group 3 off;");
    else if(button == 0x07) 
      printf("Group 3 on;");
    else if(button == 0x06) 
      printf("Group 2 off;");
    else if(button == 0x05) 
      printf("Group 2 on;");
    else if(button == 0x04)
      printf("Group 1 off;");
    else if(button == 0x03)
      printf("Group 1 on;");
    else if(button == 0x02)
      printf("All off;");
    else if(button == 0x01)
      printf("All on;");
    else if(button == 0x00) 
      printf("Slider released;");
      
    if(packet[5] & 0x10)
      printf("Long press;");
    else
      printf("Short press;");
  }
  
  int dupesReceived = mlr.dupesReceived();
  for(;dupesPrinted < dupesReceived; dupesPrinted++) {
    printf(".");
  }
}

