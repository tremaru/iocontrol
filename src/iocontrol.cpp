#include "iocontrol.h"

//tabstop=8
//#define __DEBUG__

//TODO: discard header by '{'?
//TODO: add writeUpdate()

#define jsonBufSiz 150
StaticJsonDocument<jsonBufSiz> root;

const char* legal = "abcdefghigklmnopqrstuvwxyz\
		     ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-";
const String value = "value";

// errors
enum {
	emptyJson = 601,
	failedJsonRoot = 602,
	noType = 603,
	emptyBoard = 604,
	bigJson = 605,
	nothingToWrite = 606,
	invalidHeader = 701,
	intervalError = 702,
	invalidResponse = 703,
	connectionFailed = 801,
	cableNotPlugged = 903,
	noEthHardware = 902,
	ethConfigFail = 901,
	invalidName = 1003
};

char headerEnd[] = "\r\n\r\n";

uint8_t _defaultMac[6] = {
	0xFE, 0xED, 0xDE, 0xAD, 0xBE, 0xEF
};

const char* _defaultKey = "0";

iocontrol::iocontrol(const char* boardName)
	: _mac(_defaultMac), _key(_defaultKey)
{
	_boardName = boardName;
}

iocontrol::iocontrol(const char* boardName, uint8_t* mac)
	: _mac(mac), _key(_defaultKey)
{
	_boardName = boardName;
}

iocontrol::iocontrol(const char* boardName, const char* key)
	: _mac(_defaultMac), _key(key)
{
	_boardName = boardName;
}

iocontrol::iocontrol(const char* boardName, const char* key, uint8_t* mac)
	: _mac(mac), _key(key)
{
	_boardName = boardName;
}

int iocontrol::begin()
{
	if (!Ethernet.begin(_mac)) {
		if (Ethernet.hardwareStatus() == EthernetNoHardware)
			return noEthHardware;
		else if (Ethernet.linkStatus() == LinkOFF)
			return cableNotPlugged;
		else
			return ethConfigFail;
	}

#ifdef __DEBUG__
	Serial.println(Ethernet.localIP());
#endif

	delay(1000);
	int error = readUpdate();

	if (error == invalidName)
		_boardExists = false;

	return error;
}

int iocontrol::readUpdate()
{
	if (!_boardExists)
		return invalidName;

	if (millis() - currentMillisR > _intervalR || !_created) {
		currentMillisR = millis();

		if (!_httpRequest()) {
			return connectionFailed;
		}

		_client.println(
				(String)"GET /api/readDataAll/"
				+ _boardName + "/" + _key
				+ " HTTP/1.1"
			       );

		_rest();


		if (int h_status = _httpStatus() != 200) {
			return h_status;
		}

		if (!_discardHeader())
			return invalidHeader;

		if (_client.available()) {

			_client.find('{');
			String s = "{" + _client.readStringUntil('[');
			s.concat("\"\"}");

			// parse if responese was succsessful
			// get boardSize
			bool check = false;
			int jsonError = _parseJson(check, s, F("check"));

			if (jsonError != 0) {
				return jsonError;
			}

			else if (!check) {
				return _parseJson(jsonError, s, F("message"));
			}

			else {
				_parseJson(_boardSize, s, F("countVariable"));
			}


			//create structure if valid board size

			if (_boardSize == 0) {
				return emptyBoard;
			}

			else if (!_created) {
				_boardVars = new t_item[_boardSize];
				_created = true;
			}

			//fill intervals

			if (!_intervalSet) {
				int tmp;
				int i = 0;
				jsonError = _parseJson(tmp, s, F("timeR"));
				if (tmp > 0) {
					_intervalR = 1000 * tmp;
					i++;
				}
				_parseJson(tmp, s, F("timeW"));
				if (tmp > 0) {
					_intervalW = 1000 * tmp;
					i++;
				}

				if (i == 2)
					_intervalSet = true;
			}

			//fill structure
			int i;
			for (_currentPlace = _boardSize, i = 0;
					_currentPlace > 0;
					--_currentPlace, i++) {

				jsonError = _fillData(i);
				if (jsonError)
					return jsonError;
			}

		}
		_client.stop();
	}
	else
		return intervalError;


	return 0;
}

