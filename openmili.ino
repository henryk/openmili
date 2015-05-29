#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <printf.h>
#define CE_PIN 7
#define CSN_PIN 8

const uint8_t pipe[] = {0xd1, 0x28, 0x5e, 0x55, 0x55};
#define CRC_POLY 0x8408

RF24 radio(CE_PIN, CSN_PIN);

uint8_t packet[12];

static void start_radio()
{
  radio.begin();
  radio.setAddressWidth(sizeof(pipe));
  radio.openReadingPipe(1,pipe);
  radio.setChannel(11);
  radio.setPayloadSize( sizeof(packet) );
  radio.setAutoAck(false);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_1MBPS);
  radio.disableCRC();
  radio.startListening();
}  

void setup()
{
  Serial.begin(115200);
  printf_begin();
  delay(1000);
  Serial.println("Nrf24L01 Receiver Starting");
  start_radio();
  radio.printDetails();
}

static uint16_t calc_crc(uint8_t *data, size_t data_length) {
	uint16_t state = 0;
	for (size_t i = 0; i < data_length; i++) {
		uint8_t byte = data[i];
		for (int j = 0; j < 8; j++) {
			if ((byte ^ state) & 0x01) {
				state = (state >> 1) ^ CRC_POLY;
			} else {
				state = state >> 1;
			}
			byte = byte >> 1;
		}
	}
	return state;
}


static void demangle_packet(uint8_t *in, uint8_t *out) {
  int i = 0;
  memset(out, 0, 10);

  in++; // packet[0] is 0x25 (part of original sync)
  while (i < 11) {
    if (i > 0)
      out[i - 1] |= ((in[0]) & 0xF) << 4;
    i++;
    if (i < 11)
      out[i - 1] |= (in[0] >> 4) & 0xF;
    in++;
  }
}

static void dump_milight_packet(uint8_t *packet) {
	static uint8_t prev_packet[10];
	uint8_t tmp[10] = { 0 };
	demangle_packet(packet, tmp);

	uint16_t crc = calc_crc(tmp, 8);
	if ((tmp[8] != (crc & 0xff)) || (tmp[9] != ((crc >> 8) & 0xff))) {
		return;
	}

	if (memcmp(tmp, prev_packet, 10) != 0) {
		printf("\n");
		for (int i = 0; i < 10; i++) {
			printf("%02X ", tmp[i]);
		}
		memcpy(prev_packet, tmp, 10);
	} else {
		printf(".");
	}
}



void loop()
{
  if ( radio.available() ) {
    radio.read( packet, sizeof(packet) );
    
    /* The nRF24 seems to receive in big endian: transpose bytes */
    for(int i=0; i<sizeof(packet); i++) {
      uint8_t b = packet[i];
      uint8_t m = 0;
      for(int j=0; j<8; j++) {
        m<<=1;
        m|=(b&1);
        b>>=1;
      }
      packet[i] = m;
    }
    dump_milight_packet(packet);
    
    // FIXME: HACK HACK HACK. For some reason, radio.available() isn't reset after radio.read(). Restart the entire nRF chip instead.
    radio.stopListening();
    start_radio();
  }
}

