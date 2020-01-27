#ifndef __iocontrol_h__
#define __iocontrol_h__

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h>
//typedef char* String;

class iocontrol{

	public:
		iocontrol(const char* boardName);
		iocontrol(const char* boardName, const char* key);
		iocontrol(const char* boardName, uint8_t* mac);
		iocontrol(const char* boardName, const char* key, uint8_t* mac);

		// funcs
		int begin();
		int readUpdate();
		int writeUpdate();

		long readInt(const String& varName);
		float readFloat(const String& varName);
		char* readCstring(const String& varName);
		String readString(const String& varName);
		bool readBool(const String& varName);

		//template <typename T> int write(String varName, T var);

		void write(const String& varName, int var);
		int write(const String& varName, float var);
		int write(const String& varName, float var, uint8_t prec);
		int write(const String& varName, String var);
		int write(const String& varName, bool var);

		//vars


	private:

		// functions
		bool _httpRequest();
		bool _discardHeader();
		int _fillData(int& i);
		int _sendData(int& i);
		int _parseJson(bool& ioBool, String& json, const String& field);
		int _parseJson(int& ioInt, String& json, const String& field);
		int _parseJson(long& ioInt, String& json, const String& field);
		int _parseJson(float& ioFloat, String& json, const String& field);
		int _parseJson(String& ioString, String& json, const String& field);
		int _httpStatus();
		void _rest();
		//JsonObject _parseJsonRoot(String& json, int& error);

		enum type {
			is_int,
			is_float,
			is_string
		};

		// vars
		long currentMillis;
		uint16_t _intervalR = 5000;
		uint16_t _intervalW = 3000;
		struct t_item {
			t_item() {
				_string = 0;
			}
			String name;
			type v_type;
			bool _pending = false;
			union {
				long _int;
				float _float;
                                char* _string;
				//String* _String;
			};
		};

		t_item* _boardVars;
		const char* _boardName;

		int _boardSize = 0;
		int _currentPlace;
		bool _created = false;
		bool _intervalSet = false;
		bool _boardExists = true;
		uint8_t* _mac;
		//bool _pending = false;
		//uint8_t _mac[6];

		const char* _server = "www.iocontrol.ru";
		const char* _key;
		//IPAddress _server();
		//
		EthernetClient _client;

		//byte mac[6] = {
		//        0xFE, 0xED, 0xDE, 0xAD, 0xBE, 0xEF
		//};

};

#endif
