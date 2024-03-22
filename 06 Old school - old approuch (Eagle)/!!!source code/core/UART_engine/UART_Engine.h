#ifndef _UART_ENGINE_
   #define _UART_ENGINE_

   #define  KEYHIT_DELAY_9600   250 //115              // in milliseconds "1" for 9600  12 bits (one word: 1 nbit star, 8 bits char, 1 bit parity, 2 bits stop> 1000 ms/ 9600 bit = 0.1042 ms/bit * 12 
   int1 UART_r_buffer_overflowed_STATEMENT;     //W obs³yudze czytania danych funkcja zapamiêtuje czy nie otrzymano wiêcej danych ni¿ d³ugoœc buffora przewidzianego na zapisywnaie otrzymanych danych tj. UART_SlaveBuffer
   unsigned int16 received_data_counter;        //zmienna zapamiêtuje który skolei bit danych otrzymano od slave - jest czyszczona po przetworzeniu danych
   int1  UART_data_received_STATEMENT;          //z mienna zapamiêtuje czy jakiekolwiek dane zosta³y otrzymane ze slav'a. Jeœli tak to nale¿y przeanalizow¹æ dane
   //unsigned int16 UART_kbhit_timeout;
   unsigned int8 UART_kbhit_timeout;
   


      
      
   void uart_read_slave_data(void);
   void uart_isTransmision_timeout(unsigned int8 *);
   //uart_read_slave_data();
   
   void uart_ClearRxParameter(void);      //funkcja czyœci wszystkie zmienne kontroluj¹ce parametry otrzymanej transmisji UART
   void uart_debug_display_data(char *, unsigned int16, unsigned int8);
#endif
