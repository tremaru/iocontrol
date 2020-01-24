#include "iocontrol.h"
//tabstop=8
//#define __DEBUG__
//TODO: discard header by '{'?
//TODO: fix memmory leak on _boardVars[i]._string; figure out "delete"
//TODO: mac address
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
IPAddress _server(188,124,36,57);
byte mac[6] = {
	0xFE, 0xED, 0xDE, 0xAD, 0xBE, 0xEF
};

iocontrol::iocontrol(const char* boardName)
{
	_boardName = boardName;
#ifdef __DEBUG__
	_boardVars = new t_item[10];
	char* var = "myVar";
	_boardVars[0].name = (char*)malloc(sizeof(var));
	_boardVars[0].name = var;
	_boardvars[0]._int = 42;
	//char* var2 = "my2ndVar";
	//_panel_vars[1].name = (char*)malloc(sizeof(var2));
	_boardVars[1].name = "my2ndVar";
	//_panel_vars[1].name = var2;
#endif
}

int iocontrol::begin()
{
	if (!Ethernet.begin(mac)) {
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

int iocontrol::readAll()
{
	if (millis() - currentMillis > _interval || !_created) {
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

                ///////
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
#ifdef __DEBUG__
	Serial.println(int(_panel_vars[0]._int));
	Serial.println(_panel_vars[1].name);
	Serial.println(_server);
	//std::cout << int(_panel_vars[0]._int) << '\n';
		//std::cout << _panel_vars[1].name << '\n';
#endif
}
else
	return intervalError;
}

long iocontrol::readInt(String name)
{
	for (int i = 0; i < _boardSize; i++) {
		if (_boardVars[i].name == name)
			if (_boardVars[i].v_type == is_int)
				return _boardVars[i]._int;
	}
	return 0;
}

float iocontrol::readFloat(String name)
{
	for (int i = 0; i < _boardSize; i++) {
		if (_boardVars[i].name == name)
			if (_boardVars[i].v_type == is_float)
				return _boardVars[i]._float;
	}
	return 0;
}

char* iocontrol::readCstring(String name)
{
	for (int i = 0; i < _boardSize; i++) {
		if (_boardVars[i].name == name)
			if (_boardVars[i].v_type == is_string)
				return _boardVars[i]._string;
	}
	return;
}

String iocontrol::readString(String name)
{
////////for (int i = 0; i < _boardSize; i++) {
////////	if (_boardVars[i].name == name)
////////		if (_boardVars[i].v_type == is_string)
////////			return String(_boardVars[i]._string);
////////}
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
	//
	//Serial.println(status);
	//
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

	//Serial.println(sizeof(root[field]));
	char* tmp = root[field];
	//Serial.println(sizeof(tmp));
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
		//
		Serial.println(json);
		//
		return;
	}

	return root;
}

int iocontrol::_fillData(int& i)
{

	//Serial.println(i);
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
		//
		_boardVars[i].v_type = is_float;
		jsonError = _parseJson(_boardVars[i]._float, s, "value");
	}
	else if (type == "string") {
		;
		//
//      	_boardVars[i].v_type = is_string;
//              String tmp = "";
//      	jsonError = _parseJson(tmp, s, "type");
//              _boardVars[i]._string = new char[tmp.length() + 1];
//              tmp.toCharArray(_boardVars[i]._string, tmp.length() + 1);
	}
	else
		return noType;

        if (jsonError)
                return jsonError;

	_client.read();

        return 0;
}

//template <typename T> int iocontrol::write(String varName, T var)
int iocontrol::write(String varName, int var)
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

int iocontrol::write(String varName, float var)
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

int iocontrol::write(String varName, String var)
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

int iocontrol::write(String varName, bool var)
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
