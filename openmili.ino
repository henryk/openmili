#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <printf.h>

#include "PL1167_nRF24.h"
#include "MiLightRadio.h"

#define CE_PIN 8
#define CSN_PIN 53

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
  }
  
  int dupesReceived = mlr.dupesReceived();
  for(;dupesPrinted < dupesReceived; dupesPrinted++) {
    printf(".");
  }
}

