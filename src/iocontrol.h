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
		//iocontrol(const char* boardName, byte mac[6]);

		// funcs
		int begin();
		int readAll();

		long readInt(String varName);
		float readFloat(String varName);
		char* readCstring(String varName);
		String readString(String varName);
		bool readBool(String varName);

		//template <typename T> int write(String varName, T var);

		int write(String varName, int var);
		int write(String varName, float var);
		int write(String varName, String var);
		int write(String varName, bool var);

		void setKey(String KEY)
		{
			key = KEY;
		}
		//vars

		String key = "0";

	private:

		// functions
		bool _httpRequest();
		bool _discardHeader();
		int _fillData(int& i);
		int _parseJson(bool& ioBool, String& json, String field);
		int _parseJson(int& ioInt, String& json, String field);
		int _parseJson(long& ioInt, String& json, String field);
		int _parseJson(float& ioFloat, String& json, String field);
		int _parseJson(String& ioString, String& json, String field);
		int _httpStatus();
		void _rest();
		JsonObject& _parseJsonRoot(String& json, int& error);

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
		//bool _pending = false;
		//uint8_t _mac[6];

		const char* _server = "www.iocontrol.ru";
		//IPAddress _server();
		//
		EthernetClient _client;

		//byte mac[6] = {
		//        0xFE, 0xED, 0xDE, 0xAD, 0xBE, 0xEF
		//};

};

#endif