int iocontrol::writeUpdate()
{
	bool writeFlag = false;
	if (!_boardExists)
		return invalidName;

	if (millis() - currentMillisW > _intervalW && _created) {

		currentMillisW = millis();


		String reqString = "";

		for (int i = 0; i < _boardSize; i++) {
			if (_boardVars[i]._pending) {
				writeFlag = true;
				reqString += _boardVars[i].name;
				reqString += ":";
				reqString += _prepData(i);
				reqString += ",";
			}
		}

		if (writeFlag)
			return _sendData(reqString);

		else
			return nothingToWrite;

	}
	else
		return intervalError;

}

String iocontrol::_prepData(int& i)
{
	String empty = "";
	switch (_boardVars[i].v_type) {

		case is_int:
			return String(_boardVars[i]._int);

		case is_float:
			return String(_boardVars[i]._float, _boardVars[i]._prec);

		case is_string:

			if (_boardVars[i]._string
					&& strspn(_boardVars[i]._string, legal)
					== strlen(_boardVars[i]._string))
				return String(_boardVars[i]._string);

			else
				return empty;


		default:
			return empty;
	}
}

int iocontrol::_sendData(String& req)
{

	if (!_httpRequest()) {
		return connectionFailed;
	}
/*
	req = (String)"GET /api/sendDataAll/"
	+ _boardName + "/" + _key + "/"
	+ req + " HTTP/1.1";
////
////
*/
#ifdef __DEBUG__
	Serial.println(req);
#endif
	_client.println(
			(String)"GET /api/sendDataAll/"
			+ _boardName + "/" + _key + "/"
			+ req + " HTTP/1.1"
		       );
	_rest();

	if (int h_status = _httpStatus() != 200) {
		return h_status;
	}

	if (!_discardHeader())
		return invalidHeader;

	if (_client.available()) {

		_client.find('{');
		String s = "{" + _client.readStringUntil('{');
		s.concat("\"\"}");

		bool check = false;
		int jsonError = _parseJson(check, s, F("check"));


		if (jsonError)
			return jsonError;

		else if (!check)
			return _parseJson(jsonError, s, F("message"));


		check = false;
		s = "{" + _client.readStringUntil('}');
		s.concat("}");
#ifdef __DEBUG__
		Serial.println(s);
#endif

		for (int i = 0; i < _boardSize; i++) {
			jsonError = _parseJson(check, s, String(_boardVars[i].name));
			if (jsonError)
				return jsonError;
#ifdef __DEBUG__
			Serial.print(jsonError);
			Serial.print("\t\t\t");
			Serial.print(check);
			Serial.print("\t\t\t");
			Serial.println(_boardVars[i]._tries);
#endif

			if (check)
				_boardVars[i]._pending = false;

			else if (_boardVars[i]._pending)
				_boardVars[i]._tries--;


			if (!_boardVars[i]._tries) {
				_boardVars[i]._pending = false;
				_boardVars[i]._tries = MAX_TRIES;
			}
		}

	}
	else
		return invalidResponse;
}

long iocontrol::readInt(const String& name)
{
	for (int i = 0; i < _boardSize; i++) {
		if (_boardVars[i].name == name)
			if (_boardVars[i].v_type == is_int)
				return _boardVars[i]._int;
	}
	return 0;
}

float iocontrol::readFloat(const String& name)
{
	for (int i = 0; i < _boardSize; i++) {
		if (_boardVars[i].name == name)
			if (_boardVars[i].v_type == is_float)
				return _boardVars[i]._float;
	}
	return 0;
}

char* iocontrol::readCstring(const String& name)
{
	for (int i = 0; i < _boardSize; i++) {
		if (_boardVars[i].name == name)
			if (_boardVars[i].v_type == is_string)
				return _boardVars[i]._string;
	}
	return 0;
}

String iocontrol::readString(const String& name)
{
	return String(readCstring(name));
}

bool iocontrol::_httpRequest()
{
	_client.stop();

	if (_client.connect(_server, 80)) {
		return true;
	}
	else {
		return false;
	}
}

bool iocontrol::_discardHeader()
{
	return _client.find(headerEnd);
}

int iocontrol::_httpStatus()
{
	String status = _client.readStringUntil('\n');
#ifdef __DEBUG__
	Serial.println(status);
#endif
	status = status.substring(9,13);
	return status.toInt();
}

int iocontrol::_parseJson(bool& ioBool, String& json, const String& field)
{
	if (json == "")
		return emptyJson;

	if (json.length() > jsonBufSiz)
		return bigJson;

	DeserializationError error = deserializeJson(root, json);

	if (error)
		return failedJsonRoot;

	ioBool = root[field];

	return 0;
}

