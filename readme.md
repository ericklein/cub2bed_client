# cub2bed_client
## What is cub2bed_client? 
Simple two-way message appliance

### Purpose
Client message appliance alerts server appliance of message and receives status updates from server

### Contributors

### Software Dependencies
  - uses Radiohead http://www.airspayce.com/mikem/arduino/RadioHead/RadioHead-1.100.zip
    - adfruit maintains a GitHub branch at https://github.com/adafruit/RadioHead

### BOM
  - 1X each: Adafruit Arduino Pro Mini 328 - 5V (part 2378) + Adafruit FRM69HCW Radio Breakout (part 3070) + Adafruit FTDI Friend (part 284)
  - 1X: Adafruit Feather M0 RFM69HCW Packet Radio (part 3176)
  - simple button
  - 1 protoboard
  - wire

### Pinouts
  - see Fritzing diagram


### Information Sources
    - Adafruit RFM69HCW (SPI): https://learn.adafruit.com/adafruit-rfm69hcw-and-rfm96-rfm95-rfm98-lora-packet-padio-breakouts?view=all
    - https://learn.adafruit.com/adafruit-feather-m0-radio-with-rfm69-packet-radio
    - Radiohead main page: https://www.airspayce.com/mikem/arduino/RadioHead/index.html
    - https://community.particle.io/t/my-attempt-at-rf95-lora/40573/3
  - https://groups.google.com/forum/#!forum/radiohead-arduino
  - https://learn.adafruit.com/adafruit-rfm69hcw-and-rfm96-rfm95-rfm98-lora-packet-padio-breakouts?view=all

### Issues

### Questions
      - surprised iota isn't having a problem with arg3
      - do I buffer overrun or truncate when inbound packet is larger than defined buffer len?
      - Why are we constantly resetting buf, len in loop?
      - how do you determine server and client without hardcoding pairs?
      - code looks like it could send packets to multiple clients based on form of client_address declare?

### Learnings

### Feature Requests
 
### Revisions
    - 050320: first version, adapted from adapted from datagram_client