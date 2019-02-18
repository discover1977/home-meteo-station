#include "home_meteo_station.h"

#define SW_SERIAL_DEBUG		0
#define TEST_SRAM			0

#define CS_PIN				10


#define WF_BUFFER_SIZE					10
#define TEMP_IN_ADDRESS_OFFSET			0
#define CO2_IN_ADDRESS_OFFSET			TEMP_IN_ADDRESS_OFFSET + WF_BUFFER_SIZE
#define TVOC_IN_ADDRESS_OFFSET			CO2_IN_ADDRESS_OFFSET + WF_BUFFER_SIZE

#define TEMP_OUT_ADDRESS_OFFSET			TVOC_IN_ADDRESS_OFFSET + WF_BUFFER_SIZE
#define PRESSURE_OUT_ADDRESS_OFFSET		TEMP_OUT_ADDRESS_OFFSET + WF_BUFFER_SIZE

SoftwareSerial swSerial(8, 7);
DS3231 rtc;
HTU21D htu;
SpiRAM sram(SPI_CLOCK_DIV4, CS_PIN, CHIP_23LC1024);

/* NEXTION ******************************************************************/
enum PageNumber {
	PageHome,
	PageSettings,
	PageSetTime,
	PageSetDate,
	PageInGraph,
	PageOutGraph
};
// Page HOME (id 0)
const char snTxtTempIn[] PROGMEM = "t_tempIn";
const char snTxtHumIn[] PROGMEM = "t_humIn";
const char snTxtCO2In[] PROGMEM = "t_CO2In";
const char snTxtTVOCIn[] PROGMEM = "t_TVOCIn";
const char snTxtTempOut[] PROGMEM = "t_tempOut";
const char snTxtHumOut[] PROGMEM = "t_humOut";
const char snTxtPressOut[] PROGMEM = "t_pressOut";
const char snVarForecastID[] PROGMEM = "v_forecastID";
const char snVarDayOfWeek[] PROGMEM = "var_DoWeek";
const char snVarMonth[] PROGMEM = "var_Month";
const char snWFHomePage[] PROGMEM = "wf_pageHome";

// Page SETTINGS (id 1)

// Page SET TIME (id 2)

// Page SET DATE (id 3)

// Page INDOOR GRAPH (id 4)

// Page OUTDOOR GRAPH (id 5)

/*
 * Register a button object to the touch event list.
 */
NexTouch *nex_listen_list[] = {
    //&b0,
    NULL
};
/****************************************************************** NEXTION */

uint16_t SramAddr[5] = { TEMP_IN_ADDRESS_OFFSET,
							  CO2_IN_ADDRESS_OFFSET,
							  TVOC_IN_ADDRESS_OFFSET,
							  TEMP_OUT_ADDRESS_OFFSET,
							  PRESSURE_OUT_ADDRESS_OFFSET };

struct Flag {
	uint8_t SetTime : 1;
	uint8_t ReadTime : 1;
	uint8_t ReadSensors : 1;
	uint8_t ReadDate : 1;
	uint8_t ForceReading : 1;
} Flag;

struct InDoorSensor {
	float Temperature;
	float Humidity;
	uint16_t CO2;
	uint16_t TVOC;
} InDoorSensor;

struct OutDoorSensor {
	float Temperature;
	float Humidity;
	uint16_t Pressure;
	uint8_t ForecastID;
} OutDoorSensor;

struct lDateTime {
	uint8_t Hour;
	uint8_t Minute;
	uint8_t Second;
	uint8_t Date;
	uint8_t DayOfWeek;
	uint8_t Month;
	uint8_t Year;
} lDateTime;

void nSend_SensorData() {

}

void nSend_Time() {

}

void nSend_Date() {

}

void sram_add_plot(uint16_t laddress_offset, uint8_t value) {
	uint8_t AddrOffsetIndex = 0;
	switch (laddress_offset) {
		case TEMP_IN_ADDRESS_OFFSET: AddrOffsetIndex = 0; break;
		case CO2_IN_ADDRESS_OFFSET: AddrOffsetIndex = 1; break;
		case TVOC_IN_ADDRESS_OFFSET: AddrOffsetIndex = 2; break;
		case TEMP_OUT_ADDRESS_OFFSET: AddrOffsetIndex = 3; break;
		case PRESSURE_OUT_ADDRESS_OFFSET: AddrOffsetIndex = 4; break;
		default: break;
	}

	sram.write_byte(SramAddr[AddrOffsetIndex], value);
	if(++SramAddr[AddrOffsetIndex] == (TEMP_IN_ADDRESS_OFFSET + WF_BUFFER_SIZE)) {
		SramAddr[AddrOffsetIndex] = TEMP_IN_ADDRESS_OFFSET;
	}
}

