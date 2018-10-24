/*
 * send_mail.c
 *
 *  Created on: 05/08/2015
 *      Author: Gustavo
 *
 *
 *      - Comentei as linhas que tentam descobrir a autenticação. Agora autentica direto, iniciando por AUTH LOGIN.
 */



/* ------------------------ FreeRTOS includes ----------------------------- */
#include "FreeRTOS.h"

/* ------------------------ lwIP includes --------------------------------- */
#include "inc/hw_memmap.h"
#include "lwip/api.h"
#include "lwip/tcpip.h"
#include "lwip/tcp.h"
#include "lwip/ip.h"
#include "lwip/memp.h"
#include "lwip/stats.h"
#include "lwip/mem.h"
#include "netif/etharp.h"
#include "lwip/sys.h"
#include "smtp.h"

/* ------------------------ Project includes ------------------------------ */
#include <string.h>
#include <stdio.h>

void UARTPutString(uint32_t ui32Base, char *string);

void send_mail_callback(void *arg, u8_t smtp_result, u16_t srv_err, err_t err)
{
    char pcBuf[384];

    //
    // Convert the IP Address into a string.
    //
    sprintf(pcBuf, "mail (%p) sent with results: 0x%02x, 0x%04x, 0x%08x\n\r", arg, smtp_result, srv_err, err);

    //
    // Display the string.
    //
    UARTPutString(UART0_BASE, pcBuf);
}


struct smtp_send_mail_s {
  const char *from;
  const char* to;
  const char* subject;
  const char* body;
  smtp_result_fn callback_fn;
  void* callback_arg;
  /** If this is != 0, data is *not* copied into an extra buffer
   * but used from the pointers supplied in this struct.
   * This means less memory usage, but data must stay untouched until
   * the callback function is called. */
  u8_t static_data;
}smtp_send_mail_data;

void send_mail_task(void *param)
{
	 //smtp_send_request send_mail_args;
     /* Parameters are not used - suppress compiler error. */
     LWIP_UNUSED_ARG(param);

	 //username: e3e55f2ca78e95544a954ca345bb224b
	 //password: 4853ab24d7cbae7f9e9918e53fd5dbcc
	 //server: in-v3.mailjet.com
	 //Não sei pq, mas o primeiro comanda é rejeitado, então o ehlo virá com erro

	 // first, set the server address
	 smtp_set_server_addr("in-v3.mailjet.com");
	 // set the server port: only necessary if != default (which is 25)
	 smtp_set_server_port(587);
	 // set your authentication data (unless the server does not need this, which is very rare)
	 smtp_set_auth("e3e55f2ca78e95544a954ca345bb224b", "4853ab24d7cbae7f9e9918e53fd5dbcc");

	for(;;){
		 // then, call one of the send functions, like:
		 //smtp_send_mail("gustavo.denardin@gmail.com", "gustavo.denardin@gmail.com", "Assunto teste", "Teste de corpo de email", send_mail_callback, NULL);
		smtp_send_mail_data.from = "gustavo.denardin@gmail.com";
		smtp_send_mail_data.to = "gustavo.denardin@gmail.com";
		smtp_send_mail_data.subject = "E-mail enviado pelo LwIP";
		smtp_send_mail_data.body = "Se voce recebeu este e-mail, ele foi enviado pelo LwIP por Gustavo";
		smtp_send_mail_data.callback_fn = send_mail_callback;
		smtp_send_mail_data.callback_arg = NULL;
		smtp_send_mail_data.static_data = pdTRUE;

		 smtp_send_mail_int((void *)&smtp_send_mail_data);
		 vTaskDelay(60000);
	}
}

