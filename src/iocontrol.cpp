#include "iocontrol.h"
//tabstop=8
//#define __DEBUG__

//TODO: discard header by '{'?
//TODO: add writeUpdate()

#define jsonBufSiz 150

StaticJsonDocument<jsonBufSiz> root;
// errors
enum {
	emptyJson = 601,
	failedJsonRoot = 602,
	noType = 603,
	emptyBoard = 604,
	bigJson = 605,
	invalidHeader = 701,
	intervalError = 702,
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
	//_mac[6] = {0xFE, 0xED, 0xDE, 0xAD, 0xBE, 0xEF};
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
	//Serial.println(Ethernet.localIP());
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

	if (millis() - currentMillis > _intervalR || !_created) {
		if (!_httpRequest()) {
			return connectionFailed;
		}
		///api/readDataAll/test2/0
		_client.println(
				(String)"GET /api/readDataAll/"
				+ _boardName + "/" + _key
				+ " HTTP/1.1"
			       );

		_rest();

		currentMillis = millis();

		if (int h_status = _httpStatus() != 200) {
			//Serial.println("gottcha!");
			return h_status;
		}

		if (!_discardHeader())
			return invalidHeader;

		if (_client.available()) {

			//_client.readStringUntil('\n'); // bullshit line with number that correlates with the size of the response after header. WTF?
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
				//Serial.println(check);
				_parseJson(_boardSize, s, F("countVariable"));
				//Serial.println(_boardSize);
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
					//Serial.println(_intervalR);
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

			//String tmp = "void";
			//readInt(tmp);
			//Serial.write(_client.read());
		}
		_client.stop();
	}
	else
		return intervalError;


	return 0;
}

int iocontrol::writeUpdate()
{
	if (!_boardExists)
		return invalidName;

	if (millis() - currentMillis > _intervalW && _created) {
		if (!_httpRequest()) {
			return connectionFailed;
		}
		for (int i = 0; i < _boardSize; i++) {
			if (_boardVars[i]._pending) {
				_sendData(i);
			}
		}
		//////
	}
	else
		return intervalError;

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

	//StaticJsonDocument<jsonBufSiz> root;
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

	//StaticJsonDocument<jsonBufSiz> root;
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

	//StaticJsonDocument<jsonBufSiz> root;
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

	//StaticJsonDocument<jsonBufSiz> root;
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

	//StaticJsonDocument<jsonBufSiz> root;
	DeserializationError error = deserializeJson(root, json);

	if (error)
		return failedJsonRoot;

	char* tmp = root[field];
	ioString = (String)tmp;

	return 0;
}

/*JsonObject iocontrol::_parseJsonRoot(String& json, int& error)
{
	if (json == "") {
		error =  emptyJson;
		return;
	}


	if (json.length() > jsonBufSiz) {
		error = bigJson;
		return;
	}
	StaticJsonDocument<200> root;
	DeserializationError err = deserializeJson(root, json);
	if (err) {
		error = failedJsonRoot;
		return;
	}

	//char buf[jsonBufSiz]{0};
	//json.toCharArray(buf, sizeof(buf));
	////DynamicJsonBuffer jsonBuffer(200);
	//StaticJsonBuffer<jsonBufSiz> jsonBuffer;
	//JsonObject& root = jsonBuffer.parseObject(buf);

	//if (!root.success()) {
	//	error =  failedJsonRoot;
#ifdef __DEBUG__
		Serial.println(json);
#endif
//		return;
//	}

	return root.as<JsonObject>();
}*/

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
		jsonError = _parseJson(_boardVars[i]._int, s, F("value"));
	}
	else if (type == F("float")) {

		_boardVars[i].v_type = is_float;
		jsonError = _parseJson(_boardVars[i]._float, s, F("value"));
	}
	else if (type == F("string")) {

		_boardVars[i].v_type = is_string;
		String tmp = "";
		jsonError = _parseJson(tmp, s, F("value"));
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

//template <typename T> int iocontrol::write(String varName, T var)
void iocontrol::write(const String& varName, int var)
{
	for (int i = 0; i < _boardSize; i++) {
		if (_boardVars[i].name == varName
				&& _boardVars[i].v_type == is_int) {
			_boardVars[i]._int = var;
			_boardVars[i]._pending = true;
		}
	}
	//if (!_httpRequest())
	//	return connectionFailed;

	//_client.println(
	//		(String)"GET /api/sendData/"
	//		+ _boardName + "/" + varName + "/"
	//		+ var + "?key=" + _key + " HTTP/1.1"
	//	       );

	//_rest();
	//return 0;
}

int iocontrol::write(const String& varName, float var)
{
	if (!_httpRequest())
		return connectionFailed;

	String flt = String(var, 5);
	String req = (String)"GET /api/sendData/"
			+ _boardName + "/" + varName + "/"
			+ flt + "?key=" + _key + " HTTP/1.1";

	_client.println(req);
	_rest();
	return 0;
}

int iocontrol::write(const String& varName, float var, uint8_t prec)
{
	if (!_httpRequest())
		return connectionFailed;

	String flt = String(var, prec);
	String req = (String)"GET /api/sendData/"
			+ _boardName + "/" + varName + "/"
			+ flt + "?key=" + _key + " HTTP/1.1";

	_client.println(req);
	_rest();
	return 0;
}

int iocontrol::write(const String& varName, String var)
{
	if (!_httpRequest())
		return connectionFailed;

	_client.println(
			(String)"GET /api/sendData/"
			+ _boardName + "/" + varName + "/"
			+ var + "?key=" + _key + " HTTP/1.1"
		       );
	_rest();
	return 0;
}

int iocontrol::write(const String& varName, bool var)
{
	if (!_httpRequest())
		return connectionFailed;

	_client.println(
			(String)"GET /api/sendData/"
			+ _boardName + "/" + varName + "/"
			+ var + "?key=" + _key + " HTTP/1.1"
		       );
	_rest();
	return 0;
}

void iocontrol::_rest()
{
	_client.println(F("Host: www.iocontrol.ru"));
	_client.println(F("User-Agent: arduino-ethernet"));
	_client.println(F("Connection: close"));
	_client.println();

}

/*int iocontrol::_send()
{
	for (uint8_t i = 0; i < _boardSize; i++) {
		if (_boardVars[i]._pending) {
			//TODO;
		}
	}
	return 0;

}*/
//template <> int iocontrol::write(String varName, int var);
//int iocontrol::Write(String name, float var)
//{
//	if (!_httpRequest())
//		return connectionFailed;
//}
//
//int iocontrol::Write(String name, String var)
//{
//	if (!_httpRequest())
//		return connectionFailed;
//}
