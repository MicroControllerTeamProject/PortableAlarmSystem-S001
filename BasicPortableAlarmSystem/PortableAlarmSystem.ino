
#if (defined(__AVR__))
#include <avr/pgmspace.h>
#else
#include <pgmspace.h>
#endif
//#include <MemoryFree.h>
//#include <pgmStrToRAM.h>
#include "MyBlueTooth.h"
#include "BlueToothCommandsUtil.h"
#include "LSGEEpromRW.h" 
#include <EEPROM.h> 
#include "MySim900.h"
#include "ActivityManager.h"


char version[15] = "S001 7.86-RTM";

//Library version : 6.55-RTM

ActivityManager* _delayForTemperature = new ActivityManager(60);

ActivityManager* _delayForVoltage = new ActivityManager(60);

//ActivityManager* _delayForGetCoordinates= new ActivityManager(120);

//ActivityManager* _delayForFindPhone = new ActivityManager(30); 

//ActivityManager* _delayForSignalStrength = new ActivityManager(30);

MyBlueTooth* btSerial;

MySim900* mySim900;

String _oldPassword = "";

String _newPassword = "";

#pragma region pinsDefinition

const byte _pin_powerLed = 13;

const uint8_t _pin_pir = A5;

const uint8_t _pin_buzzer = 5;

const byte _pin_rxSIM900 = 7;

const byte _pin_txSIM900 = 8;

const byte _pin_reedRelay = A4;

#pragma endregion pinsDefinition

const byte _addressStartBufPhoneNumber = 1;

const byte _addressStartBufPrecisionNumber = 12;

const byte _addressStartBufTemperatureIsOn = 14;

const byte _addressStartBufTemperatureMax = 16;

const byte _addressStartBufPirSensorIsON = 19;

const byte _addressStartDeviceAddress = 21;

const byte _addressStartDeviceName = 33;

const byte _addressStartFindOutPhonesON = 48;

const byte _addressStartBTSleepIsON = 50;

const byte _addressDBPhoneIsON = 52;

const byte _addressStartBufPhoneNumberAlternative = 54;

const byte _addressStartFindMode = 65;

const byte _addressApn = 67;

const byte _addressOffSetTemperature = 92;

const byte _addressDelayFindMe = 94;

const byte _addressExternalInterruptIsOn = 96;

const byte _addressStartDeviceAddress2 = 98;

const byte _addressStartDeviceName2 = 110;

const byte _addressBuzzerIsOn = 122;

uint8_t _isPIRSensorActivated = 0;

bool _isBlueLedDisable = true;

bool _isDisableCall = false;

bool _isOnMotionDetect = false;

bool _isOnExternalMotionDetect = false;

bool _isPositionEnable = false;

unsigned long _sensitivityAlarm;

char _prefix[4] = "+39";

bool _isAlarmOn = false;

char _phoneNumber[11];

char _phoneNumberAlternative[11];

String _whatIsHappened = "";

uint8_t _isBTSleepON = 1;

uint8_t _isExternalInterruptOn = 0;

uint8_t _isBuzzerOn = 0;

uint8_t _phoneNumbers = 0;

uint8_t _findOutPhonesMode = 0;

uint8_t _tempMax = 0;

uint8_t _delayFindMe = 1;

unsigned int _offSetTempValue = 324.13;

String _signalStrength;

String _deviceAddress = "";

String _deviceAddress2 = "";

String _deviceName = "";

String _deviceName2 = "";

float _voltageValue = 0;

float _voltageMinValue = 0;

bool _isMasterMode = false;

bool _isExtenalInterruptNormalyClosed = true;

unsigned long _timeToTurnOnAlarm = millis() + 300000;

//String _apn = "";

bool _isDeviceDetected = false;

const int BUFSIZEPHONENUMBER = 11;

const int BUFSIZEPHONENUMBERALTERANATIVE = 11;

const int BUFSIZEPIRSENSORISON = 2;
char _bufPirSensorIsON[BUFSIZEPIRSENSORISON];

const int BUFSIZEFINDOUTPHONESON = 2;
char _bufFindOutPhonesON[BUFSIZEFINDOUTPHONESON];

const int BUFSIZEDBPHONEON = 2;
char _bufDbPhoneON[BUFSIZEDBPHONEON];

const int BUFSIZETEMPERATUREMAX = 3;
char _bufTemperatureMax[BUFSIZETEMPERATUREMAX];

const int BUFSIZEDEVICEADDRESS = 13;
char _bufDeviceAddress[BUFSIZEDEVICEADDRESS];
char _bufDeviceAddress2[BUFSIZEDEVICEADDRESS];


const int BUFSIZEDEVICENAME = 15;
char _bufDeviceName[BUFSIZEDEVICENAME];
char _bufDeviceName2[BUFSIZEDEVICENAME];

const int BUFSIZEAPN = 25;
char _bufApn[BUFSIZEAPN];

const int BUFSIZEOFFSETTEMPERATURE = 5;
char _bufOffSetTemperature[BUFSIZEOFFSETTEMPERATURE];

const int BUFSIZEDELAYFINDME = 2;
char _bufDelayFindMe[BUFSIZEDELAYFINDME];

const int BUFSIZEEXTERNALINTERRUPTISON = 2;
char _bufExternalInterruptIsON[BUFSIZEEXTERNALINTERRUPTISON];

const int BUFSIZEBUZZERISON = 2;
char _bufBuzzerIsON[BUFSIZEBUZZERISON];

