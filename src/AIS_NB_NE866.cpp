/*
AIS_NB_NE866 v1.0
Author: DEVI/AIS
Create Date: 20 February 2019
Modified: 20 February 2019

This version is recommended for Mega, MKR boards, Zero, Due, 101, etc.
ARDUINO MEGA: RX = 18, TX = 19
*/

#include "AIS_NB_NE866.h"

//################### Buffer #######################
String input;
String buffer;
//################### counter value ################
byte k=0;
//################## flag ##########################
bool end=false;
bool send_NSOMI=false;
bool flag_rcv=true;
//################### Parameter ####################
bool en_rcv=false;
unsigned long previous=0;
unsigned char sendMode = 0;
String sendStr;

void event_null(char *data){}

AIS_NB_NE866::AIS_NB_NE866(){
	Event_debug =  event_null;
}

void AIS_NB_NE866:: setupDevice(String serverPort, String addressI){
	
	Serial.println(F("############ AIS_NB_NE866 Library ############"));

	Serial1.begin(9600);
    _Serial = &Serial1;   
	
	reset();
	setEchoOff();

	_Serial->println(F("AT+CMEE=2"));
	AIS_NB_NE866_RES res = wait_rx_bc(1000,F("OK"));

	String imei = getIMEI();
	if (debug) Serial.print(F("# Module IMEI-->  "));
	if (debug) Serial.println(imei);

	delay(2000);

	String fmver = getFirmwareVersion();
	if (debug) Serial.print(F("# Firmware ver-->  "));
	if (debug) Serial.println(fmver);

	String imsi = getIMSI();
	if (debug) Serial.print(F("# IMSI SIM-->  "));
	if (debug) Serial.println(imsi);
	
	attachNB(serverPort,addressI);
}

void AIS_NB_NE866:: setEchoOff(){
	_Serial->println(F("ATE0"));
	AIS_NB_NE866_RES res = wait_rx_bc(500,F("OK"));
}

void AIS_NB_NE866:: reset(){
	rebootModule();
	while (!setPhoneFunction(1)){
		Serial.print(F("."));
	}
	Serial.println();
}

void AIS_NB_NE866:: rebootModule(){
	_Serial->println(F("AT"));
	AIS_NB_NE866_RES res = wait_rx_bc(1000,F("OK"));
	if (debug) Serial.print(F("# Reboot Module"));
	_Serial->println(F("AT#REBOOT"));
	while (!waitReady()){}
	if (debug) Serial.println();
    _Serial->flush();
	delay(6000);
}

bool AIS_NB_NE866:: waitReady(){
	static bool reset_state=false;
	if(_Serial->available()){
		String input = _Serial->readStringUntil('\n');
		if(input.indexOf(F("OK"))!=-1){
			k++;
			if(k>1){ 
				if(debug) Serial.println(F("...OK"));
				k=0;
				return(true);
			}
		}
	}
	return(false);
}

bool AIS_NB_NE866:: setPhoneFunction(unsigned char mode){
	_Serial->print(F("AT+CFUN="));
	_Serial->println(mode);
	AIS_NB_NE866_RES res = wait_rx_bc(1000,F("OK"));
	delay(2000);
	return(res.status);
}

String AIS_NB_NE866:: getIMEI(){
	String out="";
	int i=0;
	_Serial->println(F("AT+CGSN"));
	while(1){
		if(_Serial->available()){
			out += _Serial->readStringUntil('\n');
			if(out.indexOf(F("OK"))!=-1) {
				break;
			}
		}
	}
	out.replace(F("OK"),"");
	return (out);
}

String AIS_NB_NE866:: getFirmwareVersion(){
	_Serial->println(F("AT+CGMR"));
	AIS_NB_NE866_RES res = wait_rx_bc(1000,F("OK"));
	String out = res.temp;
    out.replace(F("OK"),"");
	out = out.substring(0,out.length());
	res = wait_rx_bc(500,F("OK"));
	return (out);
}
String AIS_NB_NE866:: getIMSI(){
	_Serial->println(F("AT+CIMI"));
	AIS_NB_NE866_RES res = wait_rx_bc(1000,F("OK"));
	String out = res.temp;
    out.replace(F("OK"),"");
	out = out.substring(0,out.length());
	res = wait_rx_bc(500,F("OK"));
	return (out);
}

