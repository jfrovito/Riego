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

#include "sim808_ATcmd.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

CONSOLE_PRINT_ENABLE
DEBUG_PRINT_ENABLE

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

// SIM808 Rx Buffer
char sim808ResponseBuffer[ SIM808_RX_BUFF_SIZE ];
uint32_t sim808ResponseBufferSize = SIM808_RX_BUFF_SIZE;

// UARTs
uartMap_t uartSim808 = UART_232;
uartMap_t uartDebug = UART_USB;

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

void sim808CleanRxBuffer( void ){
   sim808ResponseBufferSize = SIM808_RX_BUFF_SIZE;
   memset( sim808ResponseBuffer, 0, sim808ResponseBufferSize );
}

/*---------------------------------------------------------------------------*/

// AT+CIPSTART="TCP","api.thingspeak.com",80
bool_t sim808SendTCPUDPDataToServer( char* url, uint32_t port, char* strData,
		uint32_t strDataLen ){

   bool_t retVal = FALSE;

   // Enviar dato "data" al servidor "url", puerto "port".
   debugPrintlnString( ">>>> ===========================================================" );
   debugPrintString( ">>>> Enviar dato: \"" );
   debugPrintString( strData );
   debugPrintString( "\"\r\n>>>> al servidor \"" );
   debugPrintString( url );
   debugPrintString( "\", puerto \"" );
   debugPrintInt( port );
   debugPrintlnString( "\"..." );
   debugPrintEnter();


   // AT+CIPCLOSE=1 ------------------------------------------
//   if( !sim808CloseTCPUDPConnection() )
//      return retVal;

   // AT+CIPSTART="TCP","url",port ---------------------------
   if( !sim808StartTCPUDPConnection( url, port ) )
      return retVal;

   // Ejemplo:
   // AT+CIPSEND=47 ------------------------------------------
   // GET /update?api_key=7E7IOJ276BSDLOBA&field1=66 ---------
   retVal = sim808SendTCPUDPData( strData, strDataLen );

   return retVal;
}

/*---------------------------------------------------------------------------*/

// AT+CIPSEND=39
// GET /update?key=7E7IOJ276BSDLOBA&1=69
bool_t sim808SendTCPUDPData( char* strData, uint32_t strDataLen ){

   bool_t retVal = FALSE;

   // "GET /update?key=7E7IOJ276BS\"DL\"OBA&1=69"
   // AT+CIPSEND=strDataLen strData --------------------------

   // Limpiar Buffer (es necesario antes de usar
   // "receiveBytesUntilReceiveStringOrTimeoutBlocking")
   sim808CleanRxBuffer();

   // Envio datos TCP/IP al servidor.
   debugPrintlnString( ">>>> Envio datos TCP/IP al servidor..." );

   debugPrintString( ">>>> AT+CIPSEND=" );
   debugPrintInt( strDataLen + 2); // El mas 2 es del \r\n
   debugPrintString( "\r\n" );

   consolePrintString( "AT+CIPSEND=" );
   consolePrintInt( strDataLen + 2); // El mas 2 es del \r\n
   consolePrintString( "\r\n" );

   // No poner funciones entre el envio de comando y la espera de respuesta
   retVal = receiveBytesUntilReceiveStringOrTimeoutBlocking(
               uartSim808,
			   "> ", 2,
               sim808ResponseBuffer, &sim808ResponseBufferSize,
               100000
            );
   if( retVal ){

      // Imprimo todo lo recibido
      debugPrintString( sim808ResponseBuffer );
      debugPrintlnString( "\r\n" );

      // strData\r\n --------------------------------------------

      // Limpiar Buffer (es necesario antes de usar
      // "receiveBytesUntilReceiveStringOrTimeoutBlocking")
      sim808CleanRxBuffer();

      // Envio los datos TCP/IP ------------------
      consolePrintString( strData );
      consolePrintString( "\r\n" );

      // No poner funciones entre el envio de comando y la espera de respuesta
      retVal = receiveBytesUntilReceiveStringOrTimeoutBlocking(
                  uartSim808,
				  "SEND OK\r\n", 9,
                  sim808ResponseBuffer, &sim808ResponseBufferSize,
                  20000
               );
      if( retVal ){

         // Imprimo todo lo recibido
         debugPrintString( sim808ResponseBuffer );
         debugPrintlnString( "\r\n" );

         // Limpiar Buffer (es necesario antes de usar
         // "receiveBytesUntilReceiveStringOrTimeoutBlocking")
         sim808CleanRxBuffer();

         // No poner funciones entre el envio de comando y la espera de respuesta
         retVal = receiveBytesUntilReceiveStringOrTimeoutBlocking(
                     uartSim808,
                     "CLOSED\r\n", 8,
                     sim808ResponseBuffer, &sim808ResponseBufferSize,
                     100000
                  );

         if( retVal ){

            // DATO RECIBIDOOOOOOOOOOO -----------------

            // Imprimo todo lo recibido
            debugPrintString( sim808ResponseBuffer );
            debugPrintlnString( "\r\n" );

         } else{
            debugPrintlnString( ">>>> Error al enviar los datos TCP/IP, en el envio del string" );
            debugPrintlnString( ">>>> \"strData\", cuando el SIM808 pone el prompt > " );
            debugPrintlnString( ">>>> y no se recibe la respuesta y \"CLOSED\"!!\r\n" );

            // Imprimo todo lo recibido
            debugPrintString( sim808ResponseBuffer );
            debugPrintlnString( "\r\n" );
         }

      } else{
         debugPrintlnString( ">>>> Error al enviar los datos TCP/IP, en el envio del string" );
         debugPrintlnString( ">>>> \"strData\", cuando el SIM808 pone el prompt > " );
         debugPrintlnString( ">>>> y no se recibe \"SEND OK\"!!\r\n" );

         // Imprimo todo lo recibido
         debugPrintString( sim808ResponseBuffer );
         debugPrintlnString( "\r\n" );
      }

   } else{
      debugPrintlnString( ">>>> Error al enviar los datos TCP/IP, en comando" );
      debugPrintlnString( ">>>> \"AT+CIPSEND\"!!\r\n" );
      // Imprimo todo lo recibido
      debugPrintString( sim808ResponseBuffer );
      debugPrintlnString( "\r\n" );
   }
   return retVal;
}