void setup()
{
	mySim900 = new MySim900(_pin_rxSIM900, _pin_txSIM900, false);

	mySim900->Begin(19200);

	mySim900->IsCallDisabled(false);
	
	inizializePins();

	inizializeInterrupts();

	btSerial = new MyBlueTooth(&Serial, 10, 6, 38400, 9600);

	btSerial->Reset_To_Slave_Mode();

	//mySim900->getCCLK();

	_oldPassword = btSerial->GetPassword();

	//Serial.print("oldPassword : "); Serial.println(_oldPassword);

	btSerial->ReceveMode();

	initilizeEEPromData();

	if (_findOutPhonesMode != 0)
	{
		_isBTSleepON = 0;
		btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Find activated"), BlueToothCommandsUtil::Message));
		btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
	}

	btSerial->turnOnBlueTooth();

	_whatIsHappened = F("X");

	mySim900->ATCommand("AT+CNETLIGHT=0");

	delay(500);

	mySim900->ATCommand("AT+CPMS=\"SM\"");
	delay(500);
	/*if (mySim900->IsAvailable() > 0)
	{
		String s = mySim900->ReadIncomingChars2();
		Serial.println(s);
	}*/
	mySim900->ATCommand("AT+CMGF=1");
	delay(500);
	/*delay(5000);
	if (mySim900->IsAvailable() > 0)
	{
		String s = mySim900->ReadIncomingChars2();
		Serial.println(s);
	}*/
	mySim900->ATCommand("AT+CMGD=1,4");
	/*if (mySim900->IsAvailable() > 0)
	{
		String s = mySim900->ReadIncomingChars2();
		Serial.println(s);
	}*/

	delay(1000);

	mySim900->ATCommand("AT+CBAND=""EGSM_PCS_MODE""");

	delay(1000);

	if (mySim900->IsAvailable() > 0)
	{
		String s = mySim900->ReadIncomingChars2();
		//Serial.println(s);
	}

	pinMode(_pin_pir, INPUT_PULLUP);

	blinkLedHideMode();

	//Serial.println(btSerial->getVersion());
}

void initilizeEEPromData()
{
	LSG_EEpromRW* eepromRW = new LSG_EEpromRW();

	eepromRW->eeprom_read_string(_addressStartBufPhoneNumber, _phoneNumber, BUFSIZEPHONENUMBER);

	eepromRW->eeprom_read_string(_addressStartBufPhoneNumberAlternative, _phoneNumberAlternative, BUFSIZEPHONENUMBERALTERANATIVE);

	eepromRW->eeprom_read_string(_addressDBPhoneIsON, _bufDbPhoneON, BUFSIZEDBPHONEON);
	_phoneNumbers = atoi(&_bufDbPhoneON[0]);

	eepromRW->eeprom_read_string(_addressStartFindOutPhonesON, _bufFindOutPhonesON, BUFSIZEFINDOUTPHONESON);
	_findOutPhonesMode = atoi(&_bufFindOutPhonesON[0]);

	eepromRW->eeprom_read_string(_addressStartBufPirSensorIsON, _bufPirSensorIsON, BUFSIZEPIRSENSORISON);
	_isPIRSensorActivated = atoi(&_bufPirSensorIsON[0]);

	eepromRW->eeprom_read_string(_addressStartBufTemperatureMax, _bufTemperatureMax, BUFSIZETEMPERATUREMAX);
	_tempMax = atoi(_bufTemperatureMax);

	eepromRW->eeprom_read_string(_addressStartDeviceAddress, _bufDeviceAddress, BUFSIZEDEVICEADDRESS);
	_deviceAddress = F("005A,13,389DC0");// +String(_bufDeviceAddress);

	eepromRW->eeprom_read_string(_addressStartDeviceName, _bufDeviceName, BUFSIZEDEVICENAME);
	_deviceName = F("PhoneAccess001");//String(_bufDeviceName);

	eepromRW->eeprom_read_string(_addressStartDeviceAddress2, _bufDeviceAddress2, BUFSIZEDEVICEADDRESS);
	_deviceAddress2 = F("0019,09,037A0D");// +String(_bufDeviceAddress2);

	eepromRW->eeprom_read_string(_addressStartDeviceName2, _bufDeviceName2, BUFSIZEDEVICENAME);
	_deviceName2 = F("portablePcb");// String(_bufDeviceName2);

	/*eepromRW->eeprom_read_string(_addressApn, _bufApn, BUFSIZEAPN);
	_apn = String(_bufApn);
	_apn.trim();*/

	eepromRW->eeprom_read_string(_addressOffSetTemperature, _bufOffSetTemperature, BUFSIZEOFFSETTEMPERATURE);
	_offSetTempValue = atoi(_bufOffSetTemperature);

	eepromRW->eeprom_read_string(_addressDelayFindMe, _bufDelayFindMe, BUFSIZEDELAYFINDME);
	_delayFindMe = atoi(_bufDelayFindMe);

	eepromRW->eeprom_read_string(_addressExternalInterruptIsOn, _bufExternalInterruptIsON, BUFSIZEEXTERNALINTERRUPTISON);
	_isExternalInterruptOn = atoi(&_bufExternalInterruptIsON[0]);

	eepromRW->eeprom_read_string(_addressBuzzerIsOn, _bufBuzzerIsON, BUFSIZEBUZZERISON);
	_isBuzzerOn = atoi(&_bufBuzzerIsON[0]);

	delete(eepromRW);
}

void inizializePins()
{
	pinMode(_pin_powerLed, OUTPUT);
	pinMode(0, INPUT_PULLUP);
}

void inizializeInterrupts()
{
	attachInterrupt(0, motionTiltInternalInterrupt, RISING);
	attachInterrupt(1, motionTiltExternalInterrupt, CHANGE);
}