pingRESP AIS_NB_NE866:: pingIP(String IP){
	pingRESP pingr;
	String data = "";
	_Serial->print(F("AT#PING="));
	_Serial->println(IP);
	bool status = false;
	while(1){
		if(_Serial->available()){
			input = _Serial->readStringUntil('\n');
			if(input.indexOf(F("#PING: 04"))!=-1) {
				status = true;
				break;
			}
			else if(input.indexOf(F("Incorrect"))!=-1){ 
				break;
			}
		}
	}

	if(status){
		data = input;
		int index = data.indexOf(F(","));
		int index2 = data.indexOf(F(","),index+1);
		int index3 = data.indexOf(F(","),index2+1);
		pingr.status = true;
		pingr.addr = data.substring(index+1,index2);
		pingr.ttl = data.substring(index2+1,index3);
		pingr.rtt = data.substring(index3+1,data.length());
		//Serial.println("# Ping Success");
		if (debug) Serial.println("# Ping IP:"+pingr.addr + ",ttl= " + pingr.ttl + ",rtt= " + pingr.rtt);

	}else { 
		if (debug) Serial.println(F("# Ping Failed"));
	}
	
	while(1){
		if(_Serial->available()){
			input = _Serial->readStringUntil('\n');
			if(input.indexOf(F("OK"))!=-1) {
				break;
			}
		}
	}

	return pingr;
}

String AIS_NB_NE866:: getDeviceIP(){
	String data = "";
	_Serial->println(F("AT+CGPADDR=0"));
	AIS_NB_NE866_RES res = wait_rx_bc(3000,F("+CGPADDR"));
	if(res.status){
		data = res.data;
		int index = data.indexOf(F(":"));
		int index2 = data.indexOf(F(","));
		data = res.data.substring(index2+1,data.length());
		if (debug) Serial.print(F("# Device IP: "));
		if (debug) Serial.println(data);

	}else {data = "";}
	res = wait_rx_bc(500,F("OK"));
	return data;
}

bool AIS_NB_NE866:: setAutoConnectOn(){
	_Serial->println(F("AT+NCONFIG=AUTOCONNECT,TRUE"));
	AIS_NB_NE866_RES res = wait_rx_bc(1000,F("OK"));
	return(res.status);
}

bool AIS_NB_NE866:: setAutoConnectOff(){
	_Serial->println(F("AT+NCONFIG=AUTOCONNECT,FALSE"));
	AIS_NB_NE866_RES res = wait_rx_bc(1000,F("OK"));
	return(res.status);
}

String AIS_NB_NE866:: getNetworkStatus(){
	String out = "";
	String data = "";

	_Serial->println(F("AT+CEREG=2"));
	AIS_NB_NE866_RES res = wait_rx_bc(500,F("OK"));
	_Serial->println(F("AT+CEREG?"));
	 res = wait_rx_bc(2000,F("+CEREG"));
     if(res.status){
		data = res.data;
		int index = data.indexOf(F(":"));
		int index2 = data.indexOf(F(","));
		int index3 = data.indexOf(F(","),index2+1);
		out = data.substring(index2+1,index2+2);
		if (out == F("1")){
			out = F("Registered");
		}else if (out == F("0")){
			out = F("Not Registered");
		}else if (out == F("2")){
			out = F("Trying");
		}
	if (debug) Serial.print(F("# Get Network Status : "));
	if (debug) Serial.println(out);

	}
	res = wait_rx_bc(1000,F("OK"));
	_Serial->flush();
	return(out);
}
/*
bool AIS_NB_NE866:: setAPN(String apn){
	String cmd = "AT+CGDCONT=1,\"IP\",";
	cmd += "\""+apn+"\"";
	_Serial->println(cmd);
	AIS_NB_NE866_RES res = wait_rx_bc(1000,F("OK"));
	return(res.status);
}*/

String AIS_NB_NE866:: getAPN(){
	String data="";
	String out="";
	_Serial->println(F("AT+CGDCONT?"));
	AIS_NB_NE866_RES res = wait_rx_bc(2000,F("+CGDCONT"));
	if(res.status){
		int index=0;
		int index2=0;
		data = res.data;
		index = data.indexOf(F(":"));
		index2 = data.indexOf(F(","));

		index = res.data.indexOf(F(","),index2+1);
		index2 = res.data.indexOf(F(","),index+1);
		out = data.substring(index+2,index2-1);
		Serial.print(F("# Get APN: "));
		Serial.println(out);
	}
	res = wait_rx_bc(500,F("OK"));
	return(out);
}

