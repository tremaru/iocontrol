#include "iocontrol.h"

// debug macro
#ifdef ESP32
#define PRINT(A) _debug_flag ? Serial.println(status2str(A)) : 0
#else
#define PRINT(A) do_nothing(A)
int do_nothing(int) { return 0; }
#endif

//tabstop=8
//#define __DEBUG__

// legal chars in request
const char* legal = "abcdefghigklmnopqrstuvwxyz\
		     ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-";

const String value = "value";
String status2str(int);

// errors
enum {
	httpOk = 200,
	emptyJson = 601,
	failedJsonRoot = 602,
	noType = 603,
	emptyBoard = 604,
	nothingToWrite = 606,
        nothingToRead = 607,
	invalidHeader = 701,
	intervalError = 702,
	invalidResponse = 703,
	invalidStatus = 704,
	connectionFailed = 801,
	invalidName = 1003
};

char headerEnd[] = "\r\n\r\n";

uint8_t _defaultMac[6] = {
	0xFE, 0xED, 0xDE, 0xAD, 0xBE, 0xEF
};

uint8_t rec_check = MAX_TRIES;

const char* _defaultKey = "0";

// constructor
iocontrol::iocontrol(const char* boardName, Client& client)
	: _boardName(boardName), _key(_defaultKey), _client(client)
{
	//_boardName = boardName;
}

// constructor with key
iocontrol::iocontrol(const char* boardName, const char* key, Client& client)
	: _boardName(boardName), _key(key), _client(client)
{
	//_boardName = boardName;
}

// destructor (just in case), no rule of zero, three, five or six six six is applied
iocontrol::~iocontrol()
{
	for (int i = 0; i < _boardSize; i++) {
		if (_boardVars[i]._string)
			delete[] _boardVars[i]._string;
	}

	delete[] _boardVars;
}

// calls readUpdate(), prevents further request if board doesn't exist
int iocontrol::begin()
{
	int error = readUpdate();

	if (error == invalidName)
		_boardExists = false;

	return error;
}

// reads all vars from the server, creates vars (if not created),
// updates vars from the server. Obj.begin() runs it one time
int iocontrol::readUpdate()
{
	// forbid bombarding the server with bogus board names
	if (!_boardExists) {
		PRINT(invalidName);
		return invalidName;
	}

	// check interval
	if (millis() - currentMillisR > _intervalR || !_created) {
		currentMillisR = millis();

		if (!_httpRequest()) {
			PRINT(connectionFailed);
			return connectionFailed;
		}

		String ser = "";
		if (_port == 443)
			ser = "https://www.iocontrol.ru";

		_client.println(
				(String)"GET " +ser+ "/api/readDataAll/"
				+ _boardName + "/" + _key
				//+ " HTTP/1.0"
				+ " HTTP/1.1"
			       );

		_rest();


		if (int h_status = _httpStatus() != httpOk) {
			PRINT(h_status);
			return h_status;
		}

		if (!_discardHeader()) {
                        _client.stop();
			PRINT(invalidHeader);
			return invalidHeader;
                }

		if (_client.available()) {

			_client.find('{');
			String s = "{" + _client.readStringUntil('[');
			s.concat("\"\"}");

			// parse if response was successful
			// get boardSize
			bool check = false;
			int jsonError = _parseJson(check, s, F("check"));
			int serverError = 0;

			if (jsonError != 0) {
                                _client.stop();
				PRINT(jsonError);
				return jsonError;
			}

			else if (!check) {
				_parseJson(serverError, s, F("message"));

				if (serverError == invalidName)
					_boardExists = false;

                                _client.stop();
				PRINT(serverError);
				return serverError;
			}

			else {
				_parseJson(_boardSize, s, F("countVariable"));
			}


			//create structure if valid board size

			if (_boardSize == 0) {
                                _client.stop();
				PRINT(emptyBoard);
				return emptyBoard;
			}

			else if (!_created) {
				_boardVars = new t_item[_boardSize];
				_created = true;
			}

			//fill intervals

			//if (!_intervalSet) {
				int tmp;
				//int i = 0;

				_parseJson(tmp, s, F("mTimeR"));

				if (tmp > 0) {
					_intervalR = tmp * _deviceCount;
					//i++;
				}

				_parseJson(tmp, s, F("mTimeW"));

				if (tmp > 0) {
					_intervalW = tmp * _deviceCount;
					//i++;
				}
#ifdef __DEBUG__
				Serial.print("current interval R: ");
				Serial.println(_intervalR);
				Serial.print("current interval W: ");
				Serial.println(_intervalW);
#endif

				//if (i == 2)
					//_intervalSet = true;
			//}

			//fill structure
			for (int i = 0; i < _boardSize; i++) {

				jsonError = _fillData(i);

				if (jsonError) {
                                        _client.stop();
					PRINT(jsonError);
					return jsonError;
                                }
			}
		}
		_client.stop();
	}
	else {
		PRINT(intervalError);
		return intervalError;
	}

	return 0;
}