void callSim900()
{
	//Serial.println("Faccio chiamata");

	if (_isDisableCall) { return; }

	char phoneNumber[14];

	strcpy(phoneNumber, _prefix);

	if (_phoneNumbers == 1)
	{
		strcat(phoneNumber, _phoneNumber);
	}

	if (_phoneNumbers == 2)
	{
		strcat(phoneNumber, _phoneNumberAlternative);
	}

	mySim900->DialVoiceCall(phoneNumber);

	delay(1000);

	//Inserita per scaricare buffer dopo chiamata
	//dove si puo aggiungere codice per recupero risultato.
	//E agevola la pulizia per la ricezione sms.
	mySim900->ReadIncomingChars2();

}

void motionTiltExternalInterrupt() {
	if (_isExternalInterruptOn /*&& !_isPIRSensorActivated*/) {
		_isOnExternalMotionDetect = true;
	}
}

void motionTiltInternalInterrupt()
{
	if (!_isPIRSensorActivated) {
		_isOnMotionDetect = true;
	}
}

void getSignalStrength()
{
	_signalStrength = mySim900->GetSignalStrength();
}

void turnOffBluetoohIfTimeIsOver()
{
	if (_findOutPhonesMode == 0
		&& (millis() > _timeToTurnOnAlarm)
		&& btSerial->isBlueToothOn()
		&& _isBTSleepON
		)
	{
		btSerial->turnOffBlueTooth();
	}
}

//void turnOnBlueToothIfMotionIsDetected()
//{
//	if (_isOnMotionDetect
//		&& !_isAlarmOn
//		&& btSerial->isBlueToothOff()
//		&& _isBTSleepON
//		)
//	{
//		_isOnMotionDetect = false;
//		turnOnBlueToothAndSetTurnOffTimer(false);
//	}
//}

void findOutPhonesONAndSetBluetoothInMasterModeActivity()
{
	if (_isDisableCall) { return; }
	//todo:da mettere in altro luogo.
	_deviceAddress.trim();
	_deviceName.trim();
	_deviceAddress2.trim();
	_deviceName2.trim();

	/*if ((_findOutPhonesMode == 1 || _findOutPhonesMode == 2) && _isAlarmOn)
	{*/
	/*	if (_findOutPhonesMode == 1 && !_isAlarmOn)
		{
			_isAlarmOn = true;
		}*/

	if (_isMasterMode == false)
	{
		btSerial->Reset_To_Master_Mode();
		_isMasterMode = true;
	}

	for (uint8_t i = 0; i < _delayFindMe; i++)
	{
		if (_phoneNumbers == 1) {
			_isDeviceDetected = btSerial->IsDeviceDetected(_deviceAddress, _deviceName);
			if (_isDeviceDetected) {
				break;
				//Serial.println("Find first BT");
			}
		}
	/*	if (_findOutPhonesMode == 1)
		{*/
			
			if (_phoneNumbers == 2) {
				_isDeviceDetected = btSerial->IsDeviceDetected(_deviceAddress2, _deviceName2);
				if (_isDeviceDetected) { 
					//Serial.println("Find second BT");
					break; 
				};
			}
		//}
	}
	if (_isDeviceDetected)
	{
		blinkLedHideMode();
		//reedRelaySensorActivity(_pin_reedRelay);
	}
	else
	{
		if (_findOutPhonesMode == 2)
		{
			callSim900();
			_isMasterMode = false;
		}
	}
	//return _isDeviceDetected;
	//}
}

void loop()
{
	readIncomingSMS();

	if ((millis() > _timeToTurnOnAlarm) && _isAlarmOn != true)
	{
		_isAlarmOn = true; 
	}

	//if (!(_isOnMotionDetect && _isAlarmOn))
	//{
	//	readIncomingSMS();
	//}

	//if (_delayForSignalStrength->IsDelayTimeFinished(true))
	//{
	//	getSignalStrength();
	//}

	if (_isAlarmOn && (_findOutPhonesMode == 1 || _findOutPhonesMode == 2))
	{
		findOutPhonesONAndSetBluetoothInMasterModeActivity();
	}
	//if (_delayForCallNumbers->IsDelayTimeFinished(true))
	//{
	//	_callNumbers = 0;
	//}
	if (!(_isOnMotionDetect && _isAlarmOn))
	{
		turnOffBluetoohIfTimeIsOver();
	}
	/*if (!(_isOnMotionDetect && _isAlarmOn))
	{
		turnOnBlueToothIfMotionIsDetected();
	}*/

	if (!(_isOnMotionDetect && _isAlarmOn))
	{
		internalTemperatureActivity();
	}

	if (!(_isOnMotionDetect && _isAlarmOn))
	{
		voltageActivity();
	}

	//if (_isPositionEnable)
	//{
	//	if (_delayForGetCoordinates->IsDelayTimeFinished(true))
	//	{
	//		//getCoordinates();
	//	}
	//}

	if (!(_isOnMotionDetect && _isAlarmOn))
	{
		pirSensorActivity();
	}

	motionDetectActivity();

	if (!(_isOnMotionDetect && _isAlarmOn))
	{
		blueToothConfigurationSystem();
	}
}

