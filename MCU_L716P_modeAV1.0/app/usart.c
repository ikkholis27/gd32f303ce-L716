#include <stdio.h>
#include "gd32f30x.h"
#include "usart.h"

#define UART0_BAUDRATE  9600
#define UART1_BAUDRATE  115200

void uart0_init(void)
{
    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);

    /* connect port to USARTx_Tx */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

    /* connect port to USARTx_Rx */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /* USART configure */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, UART0_BAUDRATE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);
    
    /* USART interrupt configuration */
    nvic_irq_enable(USART0_IRQn, 0, 0);

    /* enable USART receive interrupt */
    usart_interrupt_enable(USART0, USART_INT_RBNE);

    /* enable USART transmit interrupt */
    //usart_interrupt_enable(USART0, USART_INT_TBE);
}

/* register USART0 to printf function */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(USART0, (uint8_t)ch);

    return ch;
}

void uart1_init(void)
{
    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART1);

    /* connect port to USARTx_Tx */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

    /* connect port to USARTx_Rx */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_3);

    /* USART configure */
    usart_deinit(USART1);
    usart_baudrate_set(USART1, UART1_BAUDRATE);
    usart_receive_config(USART1, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART1, USART_TRANSMIT_ENABLE);
    usart_enable(USART1);

    /* USART interrupt configuration */
    nvic_irq_enable(USART1_IRQn, 0, 0);

    /* enable USART receive interrupt */
    usart_interrupt_enable(USART1, USART_INT_RBNE);

    /* enable USART transmit interrupt */
    //usart_interrupt_enable(USART1, USART_INT_TBE);
}

int uart1_send_string(char *str)
{
    char *p = str;

    while(*p) {
        usart_data_transmit(USART1, *p);
        p++;
    }

    return 0;
}

void uart_init(void)
{
    uart0_init();
    uart1_init();
}

