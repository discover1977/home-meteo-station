#include "home_meteo_station.h"

#define SW_SERIAL_DEBUG		1
#define TEST_SRAM			1

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
/* Time */
const char snTxtHour[] PROGMEM = "t_Hour";
const char snTxtMinute[] PROGMEM = "t_Minute";
/* Date */
const char snTxtDate[] PROGMEM = "t_Date";
const char snTxtYear[] PROGMEM = "t_Year";
const char snVarDayOfWeek[] PROGMEM = "var_DoWeek";
const char snVarMonth[] PROGMEM = "var_Month";
/* Sensors */
const char snTxtTempIn[] PROGMEM = "t_tempIn";
const char snTxtHumIn[] PROGMEM = "t_humIn";
const char snTxtCO2In[] PROGMEM = "t_CO2In";
const char snTxtTVOCIn[] PROGMEM = "t_TVOCIn";
const char snTxtTempOut[] PROGMEM = "t_tempOut";
const char snTxtHumOut[] PROGMEM = "t_humOut";
const char snTxtPressOut[] PROGMEM = "t_pressOut";
const char snVarForecastID[] PROGMEM = "v_forecastID";
/* Waveform */
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
	char str[20];
	char NameStr[15];

	memcpy_P(NameStr, snTxtTempIn, sizeof(snTxtTempIn));
	NexText nTxtTempIn(PageHome, 6, NameStr);
	dtostrf(InDoorSensor.Temperature, 4, 1, str);
	nTxtTempIn.setText(str);

	memcpy_P(NameStr, snTxtHumIn, sizeof(snTxtHumIn));
	NexText nTxtHumIn(PageHome, 7, NameStr);
	dtostrf(InDoorSensor.Humidity, 4, 1, str);
	nTxtHumIn.setText(str);

	memcpy_P(NameStr, snTxtCO2In, sizeof(snTxtCO2In));
	NexText nTxtCO2In(PageHome, 9, NameStr);
	sprintf(str, "%4d", InDoorSensor.CO2);
	nTxtCO2In.setText(str);

	memcpy_P(NameStr, snTxtTVOCIn, sizeof(snTxtTVOCIn));
	NexText nTxtTVOCIn(PageHome, 10, NameStr);
	sprintf(str, "%4d", InDoorSensor.TVOC);
	nTxtTVOCIn.setText(str);
}

void nSend_Time() {
	char str[20];
	char NameStr[15];

	memcpy_P(NameStr, snTxtHour, sizeof(snTxtHour));
	NexText nTxtHour(PageHome, 22, NameStr);
	sprintf(str, "%d", lDateTime.Hour);
	nTxtHour.setText(str);

	memcpy_P(NameStr, snTxtMinute, sizeof(snTxtMinute));
	NexText nTxtMinute(PageHome, 1, NameStr);
	sprintf(str, "%02d", lDateTime.Minute);
	nTxtMinute.setText(str);
}

void nSend_Date() {
	char str[20];

	char NameStr[15];

	memcpy_P(NameStr, snTxtYear, sizeof(snTxtYear));
	NexText nTxtYear(PageHome, 2, NameStr);
	sprintf(str, "20%02d", lDateTime.Year);
	nTxtYear.setText(str);

	memcpy_P(NameStr, snTxtDate, sizeof(snTxtDate));
	NexText nTxtDate(PageHome, 3, NameStr);
	sprintf(str, "%d", lDateTime.Date);
	nTxtDate.setText(str);

	memcpy_P(NameStr, snVarMonth, sizeof(snVarMonth));
	NexVariable nVarMonth(PageHome, 21, NameStr);
	nVarMonth.setValue(lDateTime.Month);

	memcpy_P(NameStr, snVarDayOfWeek, sizeof(snVarDayOfWeek));
	NexVariable nVarDayOfWeek(PageHome, 21, NameStr);
	nVarDayOfWeek.setValue(lDateTime.DayOfWeek);
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

void sram_init() {
	for(int i = TEMP_IN_ADDRESS_OFFSET; i < (PRESSURE_OUT_ADDRESS_OFFSET + WF_BUFFER_SIZE); i++) {
		sram.write_byte(i, 0x00);
	}
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

	sram_init();

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

	for(int i = 0; i < 23; i++) {
		tmp = (uint8_t)rand();
		sram_add_plot(TEMP_OUT_ADDRESS_OFFSET, tmp);
		swSerial.print("SRAM write: "); swSerial.print(i); swSerial.print(" / "); swSerial.println(tmp);
	}

	for(int i = 0; i < WF_BUFFER_SIZE; i++) {
		tmp = sram_read_plot(TEMP_OUT_ADDRESS_OFFSET, i);
		swSerial.print("SRAM  read: "); swSerial.print(i); swSerial.print(" / "); swSerial.println(tmp);
	}
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
		//InDoorSensor.CO2 = analogRead(A2);
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
