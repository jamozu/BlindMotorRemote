# Introduction
This project consists of a centralized remote-control (the Device) to manage and control roller blinds and shades compatible with 433 band RF remotes like the BF-305.

The goal is to have one Device to register and control all the blinds in an area or room of the house and control each motor through Home Assistant or another MQTT compatible hub using Cover entities.

The main features for the Device are:
* Allows to register up to 50 motors/blinds.
*	Allows to register up to 20 remote-control codes.
*	Control blinds via Home Assistant using MQTT using Cover entities.
*	Support for partially open/close positions.
*	Each motor can be added to scenes and automations.
*	The communication between Home Assistant and the remote control is via WiFi, so there is flexibility in the location of the Devices.
*	Easy to setup.

It is possible to do this same functionality with a universal RF remote control like Broadlink, but I had little success in some tests that I did with this type of devices and with 433Mhz RF signals and in the end it was more expensive and complicated to configure so I decided to try the DIY route.

For mor details check the documentation at: https://github.com/jamozu/ShadeRemote/blob/main/BlindMotor_Remote_V1.pdf
