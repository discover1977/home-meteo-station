// Do not remove the include below
#include "home_meteo_station.h"

// TODO проверить ID объектов

HTU21D htu = HTU21D();

DS3231 rtc = DS3231();

/* Nextion объекты *************************************************/

enum PageNumber {
	PageHome,
	PageSettings,
	PageSetTime,
	PageSetDate
};


/* Внутренние сенсоры */
NexText nTxtTempIn = NexText(0, 6, "t_tempIn");
NexText nTxtHumIn = NexText(0, 7, "t_humIn");
NexText nTxtCO2In = NexText(0, 9, "t_CO2In");
NexText nTxtTVOCIn = NexText(0, 10, "t_TVOCIn");

/* Наружние сенсоры */
NexText nTxtTempOut = NexText(0, 13, "t_tempOut");
NexText nTxtHumOut = NexText(0, 12, "t_humOut");
NexText nTxtPressOut = NexText(0, 11, "t_pressOut");
NexVariable nVarForecastID = NexVariable(0, 0, "v_forecastID");

/* График главного экрана */
NexWaveform nWFPageHome = NexWaveform(0, 8, "wf_pageHome");
NexVariable nVarHomePageWFID = NexVariable(0, 0, "nHomePageWFID");	// Идентификатор графика

/* Дата, время */
NexText nTxtHour = NexText(0, 0, "t_hour");
NexText nTxtMinute = NexText(0, 0, "t_minute");
NexText nTxtYear = NexText(0, 0, "t_year");
NexVariable nTxtMonth = NexVariable(0, 0, "var_monthNum");
NexText nTxtDate = NexText(0, 0, "t_date");
NexVariable nVarWeek = NexVariable(0, 0, "var_weekNum");
NexVariable nVarHour = NexVariable(2, 9, "var_hour");
NexVariable nVarMinute = NexVariable(2, 10, "var_minute");
NexVariable nVarDate = NexVariable(3, 13, "var_date");
NexVariable nVarMonth = NexVariable(3, 14, "var_month");
NexVariable nVarYear = NexVariable(3, 15, "var_year");

NexText nTxtHourPSett = NexText(2, 1, "t_hour_pSett");
NexText nTxtMinutePSett = NexText(2, 3, "t_minute_pSett");

/* Графики */
/*
NexWaveform nWFTempn = NexWaveform(0, 11, "wf_tempIn");
NexWaveform nWFHumIn = NexWaveform(0, 11, "wf_humIn");
NexWaveform nWFCO2In = NexWaveform(0, 11, "wf_CO2In");
NexWaveform nWFTVOCIn = NexWaveform(0, 11, "wf_TVOCIn");
NexWaveform nWFTempOut = NexWaveform(0, 11, "wf_tempOut");
NexWaveform nWFHumOut = NexWaveform(0, 11, "wf_humOut");
NexWaveform nWFPressOut = NexWaveform(0, 11, "wf_pressOut");
*/

/* Кнопки */
/* Время ***************************************************/
NexButton nButtSetTime = NexButton(2, 8, "b_setTime");
NexButton nButtHourInc = NexButton(2, 4, "b_hourInc");
NexButton nButtHourDec = NexButton(2, 6, "b_hourDec");
NexButton nButtMinuteInc = NexButton(2, 5, "b_minuteInc");
NexButton nButtMinuteDec = NexButton(2, 7, "b_minuteDec");
/*************************************************** Время */

/* Дата ***************************************************/
NexButton nButtSetDate = NexButton(3, 8, "b_SetDate");
NexButton nButtDateInc = NexButton(3, 11, "b_DateInc");
NexButton nButtDateDec = NexButton(3, 12, "b_DateDec");
NexButton nButtMonthInc = NexButton(3, 4, "b_MonthInc");
NexButton nButtMonthDec = NexButton(3, 6, "b_MonthDec");
NexButton nButtYearInc = NexButton(3, 5, "b_YearInc");
NexButton nButtYearDec = NexButton(3, 7, "b_YearDec");
/*************************************************** Дата */

NexTouch *nex_Listen_List[] = {
    &nButtSetTime,
	&nButtHourInc,
	&nButtHourDec,
	&nButtMinuteInc,
	&nButtMinuteDec,
	&nButtSetDate,
	&nButtDateInc,
	&nButtDateDec,
	&nButtMonthInc,
	&nButtMonthDec,
	&nButtYearInc,
	&nButtYearDec,
    NULL
};

/************************************************* Nextion объекты */

struct Flag {
	bool SetTime: 1;
	bool ReadSensors: 1;
	bool ForceReading: 1;
	bool ReadTime: 1;
	bool ReadDate: 1;
} Flag;

struct InDoorSensor{
	float Temperature;
	float Humidity;
	uint16_t Pressure;
	uint16_t CO2;
	uint16_t TVOC;
} InDoorSensor;

struct OutDoorSensor{
	float Temperature;
	float Humidity;
	uint16_t Pressure;
	uint8_t ForecastID;
} OutDoorSensor;

struct lDateTime{
	uint16_t Year;
	uint8_t Month;
	uint8_t Date;
	uint8_t DayWeek;
	uint8_t Hour;
	uint8_t Minute;
	uint8_t Second;
} lDateTime;