int iocontrol::_parseJson(int& ioInt, String& json, const String& field)
{
	if (json == "")
		return emptyJson;

	if (json.length() > jsonBufSiz)
		return bigJson;

	DeserializationError error = deserializeJson(root, json);

	if (error)
		return failedJsonRoot;

	ioInt = root[field];

	return 0;
}

int iocontrol::_parseJson(long& ioInt, String& json, const String& field)
{
	if (json == "")
		return emptyJson;

	if (json.length() > jsonBufSiz)
		return bigJson;

	DeserializationError error = deserializeJson(root, json);

	if (error)
		return failedJsonRoot;

	ioInt = root[field];

	return 0;
}

int iocontrol::_parseJson(float& ioFloat, String& json, const String& field)
{
	if (json == "")
		return emptyJson;

	if (json.length() > jsonBufSiz)
		return bigJson;

	DeserializationError error = deserializeJson(root, json);

	if (error)
		return failedJsonRoot;

	ioFloat = root[field];

	return 0;
}

int iocontrol::_parseJson(String& ioString, String& json, const String& field)
{
	if (json == "")
		return emptyJson;

	if (json.length() > jsonBufSiz)
		return bigJson;

	DeserializationError error = deserializeJson(root, json);

	if (error)
		return failedJsonRoot;

	char* tmp = root[field];
	ioString = (String)tmp;

	return 0;
}

int iocontrol::_fillData(int& i)
{

	String s = _client.readStringUntil('}');
	s.concat("}");

	String type = "";
	int jsonError = _parseJson(type, s, F("type"));
        _parseJson(_boardVars[i].name, s, F("variable"));

	if (jsonError)
		return jsonError;

	if (type == F("int")) {

		_boardVars[i].v_type = is_int;
		jsonError = _parseJson(_boardVars[i]._int, s, value);
	}
	else if (type == F("float")) {

		_boardVars[i].v_type = is_float;
		jsonError = _parseJson(_boardVars[i]._float, s, value);
	}
	else if (type == F("string")) {

		_boardVars[i].v_type = is_string;
		String tmp = "";
		jsonError = _parseJson(tmp, s, value);

		if (_boardVars[i]._string)
			delete[] _boardVars[i]._string;

		_boardVars[i]._string = new char[tmp.length() + 1];
		tmp.toCharArray(_boardVars[i]._string, tmp.length() + 1);
	}

	else
		return noType;

        if (jsonError)
                return jsonError;

	_client.read();

        return 0;
}

void iocontrol::write(const String& varName, long var)
{
	for (int i = 0; i < _boardSize; i++) {
		if (_boardVars[i].name == varName
				&& _boardVars[i].v_type == is_int) {
			if (_boardVars[i]._int != var) {
				_boardVars[i]._int = var;
				_boardVars[i]._pending = true;
			}
		}
	}
}

void iocontrol::write(const String& varName, int var)
{
	write(varName, long(var));
}

void iocontrol::write(const String& varName, float var)
{
	write(varName, var, 2);
}

void iocontrol::write(const String& varName, float var, uint8_t prec)
{
	for (int i = 0; i < _boardSize; i++) {
		if (_boardVars[i].name == varName
				&& _boardVars[i].v_type == is_float) {
			if (abs(_boardVars[i]._float) - abs(var) > 0.00001) {
				_boardVars[i]._float = var;
				_boardVars[i]._prec = prec;
				_boardVars[i]._pending = true;
			}
		}
	}
	//String flt = String(var, prec);
}

void iocontrol::write(const String& varName, String var)
{
	for (int i = 0; i < _boardSize; i++) {
		if (_boardVars[i].name == varName
				&& _boardVars[i].v_type == is_string) {

			if (String(_boardVars[i]._string) != var) {

				if (_boardVars[i]._string)
					delete[] _boardVars[i]._string;

				_boardVars[i]._string = new char[var.length() + 1];
				var.toCharArray(_boardVars[i]._string,
						var.length() + 1);

				_boardVars[i]._pending = true;
			}
		}
	}
}

void iocontrol::write(const String& varName, bool var)
{
	write(varName, (int)var);
}

void iocontrol::_rest()
{
	_client.println(F("Host: www.iocontrol.ru"));
	_client.println(F("User-Agent: arduino-ethernet"));
	_client.println(F("Connection: close"));
	_client.println();

}