void motionDetectActivity()
{
	if (_isDisableCall || _findOutPhonesMode == 2 || _isPIRSensorActivated) {
		_isOnMotionDetect = false;
		return;
	}
	//if ((millis() - _millsStart) > _sensitivityAlarm)
	//{
	//	_millsStart = 0;
	//	_isFirstTilt = true;
	//}
	
	//if ((_isOnMotionDetect && _isAlarmOn) || (_isAlarmOn && _isExternalInterruptOn && (_isExtenalInterruptNormalyClosed ^ digitalRead(3))))
	//if ((_isOnMotionDetect && _isAlarmOn) || (_isAlarmOn && (_isExtenalInterruptNormalyClosed ^ digitalRead(3))) || _isExternalInterruptOn))	/*if(true)*/
	
	if (_isAlarmOn && ((_isOnMotionDetect && !_isExternalInterruptOn) 
		|| _isOnExternalMotionDetect 
		|| ((_isExtenalInterruptNormalyClosed ^ digitalRead(3)) 
		&& _isExternalInterruptOn)))
	{
		blinkLedHideMode();

		detachInterrupt(0);

		detachInterrupt(1);

		_whatIsHappened = F("M");

		if (_findOutPhonesMode == 1)
		{
			if (!_isDeviceDetected)
			{
				callSim900();
				_isMasterMode = false;
			}
		}
		else
		{
			callSim900();
			_isMasterMode = false;
		}
		////Accendo bluetooth con ritardo annesso solo se è scattato allarme,troppo critico
		////per perdere tempo se non scattato allarme.
		//if (btSerial->isBlueToothOff() && _findOutPhonesMode == 0)
		//{
		//	delay(30000);
		//	turnOnBlueToothAndSetTurnOffTimer(false);
		//}
		////}

		//readIncomingSMS();

		/*	readIncomingSMS();

			findOutPhonesONAndSetBluetoothInMasterModeActivity();*/

		EIFR |= 1 << INTF1; //clear external interrupt 1
		EIFR |= 1 << INTF0; //clear external interrupt 0
		//EIFR = 0x01;
		sei();

		attachInterrupt(0, motionTiltInternalInterrupt, RISING);
		attachInterrupt(1, motionTiltExternalInterrupt, CHANGE);

		_isOnMotionDetect = false;
		_isOnExternalMotionDetect = false;
	}
}

//void restartBlueTooth()
//{
//	Serial.readString();
//	btSerial->ReceveMode();
//}

void turnOnBlueToothAndSetTurnOffTimer()
{
	Serial.flush();
	btSerial->Reset_To_Slave_Mode();
	//if (_findOutPhonesMode == 0 || isFromSMS)
	//{
	btSerial->ReceveMode();
	btSerial->turnOnBlueTooth();
	_timeToTurnOnAlarm = millis() + 300000;
	_isAlarmOn = false;
	//	}
	_isMasterMode = false;
}

void blinkLedHideMode()
{
	if (_isBlueLedDisable) { return; }
	for (uint8_t i = 0; i < 3; i++)
	{
		digitalWrite(_pin_powerLed, HIGH);
		delay(50);
		digitalWrite(_pin_powerLed, LOW);
		delay(50);
	}
}

void blinkLed(uint8_t blinkDelay,uint8_t numberOfBlinks)
{
	for (uint8_t i = 0; i < numberOfBlinks; i++)
	{
		digitalWrite(_pin_powerLed, HIGH);
		delay(blinkDelay);
		digitalWrite(_pin_powerLed, LOW);
		delay(blinkDelay);
	}
}

String splitStringIndex(String data, char separator, int index)
{
	int found = 0;
	int strIndex[] = { 0, -1 };
	int maxIndex = data.length() - 1;

	for (int i = 0; i <= maxIndex && found <= index; i++) {
		if (data.charAt(i) == separator || i == maxIndex) {
			found++;
			strIndex[0] = strIndex[1] + 1;
			strIndex[1] = (i == maxIndex) ? i + 1 : i;
		}
	}
	return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

String calculateBatteryLevel(float batteryLevel)

{
	if (batteryLevel <= 3.25)
		return F("[    ]+");
	if (batteryLevel <= 3.30)
		return F("[|   ]+");
	if (batteryLevel <= 3.40)
		return F("[||  ]+");
	if (batteryLevel <= 3.60)
		return F("[||| ]+");
	if (batteryLevel <= 5.50)
		return F("[||||]+");

}

void loadMainMenu()
{
	char* alarmStatus = new char[15];

	if (_isAlarmOn)
	{
		String(F("Alarm ON")).toCharArray(alarmStatus, 15);
	}
	else
	{
		String(F("Alarm OFF")).toCharArray(alarmStatus, 15);
	}

	char result[30];   // array to hold the result.

	strcpy(result, alarmStatus); // copy string one into the result.

	strcat(result, version); // append string two to the result.

	int internalTemperature = getTemp();//chipTemp->celsius();

	delete(alarmStatus);

	String battery = calculateBatteryLevel(_voltageValue);

	btSerial->println(BlueToothCommandsUtil::CommandConstructor(result, BlueToothCommandsUtil::Title));

	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Configuration", BlueToothCommandsUtil::Menu, F("001")));

	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Security", BlueToothCommandsUtil::Menu, F("004")));

	if (!_isAlarmOn)
	{
		btSerial->println(BlueToothCommandsUtil::CommandConstructor("Alarm On", BlueToothCommandsUtil::Command, F("002")));
	}
	else
	{
		btSerial->println(BlueToothCommandsUtil::CommandConstructor("Alarm OFF", BlueToothCommandsUtil::Command, F("003")));
	}

	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Temp.:" + String(internalTemperature), BlueToothCommandsUtil::Info));

	/*btSerial->println(BlueToothCommandsUtil::CommandConstructor("Batt.value:" + String(_voltageValue), BlueToothCommandsUtil::Info));*/

	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Batt.level:" + battery, BlueToothCommandsUtil::Info));

	btSerial->println(BlueToothCommandsUtil::CommandConstructor("WhatzUp:" + _whatIsHappened, BlueToothCommandsUtil::Info));

	//btSerial->println(BlueToothCommandsUtil::CommandConstructor("Signal:" + _signalStrength, BlueToothCommandsUtil::Info));

	btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));

	btSerial->Flush();

}

