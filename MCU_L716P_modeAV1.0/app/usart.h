#ifndef _GLOBAL_H_
#define _GLOBAL_H_


void uart_init(void);
int uart1_send_string(char *str);
void uart0_handle(void);
void uart1_handle(void);

#endif /* _GLOBAL_H_ */