double mapf(double x, double in_min, double in_max, double out_min, double out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void nSetTimeButt_PopCallback(void *ptr) {
	rtc.setHour(lDateTime.Hour);
	rtc.setMinute(lDateTime.Minute);
	rtc.setSecond(0);
	Flag.SetTime = false;
	Flag.ReadTime = true;
	Flag.ForceReading = true;
	digitalWrite(LED_BUILTIN, LOW);
}

void nHourChange_PopCallback(void *ptr) {
	//char str[20];
	uint32_t varValue;
	Flag.SetTime = true;
	nVarHour.getValue(&varValue);
	lDateTime.Hour = varValue;
	//sprintf(str, "%2d", lDateTime.Hour);
	//nTxtHourPSett.setText(str);
	//digitalWrite(LED_BUILTIN, HIGH);
}

void nMinuteChange_PopCallback(void *ptr) {
	//char str[20];
	uint32_t varValue;
	Flag.SetTime = true;
	nVarMinute.getValue(&varValue);
	lDateTime.Minute = varValue;
	//sprintf(str, "%02d", lDateTime.Minute);
	//nTxtMinutePSett.setText(str);
	digitalWrite(LED_BUILTIN, HIGH);
}

void nSetDateButt_PopCallback(void *ptr) {
	rtc.setDate(lDateTime.Date);
	rtc.setMonth(lDateTime.Month);
	rtc.setYear(lDateTime.Year);
	Flag.ReadDate = true;
	digitalWrite(LED_BUILTIN, LOW);
}

void nDateChange_PopCallback(void *ptr) {
	//char str[20];
	uint32_t varValue;
	nVarDate.getValue(&varValue);
	lDateTime.Date = varValue;
	//sprintf(str, "%2d", lDateTime.Hour);
	//nTxtHourPSett.setText(str);
	digitalWrite(LED_BUILTIN, HIGH);
}

void nMonthChange_PopCallback(void *ptr) {
	//char str[20];
	uint32_t varValue;
	nVarMonth.getValue(&varValue);
	lDateTime.Month = varValue;
	//sprintf(str, "%02d", lDateTime.Minute);
	//nTxtMinutePSett.setText(str);
	digitalWrite(LED_BUILTIN, HIGH);
}

void nYearChange_PopCallback(void *ptr) {
	//char str[20];
	uint32_t varValue;
	nVarYear.getValue(&varValue);
	lDateTime.Year = varValue;
	//sprintf(str, "%02d", lDateTime.Minute);
	//nTxtMinutePSett.setText(str);
	digitalWrite(LED_BUILTIN, HIGH);
}

void nSend_SensorData() {
	char str[20];

	dtostrf(InDoorSensor.Temperature, 4, 1, str);
	nTxtTempIn.setText(str);

	dtostrf(InDoorSensor.Humidity, 4, 1, str);
	nTxtHumIn.setText(str);

	sprintf(str, "%4d", InDoorSensor.CO2);
	nTxtCO2In.setText(str);

	sprintf(str, "%4d", InDoorSensor.TVOC);
	nTxtTVOCIn.setText(str);

	nWFPageHome.addValue(0, (uint8_t)(mapf(InDoorSensor.Temperature, 10, 35, 0, 70)));
}

void nSend_Time() {
	char str[20];

	sprintf(str, "%d", lDateTime.Hour);
	nTxtHour.setText(str);

	sprintf(str, "%02d", lDateTime.Minute);
	nTxtMinute.setText(str);
}

void nSend_Date() {
	char str[20];

	sprintf(str, "20%02d", lDateTime.Year);
	nTxtYear.setText(str);

	sprintf(str, "%d", lDateTime.Date);
	nTxtDate.setText(str);
	nTxtMonth.setValue(lDateTime.Month);
	nVarWeek.setValue(lDateTime.DayWeek);
}

void int0_interrrupt() {
	static uint8_t cnt = 0;

	if(!Flag.SetTime) {
		Flag.ReadTime = true;
	}

	/*if(++cnt == 10) {
		cnt = 0;
		Flag.ReadSensors = true;
	}*/
	Flag.ReadSensors = true;
}

//The setup function is called once at startup of the sketch
void setup()
{
	pinMode(LED_BUILTIN, OUTPUT);
	Flag.SetTime = false;

	Flag.ReadTime = true;
	Flag.ReadDate = true;
	Flag.ReadSensors = true;
	Flag.ForceReading = true;

	nexInit();

	nButtSetTime.attachPop(nSetTimeButt_PopCallback, &nButtSetTime);
	nButtHourInc.attachPop(nHourChange_PopCallback, &nButtHourInc);
	nButtHourDec.attachPop(nHourChange_PopCallback, &nButtHourDec);
	nButtMinuteInc.attachPop(nMinuteChange_PopCallback, &nButtMinuteInc);
	nButtMinuteDec.attachPop(nMinuteChange_PopCallback, &nButtMinuteDec);

	nButtSetDate.attachPop(nSetDateButt_PopCallback, &nButtSetDate);
	nButtDateInc.attachPop(nDateChange_PopCallback, &nButtDateInc);
	nButtDateDec.attachPop(nDateChange_PopCallback, &nButtDateDec);
	nButtMonthInc.attachPop(nMonthChange_PopCallback, &nButtMonthInc);
	nButtMonthDec.attachPop(nMonthChange_PopCallback, &nButtMonthInc);
	nButtYearInc.attachPop(nYearChange_PopCallback, &nButtYearInc);
	nButtYearDec.attachPop(nYearChange_PopCallback, &nButtYearDec);

	htu.begin();
	rtc.enableOscillator(true, true, 0);

	attachInterrupt(0, int0_interrrupt, RISING);
}

char str[40];
bool h12, PM, Century;

// The loop function is called in an endless loop
void loop()
{
	if(Flag.ReadSensors) {
		InDoorSensor.Temperature = htu.readTemperature();
		InDoorSensor.Humidity = htu.readHumidity();
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
		lDateTime.DayWeek = rtc.getDoW();
		lDateTime.Month = rtc.getMonth(Century);
		lDateTime.Year = rtc.getYear();
		nSend_Date();
		Flag.ReadDate = false;
	}

	nexLoop(nex_Listen_List);
}