void loadConfigurationMenu()
{
	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Configuration", BlueToothCommandsUtil::Title));

	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Phone:" + String(_phoneNumber), BlueToothCommandsUtil::Data, F("001")));

	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Ph.Altern.:" + String(_phoneNumberAlternative), BlueToothCommandsUtil::Data, F("099")));

	btSerial->println(BlueToothCommandsUtil::CommandConstructor("N.Phone:" + String(_phoneNumbers), BlueToothCommandsUtil::Data, F("098")));

	btSerial->println(BlueToothCommandsUtil::CommandConstructor("TempMax:" + String(_tempMax), BlueToothCommandsUtil::Data, F("004")));

	btSerial->println(BlueToothCommandsUtil::CommandConstructor("OffSetTemp:" + String(_offSetTempValue), BlueToothCommandsUtil::Data, F("095")));

	//btSerial->println(BlueToothCommandsUtil::CommandConstructor("Apn:" + _apn, BlueToothCommandsUtil::Data, F("096")));

	if (_findOutPhonesMode != 2)
	{
		btSerial->println(BlueToothCommandsUtil::CommandConstructor("PIR status:" + String(_isPIRSensorActivated), BlueToothCommandsUtil::Data, F("005")));
	}

	if (_findOutPhonesMode != 0)
	{
		btSerial->println(BlueToothCommandsUtil::CommandConstructor("Addr:" + _deviceAddress, BlueToothCommandsUtil::Data, F("010")));

		btSerial->println(BlueToothCommandsUtil::CommandConstructor("Name:" + _deviceName, BlueToothCommandsUtil::Data, F("011")));

		btSerial->println(BlueToothCommandsUtil::CommandConstructor("Addr2:" + _deviceAddress2, BlueToothCommandsUtil::Data, F("015")));

		btSerial->println(BlueToothCommandsUtil::CommandConstructor("Name2:" + _deviceName2, BlueToothCommandsUtil::Data, F("016")));

		btSerial->println(BlueToothCommandsUtil::CommandConstructor("FindLoop:" + String(_delayFindMe), BlueToothCommandsUtil::Data, F("094")));
	}

	btSerial->println(BlueToothCommandsUtil::CommandConstructor("FindMode:" + String(_findOutPhonesMode), BlueToothCommandsUtil::Data, F("012")));

	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Ext.Int:" + String(_isExternalInterruptOn), BlueToothCommandsUtil::Data, F("013")));

	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Buzz.:" + String(_isBuzzerOn), BlueToothCommandsUtil::Data, F("014")));

	btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));

}

void blueToothConfigurationSystem()
{
	LSG_EEpromRW* eepromRW = new LSG_EEpromRW();
	String _bluetoothData = "";
	if (btSerial->available())
	{
		_bluetoothData = btSerial->readString();
		//BluetoothData.trim();

		//ROOT: Main
#pragma region Main Menu-#0
		if (_bluetoothData.indexOf(F("#0")) > -1)
		{
			_timeToTurnOnAlarm = millis() + 300000;
			loadMainMenu();
		}

#pragma region Commands

		if (_bluetoothData.indexOf(F("C002")) > -1)
		{
			_isAlarmOn = true;
			_isOnMotionDetect = false;
			_timeToTurnOnAlarm = 0;
			_isDisableCall = false;
			loadMainMenu();
		}

		if (_bluetoothData.indexOf(F("C003")) > -1)
		{
			_isAlarmOn = false;
			loadMainMenu();
		}
#pragma endregion

#pragma region Data


#pragma endregion


#pragma endregion

		//ROOT Main/Configuration
#pragma region Configuration Menu-#M001
		if (_bluetoothData.indexOf(F("M001")) > -1)
		{
			_timeToTurnOnAlarm = millis() + 300000;
			loadConfigurationMenu();
		}
#pragma region Commands

#pragma endregion


#pragma region Data
		if (_bluetoothData.indexOf(F("D001")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_phoneNumber, BUFSIZEPHONENUMBER);
				eepromRW->eeprom_write_string(1, _phoneNumber);
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D094")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufDelayFindMe, BUFSIZEDEVICEADDRESS);
				eepromRW->eeprom_write_string(_addressDelayFindMe, _bufDelayFindMe);
				_delayFindMe = atoi(&_bufDelayFindMe[0]);
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D095")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufOffSetTemperature, BUFSIZEOFFSETTEMPERATURE);
				eepromRW->eeprom_write_string(_addressOffSetTemperature, _bufOffSetTemperature);
				_offSetTempValue = atoi(&_bufOffSetTemperature[0]);
			}

			loadConfigurationMenu();
		}

		//if (_bluetoothData.indexOf(F("D096")) > -1)
		//{
		//	String splitString = splitStringIndex(_bluetoothData, ';', 1);
		//	splitString.toCharArray(_bufApn, BUFSIZEAPN);
		//	eepromRW->eeprom_write_string(_addressApn, _bufApn);
		//	_apn = splitString;
		//	loadConfigurationMenu();
		//}

		if (_bluetoothData.indexOf(F("D098")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufDbPhoneON, BUFSIZEDBPHONEON);
				eepromRW->eeprom_write_string(_addressDBPhoneIsON, _bufDbPhoneON);
				_phoneNumbers = atoi(&_bufDbPhoneON[0]);
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D099")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);

			if (isValidNumber(splitString) || splitString == "#")
			{
				if (splitString == "#")
				{
					_phoneNumberAlternative[0] = '\0';
				}
				else
				{
					splitString.toCharArray(_phoneNumberAlternative, BUFSIZEPHONENUMBERALTERANATIVE);
				}
				eepromRW->eeprom_write_string(_addressStartBufPhoneNumberAlternative, _phoneNumberAlternative);
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D004")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufTemperatureMax, BUFSIZETEMPERATUREMAX);
				eepromRW->eeprom_write_string(16, _bufTemperatureMax);
				_tempMax = atoi(_bufTemperatureMax);
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D005")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufPirSensorIsON, BUFSIZEPIRSENSORISON);
				eepromRW->eeprom_write_string(19, _bufPirSensorIsON);
				_isPIRSensorActivated = atoi(&_bufPirSensorIsON[0]);
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D010")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufDeviceAddress, BUFSIZEDEVICEADDRESS);
				eepromRW->eeprom_write_string(_addressStartDeviceAddress, _bufDeviceAddress);
				_deviceAddress = splitString;
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D011")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufDeviceName, BUFSIZEDEVICENAME);
				eepromRW->eeprom_write_string(_addressStartDeviceName, _bufDeviceName);
				_deviceName = splitString;
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D015")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufDeviceAddress2, BUFSIZEDEVICEADDRESS);
				eepromRW->eeprom_write_string(_addressStartDeviceAddress2, _bufDeviceAddress2);
				_deviceAddress2 = splitString;
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D016")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufDeviceName2, BUFSIZEDEVICENAME);
				eepromRW->eeprom_write_string(_addressStartDeviceName2, _bufDeviceName2);
				_deviceName2 = splitString;
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D012")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufFindOutPhonesON, BUFSIZEFINDOUTPHONESON);
				eepromRW->eeprom_write_string(_addressStartFindOutPhonesON, _bufFindOutPhonesON);
				_findOutPhonesMode = atoi(&_bufFindOutPhonesON[0]);
				if (_findOutPhonesMode != 0)
				{
					_isBTSleepON = 0;
					if (_findOutPhonesMode == 2)
					{
						_isPIRSensorActivated = 0;
					}
				}
				else
				{
					_isBTSleepON = 1;
				}
			}

			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D013")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufExternalInterruptIsON, BUFSIZEEXTERNALINTERRUPTISON);
				eepromRW->eeprom_write_string(_addressExternalInterruptIsOn, _bufExternalInterruptIsON);
				_isExternalInterruptOn = atoi(&_bufExternalInterruptIsON[0]);
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D014")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufBuzzerIsON, BUFSIZEBUZZERISON);
				eepromRW->eeprom_write_string(_addressBuzzerIsOn, _bufBuzzerIsON);
				_isBuzzerOn = atoi(&_bufBuzzerIsON[0]);
			}
			loadConfigurationMenu();
		}

