#ifndef __silelis_parser__
   #include </silelis_parser.h>

   int8 parse_received_data(unsigned int *buffer, unsigned int16 buffer_len, rom char *ParsingDataArray)
   // unsigned int *buffer          - wskaznik do zmiennej w PIC , gdzie zapisywane s¹ otrzymane z urz¹dzenia podrzednego (np. OVC lub VMusic3) dane uart, zmianna powinna byæ tego samego typu co comunication_SlaveBuffer z transmission_buffers.h
   // unsigned int16 buffer_len     - zmianna okreœlaj¹ca ile danych PIC odczyta³ z urz¹dzenia podrzednego (np. OVC lub VMusic3), d³ugoœæ dancyh do przeskanowania, zmienna powinna byæ tego samego typu co "received_data_counter" z UART_Engine.h
   // rom char *ParsingDataArray    - sta³a zapisana w rom zawieraj¹ca informacje na temat:
   //                                              -"Wartoœæ enum"          - ENUM stanowi¹cy wartoœæ komenty (len 1) ta wartoœæ jest zwracana przez funkcjê
   //                                              -"D³ugoœæ odpowiedzi"    - unsigned int8 zawieraj¹ca infroamcje na temat d³ugoœci parsowanej frazy (iloœæ kolejnych byt'ów, które zawieraj¹ char'y z parsowan¹ fraz¹
   //                                              -"Odpowiedz z OVC"       - char x (unsigned int8)"D³ugoœæ odpowiedzi" , parsowana fraza jaka powinna byæ wyszukana w buforze.
   //
   //                                              - !!! UWAGA WA¯NE !!!        - ParsingDataArray powinna byæ zakoñczona wartoœci¹ "0"         - !!! UWAGA WA¯NE !!!    
      
   {  
      #define   ParsingDataArray_EnumCommandPosition     0x0                           //modyfikator adresu pierwszej "Wartoœæ enum" w rom char *ParsingDataArray[]
      #define   ParsingDataArray_ParsingWordLen          0x1                           //modyfikator adresu pierwszej "D³ugoœæ odpowiedzi" w rom char *ParsingDataArray[] 
      
      unsigned int8 next_Command_addres =ParsingDataArray_EnumCommandPosition;         //zmienna zapamiêtuj¹ca aktualny adres "Wartoœæ enum" w rom char *ParsingDataArray[]
      unsigned int8 next_Command_len_address =ParsingDataArray_ParsingWordLen;         //zmienna zapamiêtuj¹ca aktualny adres "D³ugoœæ odpowiedzi" w rom char *ParsingDataArray[]
      unsigned int8 next_Response_BeginAddres;                                         //zmienna zapamiêtuj¹ca aktualny adres pocz¹tku "Odpowiedz z OVC" w rom char *ParsingDataArray[]
      
      unsigned int8 ComplianceCounter;                                                 //wyko¿ystywany podczas sprawdzania ile byte'ów danych jest zgodnych pomiêdzy tabel¹ parsowania "rom char *ParsingDataArray", a otrzymanymi danymi w "unsigned int *buffer"
      while(1)
      {     
         next_Response_BeginAddres = next_Command_len_address+1;
         // petla sprawdza czy w otrzymanych danych ("unsigned int *buffer") znajduje siê któraœ z parsowanych komend (zapisanych w "rom char *ParsingDataArray")
         // jeœli siê znajduje to nastêpuje zakoñczenie pracy funkcji i zwrócenie wartoœci enum otrzymanej komendy
         for(unsigned int16 mem_addres =0; mem_addres+ParsingDataArray[next_Command_len_address]<=buffer_len;mem_addres++)
         {
            ComplianceCounter=0;                                              //przy ka¿dej nowej pêtli sprawdzania licznik zgodnoœci powinien byæ wyzerowany
        
            //poni¿sza pêtla for jest moj¹ warjacj¹ na temat memcmp. Normalnie zadzia³aæ powinno memcmp, ale tutaj jest czytanie z sta³êj z ROM, a rom jest 16bit, a char 8 bitory, st¹d "OVC_parse_data[next_Response_BeginAddres+position_counter]&0xFF lub (char)(ParsingDataArray[next_Response_BeginAddres+position_counter])"
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
   
         next_Command_addres = next_Command_addres + ParsingDataArray[next_Command_len_address] + 2;  //pobranie adresy kolejnej komendy w tabeli "rom char *ParsingDataArray". Do tej pory funkcja nie znalaz³a w otrzymanych z urz¹dzenia podrzêdnego (np. OVC lub VMusic3) danych ¿adnej obs³ugiwanej komendy
         if(ParsingDataArray[next_Command_addres]==0)                                                 //je¿eli wartoœæ enum kolejnej komendy to 0, to znaczy, ¿e funkcja dos³a do koñca tabeli parsowania ("rom char *ParsingDataArray") i urz¹dzenie podrzedne (np. OVC lub VMusic3) nie przes³a³o ¿adnej obs³ugiwanej komendy
         {
            return  ParsingDataArray[next_Command_addres];                                            //zwraca waroœæ "0" (tzw. EOT), jako, ¿e nie znaleziono danych ("unsigned int *buffer")  przed³anych przez urz¹dzenie podrzêdne (np. OVC lub VMusic3) ¿adnej pasuj¹cej do "rom char *ParsingDataArray" komendy
         } 
         else                                                                                         //je¿eli wartoœæ enum kolejnej komendy nie jest 0, to znaczy, ¿e funkcja jeszcze nie dzosz³a do koñca "rom char *ParsingDataArray" i nalezy dalej przeszukiwaæ otrzymane z OVC lub VMusic3 dane
         {
            next_Command_len_address = next_Command_addres+1;
         }    
      }
   }
   
   unsigned int16 parse_received_dataForInt16(unsigned int *buffer, unsigned int16 buffer_len)
   {
      boolean first_digit_found = False;                                      //zmienna mowi¹ca o tym, w którym miejscu zacze³a sie liczba i w którym siê skoñczy³a
      unsigned int16 returned_value=0;
      for(unsigned int16 buffer_pos = 0; buffer_pos<buffer_len; buffer_pos++)
      {
         if (*(buffer+buffer_pos)>= 0x30 && *(buffer+buffer_pos)<= 0x39)         //sprawdza czy wartoœæ ASCII w bufore jest z pomiêdzy wartoœci reprezentuj¹cych cyfry
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
