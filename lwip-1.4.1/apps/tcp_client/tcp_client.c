/* ------------------------ FreeRTOS includes ----------------------------- */
#include "FreeRTOS.h"

/* ------------------------ lwIP includes --------------------------------- */
#include "lwip/api.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#include "lwip/tcp.h"
#include "lwip/ip.h"
#include "lwip/memp.h"
#include "lwip/stats.h"

#include "lwip/mem.h"
#include "lwip/udp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/sys.h"

/* ------------------------ Project includes ------------------------------ */
#include <string.h>
#include <stdio.h>

#ifndef SOCK_TARGET_HOST
#define SOCK_TARGET_HOST  "172.20.103.36"
#endif

#ifndef SOCK_TARGET_PORT
#define SOCK_TARGET_PORT  502
#endif



void SocketTCPClient( void *pvParameters )
{
	char message[18];
	int  size;
	int s;
    int ret;
    int cnt = 0;
    struct sockaddr_in addr;

    /* Parameters are not used - suppress compiler error. */
    LWIP_UNUSED_ARG(pvParameters);

	/* set up address to connect to */
    //struct hostent* server_addr = gethostbyname(iot.eclipse.org);

	memset(&addr, 0, sizeof(addr));
	addr.sin_len = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_port = PP_HTONS(SOCK_TARGET_PORT);
	addr.sin_addr.s_addr = inet_addr(SOCK_TARGET_HOST);

	/* create the socket */
	s = lwip_socket(AF_INET, SOCK_STREAM, 0);
	if(s < 0) while(1);

	/* connect */
	do
	{
		ret = lwip_connect(s, (struct sockaddr*)&addr, sizeof(addr));
		if (ret <0)
		{
			(void)lwip_close(s);
			vTaskDelay(1000);
		}
	}while(ret != 0);
	/* should succeed */

    /* Loop forever */
    for( ;; )
    {
    	/* write something */
    	size = sprintf(message, "FreeRTOS Test %d", cnt++);
    	if (cnt > 99999) cnt = 0;
    	ret = lwip_write(s, message, size);
    	if (ret != size)
    	{
    		ret = lwip_close(s);

    		/* connect */
    		do
    		{
        		/* create the socket */
        		s = lwip_socket(AF_INET, SOCK_STREAM, 0);
        		if(s < 0) while(1);

    			ret = lwip_connect(s, (struct sockaddr*)&addr, sizeof(addr));
    			if (ret <0)
    			{
    				(void)lwip_close(s);
    				vTaskDelay(1000);
    			}
    		}while(ret != 0);
    		/* should succeed */
    	}
    	vTaskDelay( 1000 );
    }
}