#pragma endregion

#pragma Configuration Menu endregion


#pragma region Security-M004
		if (_bluetoothData.indexOf(F("M004")) > -1)
		{
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Security"), BlueToothCommandsUtil::Title));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw.:"), BlueToothCommandsUtil::Menu, F("005")));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change name:"), BlueToothCommandsUtil::Menu, F("006")));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
		}
#pragma region Menu
		if (_bluetoothData.indexOf(F("M005")) > -1)
		{
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Insert old passw.:"), BlueToothCommandsUtil::Data, F("006")));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
		}

		if (_bluetoothData.indexOf(F("M006")) > -1)
		{
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Insert name:"), BlueToothCommandsUtil::Data, F("007")));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
		}
#pragma endregion


#pragma region Commands

#pragma endregion

#pragma region Data
		if (_bluetoothData.indexOf(F("D006")) > -1)
		{
			String confirmedOldPassword = splitStringIndex(_bluetoothData, ';', 1);

			if (_oldPassword == confirmedOldPassword)
			{
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Insert new passw:"), BlueToothCommandsUtil::Data, F("008")));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
			}
			else
			{
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Wrong passw:"), BlueToothCommandsUtil::Message));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
			}

		}

		if (_bluetoothData.indexOf(F("D008")) > -1)
		{
			_newPassword = splitStringIndex(_bluetoothData, ';', 1);
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Confirm pass:"), BlueToothCommandsUtil::Data, F("009")));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
		}

		if (_bluetoothData.indexOf(F("D009")) > -1)
		{
			if (_newPassword == splitStringIndex(_bluetoothData, ';', 1))
			{
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("changed:"), BlueToothCommandsUtil::Message));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
				delay(2000);
				btSerial->SetPassword(_newPassword);
				_oldPassword = _newPassword;
			}

			else
			{
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("passw. doesn't match"), BlueToothCommandsUtil::Message));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
				btSerial->println("D006");
			}
		}


		if (_bluetoothData.indexOf(F("D007")) > -1)
		{
			String btName = splitStringIndex(_bluetoothData, ';', 1);

			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("changed:"), BlueToothCommandsUtil::Message));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
			delay(2000);
			btSerial->SetBlueToothName(btName);
		}


#pragma endregion

#pragma endregion

		delay(100);
	}
	delete(eepromRW);
}

boolean isValidNumber(String str)
{
	for (byte i = 0; i < str.length(); i++)
	{
		if (isDigit(str.charAt(i))) return true;
	}
	return false;
}

void buzzerSensorActivity()
{
	for (uint8_t i = 0; i < 15; i++)
	{
		tone(_pin_buzzer, 400, 500);
		delay(1000);
		noTone(_pin_buzzer);
	}
}

void pirSensorActivity()
{
	if (_isDisableCall) { return; }
	if (_isPIRSensorActivated && _isAlarmOn)
	{
		if (digitalRead(_pin_pir))
		{
			blinkLedHideMode();
			_whatIsHappened = F("P");

			if (_findOutPhonesMode == 1)
			{
				if (!_isDeviceDetected)
				{
					if (_isBuzzerOn)
					{
						buzzerSensorActivity();
					}
					callSim900();
					_isMasterMode = false;
					//reedRelaySensorActivity(_pin_reedRelay);
				}
			}
			else
			{
				if (_isBuzzerOn)
				{
					buzzerSensorActivity();
				}
				callSim900();
				_isMasterMode = false;
			}
		}
	}
}