/*---------------------------------------------------------------------------*/

// AT+CIPSTART="TCP","api.thingspeak.com",80
bool_t sim808StartTCPUDPConnection( char* url, uint32_t port ){

   bool_t retVal = FALSE;

   // AT+CIPSTART="TCP","url",port ---------------------------

   // Limpiar Buffer (es necesario antes de usar
   // "receiveBytesUntilReceiveStringOrTimeoutBlocking")
   sim808CleanRxBuffer();

   debugPrintString( ">>>> Conectando al servidor \"" );
   debugPrintString( url );
   debugPrintString( "\", puerto \"" );
   debugPrintInt( port );
   debugPrintlnString( "\"..." );

   debugPrintString( ">>>> AT+CIPSTART=\"TCP\",\"" );
   debugPrintString( url );
   debugPrintString( "\"," );
   debugPrintInt( port );
   debugPrintString( "\r\n" );

   consolePrintString( "AT+CIPSTART=\"TCP\",\"" );
   consolePrintString( url );
   consolePrintString( "\"," );
   consolePrintInt( port );
   consolePrintString( "\r\n" );

   // No poner funciones entre el envio de comando y la espera de respuesta
   retVal = receiveBytesUntilReceiveStringOrTimeoutBlocking(
               uartSim808,
               "CONNECT OK\r\n", 12,
			   sim808ResponseBuffer, &sim808ResponseBufferSize,
               10000
            );
   if( !retVal ){
      debugPrintString( ">>>>    Error: No se puede conectar al servidor: \"" );
      debugPrintlnString( url );
      debugPrintString( "\"," );
      debugPrintInt( port );
      debugPrintlnString( "\"!!\r\n" );
   }
   // Imprimo todo lo recibido
   debugPrintString( sim808ResponseBuffer );
   debugPrintlnString( "\r\n" );
   return retVal;
}

/*---------------------------------------------------------------------------*/

bool_t sim808CloseTCPUDPConnection( void ){

   bool_t retVal = FALSE;

   // AT+CIPCLOSE=1 -----------------------------------------------

   // Limpiar Buffer (es necesario antes de usar
   // "receiveBytesUntilReceiveStringOrTimeoutBlocking")
   sim808CleanRxBuffer();

   // Mostrar información del estado de registro y la tecnología de la celda "AT+CGREG?"
   debugPrintlnString( ">>>> Cerrar coneccion TCP o UDP enviando \"AT+CIPCLOSE=1\"..." );
   consolePrintString( "AT+CIPCLOSE=1\r\n" );
   // No poner funciones entre el envio de comando y la espera de respuesta
   retVal = receiveBytesUntilReceiveStringOrTimeoutBlocking(
               uartSim808,
			   "CLOSE OK\r\n", 10,
               sim808ResponseBuffer, &sim808ResponseBufferSize,
               20000
            );
   if( !retVal ){
      debugPrintlnString( ">>>> Fallo en el comando!!\r\n" );
   }
   // Imprimo todo lo recibido
   debugPrintString( sim808ResponseBuffer );
   debugPrintlnString( "\r\n" );
   return retVal;
}

/*---------------------------------------------------------------------------*/

