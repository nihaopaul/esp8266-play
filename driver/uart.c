#include "c_types.h"
#include "uart.h"

void Uart0_Send_Str(unsigned char *buf)
{
  while(*buf!='\0') {
    uart0_tx_buffer(buf, 1);
    buf++;
  }
}