#ifndef __silelis_parser__
   #include </silelis_parser.h>

   int8 parse_received_data(unsigned int *buffer, unsigned int16 buffer_len, rom char *ParsingDataArray)
   // unsigned int *buffer          - wskaznik do zmiennej w PIC , gdzie zapisywane s� otrzymane z urz�dzenia podrzednego (np. OVC lub VMusic3) dane uart, zmianna powinna by� tego samego typu co comunication_SlaveBuffer z transmission_buffers.h
   // unsigned int16 buffer_len     - zmianna okre�laj�ca ile danych PIC odczyta� z urz�dzenia podrzednego (np. OVC lub VMusic3), d�ugo�� dancyh do przeskanowania, zmienna powinna by� tego samego typu co "received_data_counter" z UART_Engine.h
   // rom char *ParsingDataArray    - sta�a zapisana w rom zawieraj�ca informacje na temat:
   //                                              -"Warto�� enum"          - ENUM stanowi�cy warto�� komenty (len 1) ta warto�� jest zwracana przez funkcj�
   //                                              -"D�ugo�� odpowiedzi"    - unsigned int8 zawieraj�ca infroamcje na temat d�ugo�ci parsowanej frazy (ilo�� kolejnych byt'�w, kt�re zawieraj� char'y z parsowan� fraz�
   //                                              -"Odpowiedz z OVC"       - char x (unsigned int8)"D�ugo�� odpowiedzi" , parsowana fraza jaka powinna by� wyszukana w buforze.
   //
   //                                              - !!! UWAGA WA�NE !!!        - ParsingDataArray powinna by� zako�czona warto�ci� "0"         - !!! UWAGA WA�NE !!!    
      
   {  
      #define   ParsingDataArray_EnumCommandPosition     0x0                           //modyfikator adresu pierwszej "Warto�� enum" w rom char *ParsingDataArray[]
      #define   ParsingDataArray_ParsingWordLen          0x1                           //modyfikator adresu pierwszej "D�ugo�� odpowiedzi" w rom char *ParsingDataArray[] 
      
      unsigned int8 next_Command_addres =ParsingDataArray_EnumCommandPosition;         //zmienna zapami�tuj�ca aktualny adres "Warto�� enum" w rom char *ParsingDataArray[]
      unsigned int8 next_Command_len_address =ParsingDataArray_ParsingWordLen;         //zmienna zapami�tuj�ca aktualny adres "D�ugo�� odpowiedzi" w rom char *ParsingDataArray[]
      unsigned int8 next_Response_BeginAddres;                                         //zmienna zapami�tuj�ca aktualny adres pocz�tku "Odpowiedz z OVC" w rom char *ParsingDataArray[]
      
      unsigned int8 ComplianceCounter;                                                 //wyko�ystywany podczas sprawdzania ile byte'�w danych jest zgodnych pomi�dzy tabel� parsowania "rom char *ParsingDataArray", a otrzymanymi danymi w "unsigned int *buffer"
      while(1)
      {     
         next_Response_BeginAddres = next_Command_len_address+1;
         // petla sprawdza czy w otrzymanych danych ("unsigned int *buffer") znajduje si� kt�ra� z parsowanych komend (zapisanych w "rom char *ParsingDataArray")
         // je�li si� znajduje to nast�puje zako�czenie pracy funkcji i zwr�cenie warto�ci enum otrzymanej komendy
         for(unsigned int16 mem_addres =0; mem_addres+ParsingDataArray[next_Command_len_address]<=buffer_len;mem_addres++)
         {
            ComplianceCounter=0;                                              //przy ka�dej nowej p�tli sprawdzania licznik zgodno�ci powinien by� wyzerowany
        
            //poni�sza p�tla for jest moj� warjacj� na temat memcmp. Normalnie zadzia�a� powinno memcmp, ale tutaj jest czytanie z sta��j z ROM, a rom jest 16bit, a char 8 bitory, st�d "OVC_parse_data[next_Response_BeginAddres+position_counter]&0xFF lub (char)(ParsingDataArray[next_Response_BeginAddres+position_counter])"
            for(unsigned int16 position_counter = 0;  position_counter <ParsingDataArray[next_Command_len_address]; position_counter++)
            {
            
            
               //fprintf(Dbg,"memcmp %c\r\n", (ParsingDataArray[next_Response_BeginAddres+position_counter]&0xFF));
               
               
               if (*(buffer+mem_addres+position_counter)==(ParsingDataArray[next_Response_BeginAddres+position_counter]&0xFF))
               //if (*(buffer+mem_addres+position_counter)==(char)(ParsingDataArray[next_Response_BeginAddres+position_counter]))
               {
                  ComplianceCounter++;
               }
               else
               {
                  break;
               }
            }
            if (ComplianceCounter==ParsingDataArray[next_Command_len_address])
            {
               return  ParsingDataArray[next_Command_addres];
            }
         }
   
         next_Command_addres = next_Command_addres + ParsingDataArray[next_Command_len_address] + 2;  //pobranie adresy kolejnej komendy w tabeli "rom char *ParsingDataArray". Do tej pory funkcja nie znalaz�a w otrzymanych z urz�dzenia podrz�dnego (np. OVC lub VMusic3) danych �adnej obs�ugiwanej komendy
         if(ParsingDataArray[next_Command_addres]==0)                                                 //je�eli warto�� enum kolejnej komendy to 0, to znaczy, �e funkcja dos�a do ko�ca tabeli parsowania ("rom char *ParsingDataArray") i urz�dzenie podrzedne (np. OVC lub VMusic3) nie przes�a�o �adnej obs�ugiwanej komendy
         {
            return  ParsingDataArray[next_Command_addres];                                            //zwraca waro�� "0" (tzw. EOT), jako, �e nie znaleziono danych ("unsigned int *buffer")  przed�anych przez urz�dzenie podrz�dne (np. OVC lub VMusic3) �adnej pasuj�cej do "rom char *ParsingDataArray" komendy
         } 
         else                                                                                         //je�eli warto�� enum kolejnej komendy nie jest 0, to znaczy, �e funkcja jeszcze nie dzosz�a do ko�ca "rom char *ParsingDataArray" i nalezy dalej przeszukiwa� otrzymane z OVC lub VMusic3 dane
         {
            next_Command_len_address = next_Command_addres+1;
         }    
      }
   }
   
   unsigned int16 parse_received_dataForInt16(unsigned int *buffer, unsigned int16 buffer_len)
   {
      boolean first_digit_found = False;                                      //zmienna mowi�ca o tym, w kt�rym miejscu zacze�a sie liczba i w kt�rym si� sko�czy�a
      unsigned int16 returned_value=0;
      for(unsigned int16 buffer_pos = 0; buffer_pos<buffer_len; buffer_pos++)
      {
         if (*(buffer+buffer_pos)>= 0x30 && *(buffer+buffer_pos)<= 0x39)         //sprawdza czy warto�� ASCII w bufore jest z pomi�dzy warto�ci reprezentuj�cych cyfry
         {
            first_digit_found = True;
            returned_value=returned_value*10 +((*(buffer+buffer_pos))&0b00001111);
         }
        else
         {
            if (first_digit_found==True)
            {
               break;
           }
         }
      }
      return returned_value;
   }
#endif
