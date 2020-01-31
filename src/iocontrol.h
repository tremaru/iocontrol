#ifndef __iocontrol_h__
#define __iocontrol_h__

#include <Arduino.h>
#include <Client.h>

#define HTTP_OK 200
#define DEFAULT_FLOAT_PRECISION 2
#define DEFAULT_WRITE_INTERVAL 3000
#define DEFAULT_READ_INTERVAL 5000
#define MAX_TRIES 10

class iocontrol{

	public:
		// constructor
		iocontrol(const char* boardName, Client& client);
		iocontrol(const char* boardName, const char* key, Client& client);

		// funcs
		int begin();
		int readUpdate();
		int writeUpdate();

		long readInt(const String& varName);
		float readFloat(const String& varName);
		float readFloat(const String& varName, uint8_t prec);
		char* readCstring(const String& varName);
		String readString(const String& varName);
		bool readBool(const String& varName);

		void write(const String& varName, int var);
		void write(const String& varName, long var);
		void write(const String& varName, float var);
		void write(const String& varName, float var, uint8_t prec);
		void write(const String& varName, String var);
		void write(const String& varName, bool var);

		//vars
		//no public vars. Not yet...

	private:

		// funcs
		bool _httpRequest();
		bool _discardHeader();
		int _fillData(int& i);
		int _sendData(String& data);
		String _prepData(int& i);
		int _parseJson(bool& ioBool, const String& json, const String& field);
		int _parseJson(int& ioInt, const String& json, const String& field);
		int _parseJson(long& ioInt, const String& json, const String& field);
		int _parseJson(float& ioFloat, const String& json, const String& field);
		int _parseJson(String& ioString, const String& json, const String& field);
		int _httpStatus();
		void _rest();

		enum type {
			is_int,
			is_float,
			is_string
		};

		// vars
		long currentMillisR = 0;
		long currentMillisW = 0;
		uint16_t _intervalR = DEFAULT_READ_INTERVAL;
		uint16_t _intervalW = DEFAULT_WRITE_INTERVAL;

		struct t_item {
			t_item() {
				_string = nullptr;
				_prec = DEFAULT_FLOAT_PRECISION;
				_tries = MAX_TRIES;
			}
			String name;
			type v_type;
			uint8_t _prec;
			uint8_t _tries;
			bool _pending = false;
			union {
				long _int;
				float _float;
                                char* _string;
			};
		};

		t_item* _boardVars;
		const char* _boardName;

		int _boardSize = 0;
		int _currentPlace;
		bool _created = false;
		bool _intervalSet = false;
		bool _boardExists = true;

		const char* _server = "www.iocontrol.ru";
		const char* _key;

		//Obj
		Client& _client;
};

#endif