// update all vars to server
int iocontrol::writeUpdate()
{
	bool writeFlag = false;

	// forbid bombarding the server with bogus board names
	if (!_boardExists) {
		PRINT(invalidName);
		return invalidName;
	}

	// check interval
	if (millis() - currentMillisW > _intervalW && _created) {

		currentMillisW = millis();

		// concat the request string
		String reqString = "";

		for (int i = 0; i < _boardSize; i++) {

			if (_boardVars[i]._pending) {

				writeFlag = true;
				reqString += _boardVars[i].name;
				reqString += ":";

				// call _prepData to convert vars to Strings
				reqString += _prepData(i);
				reqString += ",";
			}
		}

		// call sendData() if there is data to send
		if (writeFlag)
			return _sendData(reqString);

		else {
			PRINT(nothingToWrite);
			return nothingToWrite;
		}

	}
	else {
		PRINT(intervalError);
		return intervalError;
	}

}

// convert a _boardVars var to String to form the http request
String iocontrol::_prepData(int& i)
{
	String empty = "";
	switch (_boardVars[i].v_type) {

		case is_int:
			if (_boardVars[i]._nosign)
				return String((unsigned long)_boardVars[i]._int);
			else
				return String(_boardVars[i]._int);

		case is_float:
#ifdef ESP32
			return String(_boardVars[i]._float, (unsigned int)_boardVars[i]._prec);
#else
			return String(_boardVars[i]._float, _boardVars[i]._prec);
#endif

		case is_string:

			if (_boardVars[i]._string
					&& strspn(_boardVars[i]._string, legal)
					== strlen(_boardVars[i]._string))
				return String(_boardVars[i]._string);

			else
				return String(F("illegal_char"));


		default:
			return empty;
	}
}

// send data to the server
int iocontrol::_sendData(String& req)
{

	if (!_httpRequest()) {
		PRINT(connectionFailed);
		return connectionFailed;
	}

#ifdef __DEBUG__
	Serial.println(req);
#endif
	String ser = "";

	if (_port == 443)
		ser = "https://www.iocontrol.ru";

	// request with data
	_client.println(
			(String)"GET " +ser+ "/api/sendDataAll/"
			+ _boardName + "/" + _key + "/"
			//+ req + " HTTP/1.0"
			+ req + " HTTP/1.1"
		       );

	// rest of the request
	_rest();

	// check for success
	if (int h_status = _httpStatus() != httpOk) {
                _client.stop();
		PRINT(h_status);
		return h_status;
	}

	if (!_discardHeader()) {
                _client.stop();
		PRINT(invalidHeader);
		return invalidHeader;
        }

	if (_client.available()) {

		// JSON hocus pocus...
		_client.find('{');
		String s = "{" + _client.readStringUntil('{');
		s.concat("\"\"}");

		bool check = false;
		int jsonError = _parseJson(check, s, F("check"));
		int serverError = 0;


		if (jsonError) {
                        _client.stop();
			PRINT(jsonError);
			return jsonError;
                }

		else if (!check) {
			_parseJson(serverError, s, F("message"));
                        _client.stop();
			PRINT(serverError);
			return serverError;
		}


		check = false;
		s = "{" + _client.readStringUntil('}');
		s.concat("}");

#ifdef __DEBUG__
		Serial.println(s);
#endif

		// check if all vars were written successfully
		for (int i = 0; i < _boardSize; i++) {
			jsonError = _parseJson(check, s, String(_boardVars[i].name));

			if (jsonError) {
                                _client.stop();
				PRINT(jsonError);
				return jsonError;
                        }

#ifdef __DEBUG__
			Serial.print(jsonError);
			Serial.print("\t\t\t");
			Serial.print(check);
			Serial.print("\t\t\t");
			Serial.println(_boardVars[i]._tries);
#endif

			// reset pending for write flag if updated successfully
			if (check)
				_boardVars[i]._pending = false;

			// decrement tries counter
			else if (_boardVars[i]._pending)
				_boardVars[i]._tries--;


			// give up and reset counter if it reached zero
			if (!_boardVars[i]._tries) {
				_boardVars[i]._pending = false;
				_boardVars[i]._tries = MAX_TRIES;
			}
		}
	}
	else {
		PRINT(invalidResponse);
		return invalidResponse;
	}

	return 0;
}

