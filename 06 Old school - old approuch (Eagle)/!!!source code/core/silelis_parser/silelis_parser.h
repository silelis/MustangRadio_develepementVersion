#ifndef __silelis_parser__
   #define     __silelis_parser__
   int8 parse_received_data(unsigned int *, unsigned int16, rom char * );        //funkcja pobiera wskaznik do buffora z danymi otrzymanymi z OVC lub VMusic3 oraz d³ugoœc tych danych i zwraca (w postaci ENUM) otrzyman¹ z komendê
   unsigned int16 parse_received_dataForInt16(unsigned int *, unsigned int16);   //funkcja skanuje buffor i sprawdza czy w danym ci¹gu znajduja siê jakieœ liczby, jeœli tak to zwraca waroœæ tych liczn jako unsigned int16, a jeœli ni eby³o liczb to zwrca 0
#endif