void reedRelaySensorActivity(uint8_t pin)
{
	pinMode(pin, OUTPUT);
	blinkLedHideMode();
}

void internalTemperatureActivity()
{
	if (_delayForTemperature->IsDelayTimeFinished(true))
	{
		if ((uint8_t)getTemp() > _tempMax)
		{

			_whatIsHappened = F("T");
			callSim900();
		}
	}
}

void voltageActivity()
{
	if (_delayForVoltage->IsDelayTimeFinished(true))
	{
		_voltageValue = (5.10 / 1023.00) * analogRead(A1);
		_voltageMinValue = 3.25;

		if (_voltageValue < _voltageMinValue)
		{
			_whatIsHappened = F("V");
			callSim900();
		}
	}
}

void readIncomingSMS()
{
	//Inserita per scaricare buffer e agevolare arrivo sms.
	mySim900->ReadIncomingChars2();

	mySim900->ATCommand("AT+CMGL");//=\"REC UNREAD\"");
	//mySim900->ATCommand("AT+CMGR=1");
	delay(100);
	/**/

	if (mySim900->IsAvailable() > 0)
	{
		String response = mySim900->ReadIncomingChars2();
		delay(500);
		response.trim();
		//Serial.print(F("####")); Serial.print(response); Serial.println(F("####"));
		//if (response.substring(0, 5) == F("+CMT:"))
		//if (response.indexOf("+CMT:") != -1)
		if (response.indexOf("+CMGL:") != -1)
		{
			blinkLedHideMode();
			int position = 0;

			position = response.indexOf('"', position);
			position = response.indexOf('"', position + 1);
			position = response.indexOf('"', position + 1);

			/*Serial.println(position);

			Serial.println(response.substring(position + 4, position + 14));

			Serial.println(response.substring(position + 19, position + 29));*/

			if (response.substring(position + 4, position + 14) != _phoneNumber &&
				response.substring(position + 19, position + 29) != _phoneNumber &&
				response.substring(position + 4, position + 14) != _phoneNumberAlternative &&
				response.substring(position + 19, position + 29) != _phoneNumberAlternative)
			{
				//Serial.println("Numero errato");
				return;
			}
			int index = response.lastIndexOf('"');
			String smsCommand = response.substring(index + 1, index + 7);
			smsCommand.trim();
			//Serial.println(smsCommand);
			delay(1000);
			listOfSmsCommands(smsCommand);
		}
	}
}

void listOfSmsCommands(String command)
{

	//Enable incoming call.
	if (command == F("P1"))
	{
		_phoneNumbers = 1;
		callSim900();
	}

	if (command == F("P2"))
	{
		_phoneNumbers = 2;
		callSim900();
	}


	//Enable incoming call.
	if (command == F("Rc"))
	{
		mySim900->enableIncomingCall(1);
	}

	if (command == F("Rs"))
	{
		mySim900->disableIncomingCall();
		callSim900();
	}

	//Disattiva chiamate
	if (command == F("Dc"))
	{
		_isDisableCall = true;
	}
	//Accende bluetooth
	if (command == F("Ab"))
	{
		turnOnBlueToothAndSetTurnOffTimer();
		blinkLed(500, 3);
	}
	//Accende led
	if (command == F("Al"))
	{
		_isBlueLedDisable = false;
		blinkLed(500,3);
	}
	//Check system
	if (command == F("Ck"))
	{
		callSim900();
	}
	////Position enable.
	//if (command == F("Pe"))
	//{
	//	_isPositionEnable = true;
	//}
	////Position disable.
	//if (command == F("Pd"))
	//{
	//	_isPositionEnable = false;
	//}

	//Attiva funzione non vedermi
	if (command == F("Nv"))
	{
		_findOutPhonesMode = 1;
		_isBTSleepON = false;
		_timeToTurnOnAlarm = 0;
		findOutPhonesONAndSetBluetoothInMasterModeActivity();
		blinkLed(500, 3);
	}
	//Attiva External interrupt normalmente aperto
	if (command == F("Eo"))
	{
		_isBTSleepON = true;
		_isPIRSensorActivated = 0;
		_findOutPhonesMode = 0;
		_isBuzzerOn = 0;
		_isExternalInterruptOn = 1;
		activateFunctionAlarm();
		btSerial->turnOffBlueTooth();
		_isExtenalInterruptNormalyClosed = false;
	}
	//Disattiva External interrupt
	if (command == F("Ex"))
	{
		_isExternalInterruptOn = 0;
	}
	//Attiva External interrupt normalmente chiuso
	if (command == F("Ec"))
	{
		_isBTSleepON = true;
		_isPIRSensorActivated = 0;
		_findOutPhonesMode = 0;
		_isBuzzerOn = 0;
		_isExternalInterruptOn = 1;
		activateFunctionAlarm();
		btSerial->turnOffBlueTooth();
		_isExtenalInterruptNormalyClosed = true;
	}

	//Attiva motion detect senza bluetooth
	if (command == F("Md"))
	{
		_isBTSleepON = true;
		_isPIRSensorActivated = 0;
		_findOutPhonesMode = 0;
		_isBuzzerOn = 0;
		activateFunctionAlarm();
		btSerial->turnOffBlueTooth();
	}

	//Attiva Buzzer
	if (command == F("Bz"))
	{
		_isBuzzerOn = 1;
		blinkLed(500,3);
	}

	//Attiva pir sensor senza bluetooth
	if (command == F("Wc"))
	{
		_isBTSleepON = true;
		_isPIRSensorActivated = 1;
		_findOutPhonesMode = 0;
	/*	_isBuzzerOn = 0;*/
		activateFunctionAlarm();
		btSerial->turnOffBlueTooth();
	}

	//Find me
	if (command == F("Fm"))
	{
		_isBTSleepON = false;
		_findOutPhonesMode = 2;
		_isPIRSensorActivated = 0;
		_isBuzzerOn = 0;
		activateFunctionAlarm();
	}
}

