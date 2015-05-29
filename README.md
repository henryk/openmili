# openmili
Open implementation of the Mi-Light 2.4GHz LED light control protocol

This project should eventually contain
* Library for a virtual PL1167 based on a Nordic back-end, or real PL1167 connected over SPI
* Nordic back-ends for nRF51[48]22 running natively on nRF51-DONGLE (pca10031), and nRF24L01(+) connected over SPI to Arduino
* Library for Mi-Light protocol messages (sending and receiving)
* Dumb sender/receiver that simply exposes Mi-Light messages over serial (both nRF51/pca10031 and nRF24/Arduino)
* Host software that is a drop-in replacement for a Mi-Light Wi-Fi gateway, using the dumb sender, but with configurable sender IDs
* Maybe: Smarter firmware that can speak the protocol on its own, possibly with improved timing
