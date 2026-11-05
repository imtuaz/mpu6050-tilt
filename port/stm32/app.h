#ifndef APP_H
#define APP_H

void app_init(void);   /* call once after MX_I2C1_Init / MX_USART2_UART_Init */
void app_loop(void);   /* call repeatedly inside while(1) */

#endif /* APP_H */