// get integer from memory
long iocontrol::readInt(const String& name)
{
	for (int i = 0; i < _boardSize; i++) {
		if (_boardVars[i].name == name)
			if (_boardVars[i].v_type == is_int)
				return _boardVars[i]._int;
	}
	return 0;
}

// get bool from memory
bool iocontrol::readBool(const String& name)
{
        return !!readInt(name);
}

// get float from memory
float iocontrol::readFloat(const String& name)
{
	for (int i = 0; i < _boardSize; i++) {
		if (_boardVars[i].name == name)
			if (_boardVars[i].v_type == is_float)
				return _boardVars[i]._float;
	}
	return 0;
}

// get C string from memory
char* iocontrol::readCstring(const String& name)
{
	for (int i = 0; i < _boardSize; i++) {
		if (_boardVars[i].name == name)
			if (_boardVars[i].v_type == is_string)
				return _boardVars[i]._string;
	}
	return 0;
}

// get String object from memory
String iocontrol::readString(const String& name)
{
	return String(readCstring(name));
}

// get iocontrol.ru float precision
uint8_t iocontrol::getFloatPrec(const String& varName)
{
	for (int i =0; i < _boardSize; i++) {
		if (_boardVars[i].name == varName) {
			if (_boardVars[i].v_type == is_float) {
				return _boardVars[i]._prec;
			}
		}
	}
	return 0;
}

// connect to server
bool iocontrol::_httpRequest()
{
	_client.stop();

	if (_client.connect(_server, _port)) {
		return true;
	}
	else {
		return false;
	}
}

// fast forward to data
bool iocontrol::_discardHeader()
{
	return _client.find(headerEnd);
}

void iocontrol::_checkHttpStatus(String& status)
{
	rec_check--;

	if (!rec_check) {
		rec_check = MAX_TRIES;
		return;
	}

	if (status == "") {
		status = _client.readStringUntil('\n');
		_checkHttpStatus(status);
	}
}

// read first line of the response and get the request status
int iocontrol::_httpStatus()
{
	String status = _client.readStringUntil('\n');

	if (status == "") {
		_checkHttpStatus(status);
		if (status == "")
			return invalidStatus;
	}
#ifdef __DEBUG__
	Serial.println(status);
#endif
	int startIndex = 0;

	if ((startIndex = status.indexOf("/1.1 ")) > -1)
		startIndex += 5;
	else
		return invalidHeader;

	status = status.substring(startIndex, startIndex + 4);
	status.trim();

	return status.toInt();
}

// parse a boolean from JSON
int iocontrol::_parseJson(bool& ioBool, const String& json, const String& field)
{
	if (json == "")
		return emptyJson;

	String json2 = json;
	json2 += "}";
	int i = json2.lastIndexOf(field);
	int j = -1;
	i += field.length() + 2;
	if (json2[i] == '\"') {
		i++;
		j = json2.indexOf("\"", i);
	}

	if (j == -1)
		j = json2.indexOf(",", i);

	if (j == -1)
		j = json2.indexOf("}", i);

	if (j == -1) {
		//Serial.println(json);
		return failedJsonRoot;
	}

	if (json2.substring(i, j) == "true")
		ioBool = true;
	else
		ioBool = false;

	return 0;
}

// parse an integer from JSON
int iocontrol::_parseJson(int& ioInt, const String& json, const String& field)
{
	if (json == "")
		return emptyJson;

	int i = json.lastIndexOf(field);
	int j = -1;
	i += field.length() + 2;
	if (json[i] == '\"') {
		i++;
		j = json.indexOf("\"", i);
	}

	if (j == -1)
		j = json.indexOf(",", i);

	if (j == -1)
		j = json.indexOf("}", i);

	if (j == -1)
		return failedJsonRoot;

	ioInt = (int)json.substring(i, j).toInt();

	return 0;
}

// parse a long integer from JSON
int iocontrol::_parseJson(long& ioInt, const String& json, const String& field)
{
	if (json == "")
		return emptyJson;

	int i = json.lastIndexOf(field);
	int j = -1;
	i += field.length() + 2;
	if (json[i] == '\"') {
		i++;
		j = json.indexOf("\"", i);
	}

	if (j == -1)
		j = json.indexOf(",", i);

	if (j == -1)
		j = json.indexOf("}", i);

	if (j == -1)
		return failedJsonRoot;

	String tmp = json.substring(i, j);

	ioInt = tmp.toInt();

	return 0;
}

