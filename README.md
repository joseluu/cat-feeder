# Cat feeder firmware

This is an alternate firmware for the cat feeder https://smartsolutions4home.com/ss4h-pf-pet-feeder/

Work in progress, feeding my cat reliably since july 2021

<img src="https://raw.githubusercontent.com/joseluu/cat-feeder/master/Documents/Photos/Cat_feeder.jpg" width="300">

Uses an ESP32 Dev Module by Lilygo programmed using the arduino IDE

<img src="https://raw.githubusercontent.com/joseluu/cat-feeder/master/Documents/Photos/feeder_esp32_details.jpg" width="200">

<img src="https://raw.githubusercontent.com/joseluu/cat-feeder/master/Documents/Photos/feeder_electronics_overview.jpg" width="200">

Connects to a stepper motor as specified in the original project using the control board supplied with the stepper

<img src="https://raw.githubusercontent.com/joseluu/cat-feeder/master/Documents/Photos/feeder_stepper_board.jpg" width="200">


The MCU control board also monitors 3 infrared detectors for tracking the cat or the environment, someday these will feed the house automation MQTT

The MCU publishes a web interface on the local network using mDNS at address: http://feed-cat.local/ (works on chrome)

The web interface allows to manually operate the motor (hence feed the cat)
The web interface also logs the IR detectors activity

To avoid a stuck feed screw, the firmware does a reverse rotation first, then a forward rotation, this has proven reliable. When operating manually you need to explicitly reverse first.

As programmed, it delivers 20g of dry cat food 4 times a day.