uint8_t sram_read_plot(uint16_t laddress_offset, uint16_t index) {
	uint8_t RetVal = 0;
	uint8_t AddrOffsetIndex = 0;
	switch (laddress_offset) {
		case TEMP_IN_ADDRESS_OFFSET: AddrOffsetIndex = 0; break;
		case CO2_IN_ADDRESS_OFFSET: AddrOffsetIndex = 1; break;
		case TVOC_IN_ADDRESS_OFFSET: AddrOffsetIndex = 2; break;
		case TEMP_OUT_ADDRESS_OFFSET: AddrOffsetIndex = 3; break;
		case PRESSURE_OUT_ADDRESS_OFFSET: AddrOffsetIndex = 4; break;
		default: break;
	}

	if((SramAddr[AddrOffsetIndex] + index) < (laddress_offset + WF_BUFFER_SIZE)) {
		RetVal = sram.read_byte(SramAddr[AddrOffsetIndex] + index);
	}
	else {
		RetVal = sram.read_byte((SramAddr[AddrOffsetIndex] + index) - WF_BUFFER_SIZE);
	}

	return RetVal;
}

void int0_interrrupt() {
	static uint8_t cnt = 0;

	if(!Flag.SetTime) {
		Flag.ReadTime = true;
	}

	if(++cnt == 1) {
		cnt = 0;
		Flag.ReadSensors = true;
	}
}

//The setup function is called once at startup of the sketch
void setup()
{
#if(SW_SERIAL_DEBUG)
	swSerial.begin(9600);
	swSerial.println("Start prog");
#endif

	sram.enable();

#if(TEST_SRAM && SW_SERIAL_DEBUG)
	uint8_t tmp = 0;
	for(int i = 0; i < 5; i++) {
		tmp = (uint8_t)rand();
		sram_add_plot(TEMP_OUT_ADDRESS_OFFSET, tmp);
		swSerial.print("SRAM write: "); swSerial.print(i); swSerial.print(" / "); swSerial.println(tmp);
	}

	for(int i = 0; i < WF_BUFFER_SIZE; i++) {
		tmp = sram_read_plot(TEMP_OUT_ADDRESS_OFFSET, i);
		swSerial.print("SRAM  read: "); swSerial.print(i); swSerial.print(" / "); swSerial.println(tmp);
	}

	/*for(int i = 0; i < 13; i++) {
		tmp = (uint8_t)rand();
		sram_add_plot(TEMP_OUT_ADDRESS_OFFSET, tmp);
		swSerial.print("SRAM write: "); swSerial.print(i); swSerial.print(" / "); swSerial.println(tmp);
	}

	for(int i = 0; i < WF_BUFFER_SIZE; i++) {
		tmp = sram_read_plot(TEMP_OUT_ADDRESS_OFFSET, i);
		swSerial.print("SRAM  read: "); swSerial.print(i); swSerial.print(" / "); swSerial.println(tmp);
	}*/
#endif

	nexInit();

	pinMode(A0, OUTPUT);
	pinMode(A1, OUTPUT);
	digitalWrite(A0, HIGH);
	digitalWrite(A1, LOW);

	Flag.SetTime = false;
	Flag.ReadTime = true;
	Flag.ReadDate = true;
	Flag.ReadSensors = true;
	Flag.ForceReading = true;

	// Nextion button attach PopCallback function
	//nButtSetTime.attachPop(nSetTimeButt_PopCallback, &nButtSetTime);
	//nButtSetDate.attachPop(nSetDateButt_PopCallback, &nButtSetDate);

	htu.begin();
	rtc.enableOscillator(true, true, 0);

	attachInterrupt(0, int0_interrrupt, RISING);
}

bool h12, PM, Century;

// The loop function is called in an endless loop
void loop()
{
	if(Flag.ReadSensors) {
		InDoorSensor.Temperature = htu.readTemperature();
		InDoorSensor.Humidity = htu.readHumidity();
		InDoorSensor.CO2 = analogRead(A2);
		nSend_SensorData();
		Flag.ReadSensors = false;
	}

	if(Flag.ReadTime) {
		lDateTime.Hour = rtc.getHour(h12, PM);
		lDateTime.Minute = rtc.getMinute();
		lDateTime.Second = rtc.getSecond();
		if(lDateTime.Second == 0) {
			nSend_Time();
		}
		if(Flag.ForceReading) {
			Flag.ForceReading = false;
			nSend_Time();
		}
		if((lDateTime.Hour == 0) && (lDateTime.Minute == 0) && (lDateTime.Second == 0)) {
			Flag.ReadDate = true;
		}
		Flag.ReadTime = false;
	}

	if(Flag.ReadDate) {
		lDateTime.Date = rtc.getDate();
		lDateTime.DayOfWeek = rtc.getDoW();
		lDateTime.Month = rtc.getMonth(Century);
		lDateTime.Year = rtc.getYear();
		nSend_Date();
		Flag.ReadDate = false;
		digitalWrite(LED_BUILTIN, LOW);
	}

	//nexLoop(nex_Listen_List);
}
