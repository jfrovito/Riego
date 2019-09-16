/* Copyright 2019, Juan Rovito.
 * All rights reserved.
 *
 * This file is part sAPI library for microcontrollers.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * Date: 2019-09-13
 */

/*==================[inclusions]=============================================*/

#include "sapi.h"       	// <= sAPI header
#include "Riego.h"   		// <= own header
#include "Sensors.h"   		//
#include "sim808_ATcmd.h"	//

/*==================[macros and definitions]=================================*/

// UART list:
//  - UART_GPIO or UART_485
//  - UART_USB or UART_ENET
//  - UART_232

#define UART_DEBUG		UART_USB
#define UART_SIM808		UART_232
#define UARTS_BAUD_RATE	115200

#define THINGSPEAK_SERVER_URL		"api.thingspeak.com"
#define THINGSPEAK_SERVER_PORT		80
#define THINGSPEAK_WRITE_API_KEY	"KSB7D4N8SZ73K9JJ"

#define THINGSPEAK_GET_UPDATE		"GET /update?api_key="
#define THINGSPEAK_FIELD_TAG		"&field"

#define THINGSPEAK_MAX_CH_FIELDS	8


#define MIN_SOIL_HUMIDITY			67.5


/*==================[internal data declaration]==============================*/
DEBUG_PRINT_ENABLE
/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

typedef struct{
	   float	MeasValue;
	   char		StringValue[64];
} SensorData_t;

typedef struct{
	   char		Value[64];
	   uint8_t	Field;
} Thingspeak_t;

enum thingspeak_field{
	SYSSTAT_FIELD_NUM = 0,
	PREDMODESTAT_FIELD_NUM,
	AIRTEMP_FIELD_NUM,
	AIRHUM_FIELD_NUM,
	SOILTEMP_FIELD_NUM,
	SOILHUM_FIELD_NUM,
	WATERFLOW_FIELD_NUM,
	SPARE_FIELD_NUM
};

bool_t sysStat;
bool_t predModeStat;
bool_t cmdRiego;
SensorData_t airTemp;
SensorData_t airHum;
SensorData_t soilTemp;
SensorData_t soilHum;
SensorData_t waterFlow;
SensorData_t spare;

Thingspeak_t ThingspeakSensorChannel[8];

char	tcpIpDataToSend[100];
uint8_t	i;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/


/* FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE RESET. */
int main(void){

   /* ------------- INICIALIZACIONES ------------- */

   /* Inicializar la placa */
   boardConfig();

   // Inicializar UART_USB como salida de consola
   debugPrintConfigUart( UART_DEBUG, UARTS_BAUD_RATE );

   // Inicializar SIM808
   if( !sim808Init( UART_SIM808, UART_DEBUG, UARTS_BAUD_RATE ) ){
	  stopProgramError(); // Como dio falso (error) me quedo en un bucle infinito
   }

   // Inicializar AnalogIO
   // Posibles configuraciones: ADC_ENABLE,  ADC_DISABLE
   adcConfig( ADC_ENABLE ); /* ADC */

   // Inicializar DHT11
   dht11Init(GPIO1);


   gpioConfig( GPIO2, GPIO_OUTPUT );


   for( i=0; i<THINGSPEAK_MAX_CH_FIELDS; i++){
	   strcpy(ThingspeakSensorChannel[i].Value, "0");
	   ThingspeakSensorChannel[i].Field = i+1;
    }

   /* ------------- REPETIR POR SIEMPRE ------------- */
   while(TRUE) {

	   // Air Temperature and Humidity Sensors
	   if( dht11Read( &airHum.MeasValue, &airTemp.MeasValue ) ) {

		   gpioWrite( LEDG, ON );
		   gpioWrite( LEDR, OFF );

		   debugPrintString( "Temperatura Aire: ");
		   formatDHT11( airTemp.MeasValue, airTemp.StringValue, 0 );
		   debugPrintString( airTemp.StringValue );
		   debugPrintlnString( " degC." );

		   strcpy(ThingspeakSensorChannel[2].Value, airTemp.StringValue);

		   debugPrintString( "Humedad Aire: ");
		   formatDHT11( airHum.MeasValue, airHum.StringValue, 0 );
		   debugPrintString( airHum.StringValue );
		   debugPrintlnString( " %." );

		   strcpy(ThingspeakSensorChannel[3].Value, airHum.StringValue);

		   debugPrintEnter();

	   }
	   else {
		   gpioWrite( LEDG, OFF );
		   gpioWrite( LEDR, ON );

		   debugPrintlnString( "Error al leer DHT11." );
		   debugPrintEnter();
	   }


       /* Leo la Entrada Analogica AI0 - ADC0 CH1 */
	   soilHum.MeasValue = (float) adcRead( CH1 );

	   debugPrintString( "Humedad Suelo: ");
	   formatDHT11( soilHum.MeasValue, soilHum.StringValue, 0 );
	   debugPrintString( soilHum.StringValue );
	   debugPrintlnString( " %." );

	   strcpy(ThingspeakSensorChannel[5].Value, soilHum.StringValue);

		// Armo el dato a enviar, en este caso para grabar un dato en el canal de Thinspeak
		// Ejemplo: "GET /update?api_key=7E7IOJ276BSDLOBA&field1=66"
		tcpIpDataToSend[0] = 0; // Reseteo la cadena que guarda las otras agregando un caracter NULL al principio
		strcat( tcpIpDataToSend, THINGSPEAK_GET_UPDATE );     // Agrego la peticion de escritura de datos
		strcat( tcpIpDataToSend, THINGSPEAK_WRITE_API_KEY );   // Agrego la clave de escritura del canal

		for( i=0; i<THINGSPEAK_MAX_CH_FIELDS; i++ ){
			strcat( tcpIpDataToSend, THINGSPEAK_FIELD_TAG );       // Agrego field del canal
			strcat( tcpIpDataToSend, intToString(ThingspeakSensorChannel[i].Field) );
			strcat( tcpIpDataToSend, "=" );                        // Agrego el valor a enviar
			strcat( tcpIpDataToSend, ThingspeakSensorChannel[i].Value);//intToString( buffout ) );
		}

		// Envio los datos TCP/IP al Servidor de Thingpeak
		// Ver en: https://thingspeak.com/channels/377497/
		sim808SendTCPUDPDataToServer( THINGSPEAK_SERVER_URL, THINGSPEAK_SERVER_PORT,
								  tcpIpDataToSend, strlen( tcpIpDataToSend ) );

		delay(20000);

		sysStat = ON;
		predModeStat = ON;

		if ( sysStat && predModeStat && soilHum.MeasValue < MIN_SOIL_HUMIDITY) {
			cmdRiego = ON;
		}
		else {
			cmdRiego = OFF;
		}

		gpioWrite( GPIO1, cmdRiego );

   }

   /* NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa no es llamado
      por ningun S.O. */
   return 0 ;
}

/*==================[end of file]============================================*/
