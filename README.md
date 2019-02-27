# AIS_NB_NE866

The Arduino Library is designed for `DEVIO NB-XBEE I`to connect with your own IoT Platform/ Server. 

This version is recommended for Mega, MKR boards, Zero, Due, 101, etc.

Note: ARDUINO MEGA: RX = 18, TX = 19

## Function List:

### General Function:
	debug(<Boolean>)
		○ True : Print Log on Serial Monitor
		○ False: No Log on Serial Monitor
	setupDevice(<serverPort>) 
	getDeviceIP()
		○ Return: <String IP>
	str2HexStr(< String data>)
		○ Return: <String dataHexString>
	pingIP(serverIP)
		○ Return type: pingRESP
		○ Value: addr, ttl, rtt

### Send Data:
	sendUDPmsgStr(<serverIP>,< serverPort>,<udpData>)
		○ Maximum udpData : 512 Bytes
	sendUDPmsg (<serverIP>,< serverPort>,<udpDataHEX>)
		○ Maximum udpDataHEX : 512 Bytes

### Receive Data: 
	waitResponse()
		○ Return type: UDPReceive
		○ Value: ip_address, port, length, data, remaining_length

## Device Compatibility:
### DEVIO NB-XBEE I
`DEVIO NB-XBEE I` is a communication board for XBEE form factor compatible pinout. 

For `DEVIO NB-XBEE I` Pinout, please go to "www.ais.co.th/aiap" in the "IoT Dev Center" menu.

### Specifications:
  Communication Module: Telit NE866B1-E1
  Network Technology: LTE Cat. NB1 (NB-IoT)
  
  Frequency: B8 (900 MHz) /B20 (800MHz)
  
  Data Transmission:
  - Uplink 20 kbps (Single-tone)
  - Downlink 250 kbps 
  
  Protocol Stack:
  - UDP/IP
  
  Serial Communication (UART)
  - Control via AT commands according to 3GPP TS 27.005, 27.007 and customized AT commands.
  
  Dimensions: 
  - Width x Length x Height: 25x35x2 mm (Excluded antenna)
  
  NB-IoT Module Certification
  - RED (Europe), CCC (China), GCF.



# Getting Started
  1. Connect `DEVIO NB-XBEE I` to Arduino Board
  2. Coding & Compile using Arduino Desktop IDE
		- Install the Arduino board drivers
		- Import AIS NB-IoT Library 
		- Open AIS NB-IoT Example and specify your server IP & Port
		- Compile & Upload