// parse a float from JSON
int iocontrol::_parseJson(float& ioFloat, const String& json, const String& field, uint8_t& prec)
{
	if (json == "")
		return emptyJson;

	int i = json.lastIndexOf(field);
	int j = -1;
	i += field.length() + 2;
	if (json[i] == '\"') {
		i++;
		j = json.indexOf('\"', i);
	}

	if (j == -1)
		j = json.indexOf(',', i);

	if (j == -1)
		j = json.indexOf('}', i);

	if (j == -1)
		return failedJsonRoot;

	String tmp = json.substring(j, i);
	ioFloat = tmp.toFloat();
	i = json.lastIndexOf('.');
	i++;
	uint8_t tmp1 = j - i;
	if (tmp1 < 6 && tmp1 > 0) {
		prec = tmp1;
	}

	return 0;
}

// parse a String object from JSON
int iocontrol::_parseJson(String& ioString, const String& json, const String& field)
{
	if (json == "")
		return emptyJson;

	int i = json.lastIndexOf(field);
	int j = -1;
	i += field.length() + 2;
	if (json[i] == '\"') {
		i++;
		j = json.indexOf("\"", i);
	}

	if (j == -1)
		j = json.indexOf(",", i);

	if (j == -1)
		j = json.indexOf("}", i);

	if (j == -1) {
		return failedJsonRoot;
	}

	ioString = json.substring(i, j);

	return 0;
}

