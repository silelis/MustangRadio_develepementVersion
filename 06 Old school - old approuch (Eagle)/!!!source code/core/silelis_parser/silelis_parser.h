#ifndef __silelis_parser__
   #define     __silelis_parser__
   int8 parse_received_data(unsigned int *, unsigned int16, rom char * );        //funkcja pobiera wskaznik do buffora z danymi otrzymanymi z OVC lub VMusic3 oraz d�ugo�c tych danych i zwraca (w postaci ENUM) otrzyman� z komend�
   unsigned int16 parse_received_dataForInt16(unsigned int *, unsigned int16);   //funkcja skanuje buffor i sprawdza czy w danym ci�gu znajduja si� jakie� liczby, je�li tak to zwraca waro�� tych liczn jako unsigned int16, a je�li ni eby�o liczb to zwrca 0
#endif
