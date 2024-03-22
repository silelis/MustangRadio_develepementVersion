#ifndef _UART_ENGINE_
   #define _UART_ENGINE_

   #define  KEYHIT_DELAY_9600   250 //115              // in milliseconds "1" for 9600  12 bits (one word: 1 nbit star, 8 bits char, 1 bit parity, 2 bits stop> 1000 ms/ 9600 bit = 0.1042 ms/bit * 12 
   int1 UART_r_buffer_overflowed_STATEMENT;     //W obs�yudze czytania danych funkcja zapami�tuje czy nie otrzymano wi�cej danych ni� d�ugo�c buffora przewidzianego na zapisywnaie otrzymanych danych tj. UART_SlaveBuffer
   unsigned int16 received_data_counter;        //zmienna zapami�tuje kt�ry skolei bit danych otrzymano od slave - jest czyszczona po przetworzeniu danych
   int1  UART_data_received_STATEMENT;          //z mienna zapami�tuje czy jakiekolwiek dane zosta�y otrzymane ze slav'a. Je�li tak to nale�y przeanalizow�� dane
   //unsigned int16 UART_kbhit_timeout;
   unsigned int8 UART_kbhit_timeout;
   


      
      
   void uart_read_slave_data(void);
   void uart_isTransmision_timeout(unsigned int8 *);
   //uart_read_slave_data();
   
   void uart_ClearRxParameter(void);      //funkcja czy�ci wszystkie zmienne kontroluj�ce parametry otrzymanej transmisji UART
   void uart_debug_display_data(char *, unsigned int16, unsigned int8);
#endif