void activateFunctionAlarm()
{
	_timeToTurnOnAlarm = 0;
	_isDisableCall = false;
	_isAlarmOn = true;
	callSim900();
}

//void getCoordinates()
//{
//	mySim900->ReadIncomingChars2();
//
//	char * apnCommand = new char[50];
//
//	char * apnString = new char[25];
//
//	_apn.toCharArray(apnString, (_apn.length() + 1));
//
//	strcpy(apnCommand, "AT+SAPBR=3, 1,\"APN\", \"");
//
//	strcat(apnCommand, apnString);
//
//	strcat(apnCommand, "\"");
//
//
//	//Serial.println(apnCommand);
//
//	mySim900->ATCommand(apnCommand);
//
//	delete(apnString);
//
//	delete(apnCommand);
//
//	//"AT + SAPBR = 3, 1, \"Contype\", \"GPRS\""
//	/*mySim900->ATCommand("AT + SAPBR = 3, 1,\"APN\", \"internet.wind\"");*/
//	/*mySim900->ATCommand("AT + SAPBR = 3, 1,\"APN\", \"web.coopvoce.it\"");*/
//	//mySim900->ATCommand("AT + SAPBR = 3, 1,\"APN\", \"mobile.vodafone.it\"");
//	//mySim900->ATCommand("AT + SAPBR = 3, 1,\"APN\", \"wap.tim.it\"");
//	/*mySim900->ATCommand("AT + SAPBR = 3, 1,\"APN\", \"ibox.tim.it\"");*/
//
//	delay(1500);
//	if (mySim900->IsAvailable() > 0)
//	{
//		//Serial.println(mySim900->ReadIncomingChars2());
//		mySim900->ReadIncomingChars2();
//
//	}
//
//	mySim900->ATCommand("AT+SAPBR=0,1");
//	delay(2000);
//	if (mySim900->IsAvailable() > 0)
//	{
//		//Serial.println(mySim900->ReadIncomingChars2());
//		mySim900->ReadIncomingChars2();
//
//	}
//	mySim900->ATCommand("AT+SAPBR=1,1");
//	delay(2000);
//	if (mySim900->IsAvailable() > 0)
//	{
//		//Serial.println(mySim900->ReadIncomingChars2());
//		mySim900->ReadIncomingChars2();
//
//	}
//	mySim900->ATCommand("AT+SAPBR=2,1");
//	delay(5500);
//	if (mySim900->IsAvailable() > 0)
//	{
//		//Serial.println(mySim900->ReadIncomingChars2());
//		mySim900->ReadIncomingChars2();
//
//	}
//
//	mySim900->ATCommand("AT+CIPGSMLOC=1,1");
//	delay(10000);
//	if (mySim900->IsAvailable() > 0)
//	{
//		String h = mySim900->ReadIncomingChars2();
//		h.trim();
//
//		if (h.substring(19, 30) == F("+CIPGSMLOC:"))
//		{
//			//Serial.println("Entrato");
//			String b = h.substring(33, 42);
//			String a = h.substring(43, 52);
//			String site = F("google.com/maps/search/?api=1&query=");
//			site = site + a + ',' + b;
//			mySim900->SendTextMessageSimple(site, String(_phoneNumber));
//		}
//
//		
//
//	}
//}

double getTemp(void)
{
	unsigned int wADC;
	double t;

	// The internal temperature has to be used
	// with the internal reference of 1.1V.
	// Channel 8 can not be selected with
	// the analogRead function yet.

	// Set the internal reference and mux.
	ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
	ADCSRA |= _BV(ADEN);  // enable the ADC

	delay(20);            // wait for voltages to become stable.

	ADCSRA |= _BV(ADSC);  // Start the ADC

						  // Detect end-of-conversion
	while (bit_is_set(ADCSRA, ADSC));

	// Reading register "ADCW" takes care of how to read ADCL and ADCH.
	wADC = ADCW;

	// The offset of 324.31 could be wrong. It is just an indication.
	t = (wADC - _offSetTempValue) / 1.22;

	// The returned temperature is in degrees Celsius.
	return (t);
}

//unsigned int offSetTempValue(double externalTemperature)
//{
//	unsigned int wADC;
//	double t;
//	// The internal temperature has to be used
//	// with the internal reference of 1.1V.
//	// Channel 8 can not be selected with
//	// the analogRead function yet.
//
//	// Set the internal reference and mux.
//	ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
//	ADCSRA |= _BV(ADEN);  // enable the ADC
//
//	delay(20);            // wait for voltages to become stable.
//
//	ADCSRA |= _BV(ADSC);  // Start the ADC
//
//						  // Detect end-of-conversion
//	while (bit_is_set(ADCSRA, ADSC));
//
//	// Reading register "ADCW" takes care of how to read ADCL and ADCH.
//	wADC = ADCW;
//
//	// The offset of 324.31 could be wrong. It is just an indication.
//	//t = (wADC - _offSetTempValue) / 1.22;
//
//	// The returned temperature is in degrees Celsius.
//	return (-(externalTemperature * 1.22) + wADC);
//}