bool_t sim808isAlive( void ){

   bool_t retVal = FALSE;

   // AT -----------------------------------------------

   // Limpiar Buffer (es necesario antes de usar
   // "receiveBytesUntilReceiveStringOrTimeoutBlocking")
   sim808CleanRxBuffer();

   // Chequear si se encuentra el modulo GSM GPS enviandole "AT"
   debugPrintlnString( ">>>> Chequear si se encuentra el modulo GSM GPS enviando \"AT\"...\r\n" );
   consolePrintString( "AT\r\n" );
   // No poner funciones entre el envio de comando y la espera de respuesta
   retVal = receiveBytesUntilReceiveStringOrTimeoutBlocking(
               uartSim808,
			   "OK\r\n", 4,
               sim808ResponseBuffer, &sim808ResponseBufferSize,
               20000
            );
   if( retVal ){
      debugPrintlnString( ">>>> Modulo SIM808 GSM GPS detectado.\r\n" );
   } else{
      debugPrintlnString( ">>>> Error: Modulo SIM808 GSM GPS No detectado!!\r\n" );
   }
   // Imprimo todo lo recibido
   debugPrintString( sim808ResponseBuffer );
   debugPrintlnString( "\r\n" );
   return retVal;
}

/*---------------------------------------------------------------------------*/

bool_t sim808ReadNetworkRegistration( void ){

   bool_t retVal = FALSE;

   // AT+CREG? -----------------------------------------------

   // Limpiar Buffer (es necesario antes de usar
   // "receiveBytesUntilReceiveStringOrTimeoutBlocking")
   sim808CleanRxBuffer();

   // Mostrar información del estado de registro y la tecnología de la celda "AT+CREG?"
   debugPrintlnString( ">>>> Consultando registro en la red enviando \"AT+CREG?\"..." );
   consolePrintString( "AT+CREG?\r\n" );
   // No poner funciones entre el envio de comando y la espera de respuesta
   retVal = receiveBytesUntilReceiveStringOrTimeoutBlocking(
               uartSim808,
               "OK\r\n", 4,
               sim808ResponseBuffer, &sim808ResponseBufferSize,
               20000
            );
   if( !retVal ){
      debugPrintlnString( ">>>> Error: No se encuentran registrado!!\r\n" );
   }
   // Imprimo todo lo recibido
   debugPrintString( sim808ResponseBuffer );
   debugPrintlnString( "\r\n" );
   return retVal;
}

/*---------------------------------------------------------------------------*/

bool_t sim808ReadNetworkRegistrationStatus( void ){

   bool_t retVal = FALSE;

   // AT+CGREG -----------------------------------------------

   // Limpiar Buffer (es necesario antes de usar
   // "receiveBytesUntilReceiveStringOrTimeoutBlocking")
   sim808CleanRxBuffer();

   // Mostrar información del estado de registro y la tecnología de la celda "AT+CGREG?"
   debugPrintlnString( ">>>> Consultando estado de registro enviando \"AT+CREG?\"..." );
   consolePrintString( "AT+CGREG?\r\n" );
   // No poner funciones entre el envio de comando y la espera de respuesta
   retVal = receiveBytesUntilReceiveStringOrTimeoutBlocking(
               uartSim808,
			   "OK\r\n", 4,
               sim808ResponseBuffer, &sim808ResponseBufferSize,
               20000
            );
   if( !retVal ){
      debugPrintlnString( ">>>> Error: No se encuentran registrado!!\r\n" );
   }
   // Imprimo todo lo recibido
   debugPrintString( sim808ResponseBuffer );
   debugPrintlnString( "\r\n" );
   return retVal;
}

/*---------------------------------------------------------------------------*/

bool_t sim808Init( uartMap_t uartForSim808, uartMap_t uartForDebug, uint32_t baudRate ){

   bool_t retVal = FALSE;

   uartSim808 = uartForSim808;
   uartDebug = uartForDebug;

   // Initialize HW ------------------------------------------

   // Inicializar UART_USB como salida de debug
   debugPrintConfigUart( uartDebug, baudRate );
   debugPrintlnString( ">>>> UART_USB configurada como salida de debug.\r\n" );

   // Inicializr otra UART donde se conecta el SIM808 como salida de consola
   consolePrintConfigUart( uartSim808, baudRate );
   debugPrintlnString( ">>>> UART_SIM808 (donde se conecta el SIM808), configurada como salida de consola.\r\n" );

   // AT -----------------------------------------------------
   retVal = sim808isAlive();

   // AT+CREG? -----------------------------------------------
   retVal = sim808ReadNetworkRegistration();

   // AT+CGREG? -----------------------------------------------
   retVal = sim808ReadNetworkRegistrationStatus();

   return retVal;
}

/*---------------------------------------------------------------------------*/

void stopProgramError( void ){
   // Si hay un error grave me quedo en un bucle infinito
   // en modo bajo consumo
   while( TRUE ){
      sleepUntilNextInterrupt();
   }
}


/*==================[end of file]============================================*/