bool AIS_NB_NE866:: attachNB(String serverPort, String addressI) {
	bool ret=false;
	signal sig;

	while(1){
		sig = getSignal(0);
		if(sig.csq.indexOf(F("99"))==-1) break;
		delay(200);
	}

	if(!getNBConnect()){
		if (debug) Serial.print(F("# Connecting NB-IoT Network"));
		for(int i=1;i<60;i+=1){
			setPhoneFunction(1);
			setAutoConnectOn();
			sgact(1);
			delay(3000);
			if(getNBConnect()){ 				  
				ret=true;
				break;
			}
		Serial.print(F("."));
		}
	} else{
			return true;
		}
	if (debug) Serial.println();

	if (ret){
		if (debug) Serial.println(F("> Connected"));
	}
	else {
			if (debug) Serial.print(F("> Disconnected"));
		 }
	if (debug) Serial.println(F("\n========="));
	return ret;
}
bool AIS_NB_NE866:: detachNB(){
	bool ret=false;
	_Serial->flush();
	if (debug) Serial.print(F("# Disconnecting NB-IoT Network"));
	sgact(0);
	delay(1000);
	for(int i=1;i<60;i+=1){
		if (debug) Serial.print(F("."));
		if(!getNBConnect()){ 
			ret=true; 
			break;
		}

	}
	if (debug) Serial.println(F("> Disconnected"));
	return ret;
}

bool AIS_NB_NE866:: sgact(unsigned char mode){
	_Serial->print(F("AT#SGACT=0,")); 
	_Serial->println(mode);
	AIS_NB_NE866_RES res = wait_rx_bc(5000,F("OK"));
	return(res.status);
}

bool AIS_NB_NE866:: getNBConnect(){
	_Serial->println(F("AT#SGACT?"));
	AIS_NB_NE866_RES res = wait_rx_bc(500,F("#SGACT:"));
	bool ret;
	if(res.status){
        if(res.data.indexOf(F("#SGACT: 0,1"))!=-1){
				ret = true;
			}
		if(res.data.indexOf(F("#SGACT: 0,0"))!=-1){
				ret = false;
			}
	}
	res = wait_rx_bc(500,F("OK"));
	delay(10);
	return(ret);
}

signal AIS_NB_NE866:: getSignal(){
	signal sig = getSignal(1);
	return(sig);
}

signal AIS_NB_NE866:: getSignal(int state){
	_Serial->println(F("AT+CSQ"));
	AIS_NB_NE866_RES res = wait_rx_bc(500,F("+CSQ"));
	signal sig;
	int x = 0;
	if(res.status){
		if(res.data.indexOf(F("+CSQ"))!=-1){
			int index = res.data.indexOf(F(":"));
			int index2 = res.data.indexOf(F(","));
			sig.csq = res.data.substring(index+1,index2);
			x = sig.csq.toInt();
			x = (2*x)-113;
			sig.rssi = String(x);
			sig.ber  = res.data.substring(index2+1);
			if (debug && state==1) Serial.println("# Get CSQ Signal: csq= " + sig.csq + ", rssi= " + sig.rssi + ", ber= " +sig.ber);
		}
	}
	res = wait_rx_bc(500,F("OK"));
	return(sig);
}

void AIS_NB_NE866:: createUDPSocket(String port, String addressI) {
	if(sendMode == MODE_STRING_HEX){                            
		_Serial->println(F("AT#SCFGEXT=1,3,1,0,0,1"));
	}
	else{
		_Serial->println(F("AT#SCFGEXT=1,3,0,0,0,0"));
	}

	AIS_NB_NE866_RES res = wait_rx_bc(3000,F("OK"));
	_Serial->print(F("AT#SD=1,1,"));
	_Serial->print(port);
	_Serial->print(F(",\""));
	_Serial->print(addressI);
	_Serial->println(F("\",0,5100,1"));

	res = wait_rx_bc(3000,F("OK"));
	delay(300);
	if (debug && res.status) Serial.println(F("# Create socket success"));
}

UDPSend AIS_NB_NE866:: sendUDPmsg( String addressI,String port,unsigned int len,char *data,unsigned char send_mode){
	sendMode = send_mode;

	createUDPSocket(port, addressI);

	UDPSend ret;
	if (debug) Serial.println(F("\n========="));
	if (debug) Serial.print(F("# Sending Data IP="));
	if (debug) Serial.println(addressI);
	if (debug) Serial.print(F("# PORT="));
	if (debug) Serial.print(port);
	if (debug) Serial.println();

	_Serial->print(F("AT#SSENDEXT=1,"));

	if(send_mode == MODE_STRING_HEX){
		_Serial->print(String(len/2));
		_Serial->write(13);
	}
	else{
		_Serial->print(String(len));
		_Serial->write(13);
	}

	AIS_NB_NE866_RES res = wait_rx_bc(200,F(">"));;

	if (debug) Serial.print(F("# Data="));
		if(send_mode == MODE_STRING_HEX){
			for(int i=0;i<len;i++){
				_Serial->print(data[i]);
				delay(10);

				if (debug) Serial.print(data[i]);
			}
		}
		if(send_mode == MODE_STRING){
			if (debug) Serial.print(data);
			_Serial->print(data);
		}

	Serial.println();
	_Serial->write(13);
	
	res = wait_rx_bc(6000,F("OK"));	

	if(res.status){
		if (debug) Serial.println(F("# Send OK"));
	}else {
		if (debug) Serial.println(F("# Send ERROR"));		
	}

	_Serial->flush();
	return(ret);
}