// fill _boardVars struct with data
int iocontrol::_fillData(int& i)
{

	String s = _client.readStringUntil('}');
	s.concat("}");

	String type = "";
	int jsonError = _parseJson(type, s, F("type"));

	if (jsonError)
		return jsonError;

	jsonError = _parseJson(_boardVars[i].name, s, F("variable"));

	if (jsonError)
		return jsonError;

	if (type == F("int")) {

		_boardVars[i].v_type = is_int;
		jsonError = _parseJson(_boardVars[i]._int, s, value);
	}
	else if (type == F("float")) {

		_boardVars[i].v_type = is_float;
		jsonError = _parseJson(_boardVars[i]._float, s, value, _boardVars[i]._prec);
	}
	else if (type == F("string")) {

		_boardVars[i].v_type = is_string;
		String tmp = "";
		jsonError = _parseJson(tmp, s, value);

		// early exit to prevent creating bogus data
		if (jsonError)
			return jsonError;

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

// write unsigned int to the panel
void iocontrol::write(const String& varName, unsigned int var)
{
	for (int i = 0; i < _boardSize; i++) {
		if (_boardVars[i].name == varName
				&& _boardVars[i].v_type == is_int) {
			_boardVars[i]._nosign = true;
		}
	}
	write(varName, long(var));
}

//write unsigned long to the panel
void iocontrol::write(const String& varName, unsigned long var)
{
	for (int i = 0; i < _boardSize; i++) {
		if (_boardVars[i].name == varName
				&& _boardVars[i].v_type == is_int) {
			_boardVars[i]._nosign = true;
		}
	}
	write(varName, long(var));
}

// write long to the panel
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

// write int to the panel
void iocontrol::write(const String& varName, int var)
{
	write(varName, long(var));
}

// write float to the panel
void iocontrol::write(const String& varName, float var)
{
	for (int i = 0; i < _boardSize; i++) {
		if (_boardVars[i].name == varName
				&& _boardVars[i].v_type == is_float) {
			if (abs(abs(_boardVars[i]._float) - abs(var))
					> pow(10, -_boardVars[i]._prec)) {
				_boardVars[i]._float = var;
				_boardVars[i]._pending = true;
			}
		}
	}
}
// write doulbe to the panel
void iocontrol::write(const String& varName, double var)
{
	write(varName, (float)var);
}
// write String to the panel
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

// write bool to the panel
void iocontrol::write(const String& varName, bool var)
{
	write(varName, long(var));
}

// the rest of the http request
void iocontrol::_rest()
{
	_client.println(F("Host: www.iocontrol.ru"));
	_client.println(F("User-Agent: arduino-ethernet"));
	_client.println(F("Connection: close"));
	_client.println();
}

// form the info String
String iocontrol::info()
{
	String s = "";
	s += "Number of variables: ";
	s += String(_boardSize);
	s += "\n";
	for (int i = 0; i < _boardSize; i++) {
		s += "Name: ";
		s += _boardVars[i].name;
		s += ", ";
		s += "type: ";

		switch (_boardVars[i].v_type) {
			case is_int:
				s += "int";
				s += ", ";
				s += "value: ";
				s += String(_boardVars[i]._int);
				s += "\n";
				break;
			case is_float:
				s += "float";
				s += ", ";
				s += "value: ";
#ifdef ESP32
				s += String(_boardVars[i]._float, (unsigned int)_boardVars[i]._prec);
#else
				s += String(_boardVars[i]._float, _boardVars[i]._prec);
#endif
				s += "\n";
				break;
			case is_string:
				s += "string";
				s += ", ";
				s += "value: ";
				s += String(_boardVars[i]._string);
				s += "\n";
				break;
			default:
				break;
		}
	}
	return s;
}

// read 8x8 matrix image from the server
void iocontrol::readMatrix(const String& name, uint8_t*const image)
{
	for (int i = 0; i < _boardSize; i++) {
		if (_boardVars[i].name == name)
			if (_boardVars[i].v_type == is_string) {
				// spaghetti code, hooray!
				_strtoMatrix(_boardVars[i]._string, image);
			}
	}
}

// convert hex string to 8 bytes
void iocontrol::_strtoMatrix(const char* str, uint8_t*const image)
{
	uint8_t j = 0;

	// check if data size is correct
	// (this could violate memory, but shouldn't 'cause
	// this function is exclusively called from the function
	// above, that checks for a valid C string)
	while (str[j] != '\0' || j > 16)
		j++;

	// do nothing if incorrect data size
	if (j == 16) {

		char tmp[2]{0};

		for (int i = 0; i < 16; i++) {

			uint8_t tmp_int = 0;

			if (str[i] > 47 && str[i] < 58)
				tmp_int = str[i] - 48;

			else if (str[i] > 96 && str[i] < 103)
				tmp_int = str[i] - 87;

			if (!(i & 1)) {
				tmp[0] = tmp_int * 0x10;
			}
			else {
				tmp[1] = tmp_int;
				image[i/2] = tmp[0] + tmp[1];
			}
		}
	}
}

// goddamn it, i know i should add size to the parameters...
// user expected to pass 8 byte array.
void iocontrol::writeMatrix(const String& varName, uint8_t*const var)
{
	for (int i = 0; i < _boardSize; i++) {
		if (_boardVars[i].name == varName
				&& _boardVars[i].v_type == is_string) {

			String tmp = "";

			for (int i = 0; i < 8; i++) {

				if (var[i] < 0x10)
					tmp += "0" + String(var[i], HEX);

				else
					tmp += String(var[i], HEX);
			}

			if (String(_boardVars[i]._string) != tmp && tmp.length() == 16) {

				if (_boardVars[i]._string)
					delete[] _boardVars[i]._string;

				_boardVars[i]._string = new char[tmp.length() + 1];
				tmp.toCharArray(_boardVars[i]._string,
						tmp.length() + 1);

				_boardVars[i]._pending = true;
			}
		}
	}
}

// set amount of devices sharing the same IP to prevent going over API limits
void iocontrol::setDeviceCountOnIP(uint8_t deviceCount)
{
	if (deviceCount > 0)
		_deviceCount = deviceCount;
}

//#ifdef ARDUINO_ARCH_ESP32
void iocontrol::setHttps()
{
	_port = 443;
}
//#endif
#ifdef ESP32
void iocontrol::enableDebug()
{
	_debug_flag = true;
}

void iocontrol::disableDebug()
{
	_debug_flag = false;
}

String status2str(int status)
{
	switch (status) {
		case httpOk:
			return "OK";
		case emptyJson:
			return "Пустой JSON";
		case failedJsonRoot:
			return "Повреждённый JSON";
		case noType:
			return "Неизвестный тип переменной";
		case emptyBoard:
			return "Пустая панель";
		case nothingToWrite:
			return "Данные не изменились";
		case invalidHeader:
			return "Неверный заголовок ответа";
		case intervalError:
			return "Не истёк интервал ожидания";
		case invalidResponse:
			return "Неизвестный ответ сервера";
		case connectionFailed:
			return "Не удалось подключиться к серверу";
		case invalidName:
			return "Панель с таким именем не существует";
		default:
			return String(status);
	}
}
#endif
