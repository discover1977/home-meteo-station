// Do not remove the include below
#include "home_meteo_station.h"

// TODO проверить ID объектов

HTU21D htu = HTU21D();
/* Nextion объекты *************************************************/

/* Внутренние сенсоры */
NexText nTxtTempIn = NexText(0, 0, "t_tempIn");
NexText nTxtHumIn = NexText(0, 0, "t_humIn");
NexText nTxtCO2In = NexText(0, 0, "t_CO2In");
NexText nTxtTVOCIn = NexText(0, 0, "t_TVOCIn");

/* Наружние сенсоры */
NexText nTxtTempOut = NexText(0, 0, "t_tempOut");
NexText nTxtHumOut = NexText(0, 0, "t_tempOut");
NexText nTxtPressOut = NexText(0, 0, "t_tempOut");
NexVariable nVarForecastID = NexVariable(0, 0, "v_forecastID");

/* График главного экрана */
NexWaveform nWFPageHome = NexWaveform(0, 8, "wf_pageHome");
NexVariable nVarHomePageWFID = NexVariable(0, 0, "nHomePageWFID");	// Идентификатор графика

/* Переменные */
NexVariable nVarHour = NexVariable(2, 9, "var_hour");
NexVariable nVarMinute = NexVariable(2, 10, "var_minute");

/* Дата, время */
NexText nTxtHour = NexText(0, 0, "t_hour");
NexText nTxtMinute = NexText(0, 0, "t_minute");
NexText nTxtYear = NexText(0, 0, "t_year");
NexVariable nTxtMonth = NexVariable(0, 0, "var_monthNum");
NexText nTxtDate = NexText(0, 0, "t_date");
NexVariable nVarWeek = NexVariable(0, 0, "var_weekNum");

NexText nTxtHourPSett = NexText(2, 1, "t_hour_pSett");
NexText nTxtMinutePSett = NexText(2, 3, "t_minute_pSett");

/* Графики */
NexWaveform nWFTempn = NexWaveform(0, 11, "wf_tempIn");
NexWaveform nWFHumIn = NexWaveform(0, 11, "wf_humIn");
NexWaveform nWFCO2In = NexWaveform(0, 11, "wf_CO2In");
NexWaveform nWFTVOCIn = NexWaveform(0, 11, "wf_TVOCIn");
NexWaveform nWFTempOut = NexWaveform(0, 11, "wf_tempOut");
NexWaveform nWFHumOut = NexWaveform(0, 11, "wf_humOut");
NexWaveform nWFPressOut = NexWaveform(0, 11, "wf_pressOut");

/* Кнопки */
NexButton nButtSetTime = NexButton(2, 8, "b_setTime");
NexButton nButtHourInc = NexButton(2, 4, "b_hourInc");
NexButton nButtHourDec = NexButton(2, 6, "b_hourDec");
NexButton nButtMinuteInc = NexButton(2, 5, "b_minuteInc");
NexButton nButtMinuteDec = NexButton(2, 7, "b_minuteDec");


char buffer[10] = {0};		// Буфер для приёма сообщений от Nextion

NexTouch *nex_Listen_List[] =
{
    &nButtSetTime,
	&nButtHourInc,
	&nButtHourDec,
	&nButtMinuteInc,
	&nButtMinuteDec,
    NULL
};

/************************************************* Nextion объекты */

struct Flag {
	uint8_t SetTime: 1;
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
	uint8_t Week;
	uint8_t Hour;
	uint8_t Minute;
	uint8_t Second;
} lDateTime;

void nSetTimeButt_PopCallback(void *ptr)
{

}

void nHourInc_PopCallback(void *ptr)
{
	char str[20];
	uint32_t varValue;
	Flag.SetTime = 1;

	nVarHour.getValue(&varValue);
	lDateTime.Hour = varValue;
	sprintf(str, "%d", lDateTime.Hour);
	nTxtHourPSett.setText(str);
}

void nHourDec_PopCallback(void *ptr)
{
	char str[20];
	uint32_t varValue;
	Flag.SetTime = 1;

	nVarHour.getValue(&varValue);

	lDateTime.Hour = varValue;

	sprintf(str, "%d", lDateTime.Hour);
	nTxtHourPSett.setText(str);
}

void nMinuteInc_PopCallback(void *ptr)
{
	Flag.SetTime = 1;
}

void nMinuteDec_PopCallback(void *ptr)
{
	Flag.SetTime = 1;
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

	nWFPageHome.addValue(0, map(InDoorSensor.Temperature, 10, 35, 0, 70));
}

void nSend_DateTime() {
	char str[20];
#if(DEBUG_DATE_TIME)
	if(++lDateTime.Second == 60) {
		lDateTime.Second = 0;
		if(++lDateTime.Minute == 60) {
			lDateTime.Minute = 0;
			if(++lDateTime.Hour == 24) {
				lDateTime.Hour = 0;
				if(++lDateTime.Week == 7) {
					lDateTime.Week = 0;
				}
			}
		}
	}
#endif
	sprintf(str, "20%02d", lDateTime.Year);
	nTxtYear.setText(str);

	sprintf(str, "%d", lDateTime.Hour);
	nTxtHour.setText(str);

	sprintf(str, "%02d", lDateTime.Minute);
	nTxtMinute.setText(str);

	nTxtMonth.setValue(lDateTime.Month);
	nVarWeek.setValue(lDateTime.Week);
}

//The setup function is called once at startup of the sketch
void setup()
{
	lDateTime.Year = 19;
	lDateTime.Month = 2;
	lDateTime.Date = 4;
	lDateTime.Week = 4;
	lDateTime.Hour = 23;
	lDateTime.Minute = 59;
	lDateTime.Second = 0;

	Flag.SetTime = 0;

	nexInit();

	nButtSetTime.attachPop(nSetTimeButt_PopCallback, &nButtSetTime);
	nButtHourInc.attachPop(nHourInc_PopCallback, &nButtHourInc);
	nButtHourDec.attachPop(nHourDec_PopCallback, &nButtHourDec);
	nButtMinuteInc.attachPop(nMinuteInc_PopCallback, &nButtMinuteInc);
	nButtMinuteDec.attachPop(nMinuteDec_PopCallback, &nButtMinuteDec);

	htu.begin();
}

// The loop function is called in an endless loop
void loop()
{
	InDoorSensor.Temperature = htu.readTemperature();
	InDoorSensor.Humidity = htu.readHumidity();

	// nSend_SensorData();
	// nSend_DateTime();

	nexLoop(nex_Listen_List);

	// delay(500);
}