UDPReceive AIS_NB_NE866:: waitResponse(){
  	UDPReceive rx_ret;

  if(_Serial->available()){
    char data=(char)_Serial->read();
    if(data=='\n' || data=='\r'){
      if(k>2){
        end=true;
        k=0;
      }
      k++;
    }
    else{
      input+=data;
    }
  }
  if(end){
    end=false;

    int index1 = input.indexOf(F(","));
    if(index1!=-1){
    	int index0 = input.indexOf(F("STRING: "));
        int index2 = input.indexOf(F(","),index1+1);
        int index3 = input.indexOf(F(","),index2+1);
        int index4 = input.indexOf(F(","),index3+1);
        int index5 = input.indexOf(F(","),index4+1);
        int index6 = input.indexOf(F("\r"));        

        rx_ret.socket = input.substring(index3+1,index5).toInt();
        rx_ret.ip_address = input.substring(index0+7,index1);
        rx_ret.port = input.substring(index1+1,index2).toInt();
        rx_ret.length = input.substring(index3+1,index4).toInt();
        rx_ret.data = input.substring(index5+1,index6);

        if (debug) receive_UDP(rx_ret);

    }

    send_NSOMI=false;
    input=F("");
          
  }
    return rx_ret;
}

UDPSend AIS_NB_NE866:: sendUDPmsg(String addressI,String port,String data){
	int x_len = data.length();

	char buf[x_len+2];
	data.toCharArray(buf,x_len+1);
	return(sendUDPmsg(addressI,port,x_len,buf,MODE_STRING_HEX));
}
UDPSend AIS_NB_NE866:: sendUDPmsgStr(String addressI,String port,String data){
	sendStr = data;
	int x_len = data.length();
	char buf[x_len+2];
	data.toCharArray(buf,x_len+1);
	return(sendUDPmsg(addressI,port,x_len,buf,MODE_STRING));
}

bool AIS_NB_NE866:: closeUDPSocket(){
	_Serial->println(F("AT#SH=1"));
	AIS_NB_NE866_RES res = wait_rx_bc(1000,F("OK"));
	return(res.status);
}

AIS_NB_NE866_RES AIS_NB_NE866:: wait_rx_bc(long tout,String str_wait){
	unsigned long pv_ok = millis();
	unsigned long current_ok = millis();
	String input;
	unsigned char flag_out=1;
	unsigned char res=-1;
	AIS_NB_NE866_RES res_;
	res_.temp="";
	res_.data = "";

	while(flag_out){
		if(_Serial->available()){
			input = _Serial->readStringUntil('\n');
			res_.temp+=input;
			if(input.indexOf(str_wait)!=-1){
				res=1;
				flag_out=0;
			}
		    else if(input.indexOf(F("ERROR"))!=-1){
				res=0;
				flag_out=0;
			}
		}
		current_ok = millis();
		if (current_ok - pv_ok>= tout){
			flag_out=0;
			res=0;
			pv_ok = current_ok;
		}
	}

	res_.status = res;
	res_.data = input;
	return(res_);
}

void AIS_NB_NE866::printHEX(char *str){
  char *hstr;
  hstr=str;
  char out[3]="";
  int i=0;
  bool flag=false;
  while(*hstr){
    flag=itoa((int)*hstr,out,16);
    
    if(flag){
      _Serial->print(out);

      if(debug){
        Serial.print(out);
      }
      
    }
    hstr++;
  }
}

String AIS_NB_NE866:: str2HexStr(String strin){
  int lenuse = strin.length();
  char charBuf[lenuse*2-1];
  char strBuf[lenuse*2-1];
  String strout = "";
  strin.toCharArray(charBuf,lenuse*2) ;
  for (int i = 0; i < lenuse; i++){
    sprintf(strBuf, "%02X", charBuf[i]);

    if (String(strBuf) != F("00") ){
           strout += strBuf;
    }
  }

  return strout;
}

char AIS_NB_NE866:: char_to_byte(char c){
	if((c>='0')&&(c<='9')){
		return(c-0x30);
	}
	if((c>='a')&&(c<='f')){
		return (c-'a'+10);
	}
}

void AIS_NB_NE866:: receive_UDP(UDPReceive rx){
  String dataStr;
  Serial.println(F("======"));
  Serial.println(F("# Incoming Data"));
  Serial.println("# IP--> " + rx.ip_address);
  Serial.println("# Port--> " + String(rx.port));
  Serial.println("# Length--> " + String(rx.length));
  Serial.println("# Data--> " + rx.data);
  Serial.println(F("======"));
}
