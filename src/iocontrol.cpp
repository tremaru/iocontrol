#include "iocontrol.h"
//tabstop=8
//#define __DEBUG__

//TODO: discard header by '{'?
//TODO: changable mac address
//TODO: replace readAll() with update() or schedule()

#define jsonBufSiz 150

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
	ethConfigFail = 901
};

char headerEnd[] = "\r\n\r\n";
const uint8_t _defaultMac[6] = {
	0xFE, 0xED, 0xDE, 0xAD, 0xBE, 0xEF
};

iocontrol::iocontrol(const char* boardName)
	: _mac(_defaultMac)
{
	_boardName = boardName;
	//_mac[6] = {0xFE, 0xED, 0xDE, 0xAD, 0xBE, 0xEF};
}

iocontrol::iocontrol(const char* boardName, const uint8_t* mac)
	: _mac(mac)
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
	return readAll();
}

//TODO: replace with schedule() or update() that reads and checks for pending writes.
int iocontrol::readAll()
{
	if (millis() - currentMillis > _intervalR || !_created) {
		if (!_httpRequest()) {
			return connectionFailed;
		}

		_client.println(
				(String)"GET /api/readDataAll.php?key="
				+ key + "&board=" + _boardName
				+ " HTTP/1.1"
			       );

		_client.println("Host: www.iocontrol.ru");
		_client.println("User-Agent: arduino-ethernet");
		_client.println("Connection: close");
		_client.println();

		currentMillis = millis();
		//}
		//delay(10000);
		//}

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
		bool check;
		int jsonError = _parseJson(check, s, "check");

		if (jsonError != 0) {
			return jsonError;
		}

		else if (!check) {
			return _parseJson(jsonError, s, "message");
		}

		else {
			//Serial.println(check);
			_parseJson(_boardSize, s, "countVariable");
			//Serial.println(_boardSize);
		}


		//create structure if valid board size

		if (_boardSize == 0) {
			return emptyBoard;
		}

		else if (!_created) {
			_boardVars = new t_item[_boardSize];
			_created = true;
#ifdef __DEBUG__
			Serial.println("Start!");
#endif
		}

		//fill intervals

		if (!_intervalSet) {
			int tmp;
			int i = 0;
			jsonError = _parseJson(tmp, s, "timeR");
			if (tmp > 0) {
				_intervalR = 1000 * tmp;
				i++;
				//Serial.println(_intervalR);
			}
			_parseJson(tmp, s, "timeW");
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
	return 0;
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
	return;
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

int iocontrol::_parseJson(bool& ioBool, String& json, String field)
{
	int error = 0;

	JsonObject& root = _parseJsonRoot(json, error);

	if (error)
		return error;

	ioBool = root[field];

	return 0;
}

int iocontrol::_parseJson(int& ioInt, String& json, String field)
{
	int error = 0;

	JsonObject& root = _parseJsonRoot(json, error);

	if (error)
		return error;

	ioInt = root[field];

	return 0;
}

int iocontrol::_parseJson(long& ioInt, String& json, String field)
{
	int error = 0;

	JsonObject& root = _parseJsonRoot(json, error);

	if (error)
		return error;

	ioInt = root[field];

	return 0;
}

int iocontrol::_parseJson(float& ioFloat, String& json, String field)
{
	int error = 0;

	JsonObject& root = _parseJsonRoot(json, error);

	if (error)
		return error;

	ioFloat = root[field];

	return 0;
}

int iocontrol::_parseJson(String& ioString, String& json, String field)
{
	int error = 0;

	JsonObject& root = _parseJsonRoot(json, error);

	if (error)
		return error;

	char* tmp = root[field];
	ioString = (String)tmp;

	return 0;
}

JsonObject& iocontrol::_parseJsonRoot(String& json, int& error)
{
	if (json == "") {
		error =  emptyJson;
		return;
	}

	if (json.length() > jsonBufSiz) {
		error = bigJson;
		return;
	}
	char buf[jsonBufSiz]{0};
	json.toCharArray(buf, sizeof(buf));
	//DynamicJsonBuffer jsonBuffer(200);
	StaticJsonBuffer<jsonBufSiz> jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(buf);

	if (!root.success()) {
		error =  failedJsonRoot;
#ifdef __DEBUG__
		Serial.println(json);
#endif
		return;
	}

	return root;
}

int iocontrol::_fillData(int& i)
{

	String s = _client.readStringUntil('}');
	s.concat("}");

	String type = "";
	int jsonError = _parseJson(type, s, "type");
        _parseJson(_boardVars[i].name, s, "variable");

	if (jsonError)
		return jsonError;

	if (type == "int") {

		_boardVars[i].v_type = is_int;
		jsonError = _parseJson(_boardVars[i]._int, s, "value");
	}
	else if (type == "float") {

		_boardVars[i].v_type = is_float;
		jsonError = _parseJson(_boardVars[i]._float, s, "value");
	}
	else if (type == "string") {

		_boardVars[i].v_type = is_string;
		String tmp = "";
		jsonError = _parseJson(tmp, s, "value");
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
int iocontrol::write(const String& varName, int var)
{
	if (!_httpRequest())
		return connectionFailed;

	_client.println(
			(String)"GET /api/sendData/"
			+ _boardName + "/" + varName + "/"
			+ var + "?key=" + key + " HTTP/1.1"
		       );

	_rest();
	return 0;
}

int iocontrol::write(const String& varName, float var)
{
	//Serial.println(var, 5);
	if (!_httpRequest())
		return connectionFailed;

	String flt = String(var, 5);
	String req = (String)"GET /api/sendData/"
			+ _boardName + "/" + varName + "/"
			+ flt + "?key=" + key + " HTTP/1.1";

	Serial.println(req);

	_client.println( req );
////////		(String)"GET /api/sendData/"
////////		+ _boardName + "/" + varName + "/"
////////		+ var + "?key=" + key + " HTTP/1.1"
////////	       );
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
			+ var + "?key=" + key + " HTTP/1.1"
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
			+ var + "?key=" + key + " HTTP/1.1"
		       );
	_rest();
	return 0;
}

void iocontrol::_rest()
{
	_client.println("Host: www.iocontrol.ru");
	_client.println("User-Agent: arduino-ethernet");
	_client.println("Connection: close");
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
