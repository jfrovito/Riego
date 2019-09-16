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

/* Date: 2019-09-13 */

#ifndef _SIM808_ATCMD_H_
#define _SIM808_ATCMD_H_

/*==================[inclusions]=============================================*/

#include "sapi.h"       // <= sAPI header
#include <string.h>   // <= Biblioteca de manejo de Strings, ver:
// https://es.wikipedia.org/wiki/String.h
// http://www.alciro.org/alciro/Programacion-cpp-Builder_12/funciones-cadenas-caracteres-string.h_448.htm

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

#ifndef UART_DEBUG
#define UART_DEBUG					UART_USB
#endif

#ifndef UART_SIM808
#define UART_SIM808					UART_232
#endif

#ifndef UARTS_BAUD_RATE
#define UARTS_BAUD_RATE				115200
#endif

#define SIM808_RX_BUFF_SIZE         1024


/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

bool_t sim808Init( uartMap_t uartForSim808, uartMap_t uartForDebug, uint32_t baudRate );

void sim808CleanRxBuffer( void );

bool_t sim808isAlive( void );

bool_t sim808ReadNetworkRegistration( void );

bool_t sim808ReadNetworkRegistrationStatus( void );

bool_t sim808CloseTCPUDPConnection( void );

bool_t sim808StartTCPUDPConnection( char* url, uint32_t port );

bool_t sim808SendTCPUDPData( char* strData, uint32_t strDataLen );

bool_t sim808SendTCPUDPDataToServer( char* url, uint32_t port, char* strData, uint32_t strDataLen );

void stopProgramError( void );

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* #ifndef _SIM808_ATCMD_H_ */
