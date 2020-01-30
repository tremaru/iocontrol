#ifndef __iocontrol_h__
#define __iocontrol_h__

#include <Arduino.h>
#include <Client.h>
//#include <SPI.h>
#ifdef __AVR__
	#include <Ethernet.h>
#endif
#if defined(ESP8266) || defined(ESP32)
	#include <WiFi.h>
#endif
//#include <ArduinoJson.h>
//#include <Client.h>

#define DEFAULT_FLOAT_PRECISION 2
#define DEFAULT_WRITE_INTERVAL 3000
#define DEFAULT_READ_INTERVAL 5000
#define MAX_TRIES 10

class iocontrol{

	public:
		// constructor
		iocontrol(const char* boardName);
		iocontrol(const char* boardName, const char* key);
		iocontrol(const char* boardName, uint8_t* mac);
		iocontrol(const char* boardName, const char* key, uint8_t* mac);

		// funcs
#ifdef __AVR__
		int begin(const EthernetClient& client);
#endif
#if defined(ESP8266) || defined(ESP32)
		int begin(const WiFiClient& client);
#endif
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
		uint8_t* _mac;

		const char* _server = "www.iocontrol.ru";
		const char* _key;

		//Obj
#ifdef __AVR__
		EthernetClient _client;
#endif

#if defined(ESP8266) || defined(ESP32)
		WiFiClient _client;
#endif
};

/*
class EthernetClient{
public:
	EthernetClient() : sockindex(MAX_SOCK_NUM), _timeout(1000) { }
	EthernetClient(uint8_t s) : sockindex(s), _timeout(1000) { }

	uint8_t status();
	virtual int connect(IPAddress ip, uint16_t port);
	virtual int connect(const char *host, uint16_t port);
	virtual int availableForWrite(void);
	virtual size_t write(uint8_t);
	virtual size_t write(const uint8_t *buf, size_t size);
	virtual int available();
	virtual int read();
	virtual int read(uint8_t *buf, size_t size);
	virtual int peek();
	virtual void flush();
	virtual void stop();
	virtual uint8_t connected();
	virtual operator bool() { return sockindex < MAX_SOCK_NUM; }
	virtual bool operator==(const bool value) { return bool() == value; }
	virtual bool operator!=(const bool value) { return bool() != value; }
	virtual bool operator==(const EthernetClient&);
	virtual bool operator!=(const EthernetClient& rhs) { return !this->operator==(rhs); }
	uint8_t getSocketNumber() const { return sockindex; }
	virtual uint16_t localPort();
	virtual IPAddress remoteIP();
	virtual uint16_t remotePort();
	virtual void setConnectionTimeout(uint16_t timeout) { _timeout = timeout; }

	friend class EthernetServer;

	using Print::write;

private:
	uint8_t sockindex; // MAX_SOCK_NUM means client not in use
	uint16_t _timeout;
};
*/
#endif
