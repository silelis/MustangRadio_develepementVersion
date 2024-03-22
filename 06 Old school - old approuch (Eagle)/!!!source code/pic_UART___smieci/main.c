#include <main.h>



#define BUFFER_SIZE 251
BYTE buffer[BUFFER_SIZE];
BYTE next_in = 0;
BYTE next_out = 0;
int1 buffer_overflow = FALSE;
int1 int_rda_STATEMENT = FALSE;

//BYTE LAST_next_in=0;

#int_rda
void serial_isr() {
   int t;
   
 /*  if (int_rda_STATEMENT == FALSE)
   {
      next_in = 0;
      next_out = 0;
   }*/
   
   

   buffer[next_in]=fgetc(Hrdw);
   t=next_in;
   next_in=(next_in+1) % BUFFER_SIZE;
   if(next_in==next_out)
   {
     next_in=t;           // Buffer full !!
     buffer_overflow = TRUE;
     
   }
   int_rda_STATEMENT = TRUE;
     
}

#define bkbhit (next_in!=next_out)

BYTE bgetc() {
   BYTE c;

   while(!bkbhit) ;
   c=buffer[next_out];
   next_out=(next_out+1) % BUFFER_SIZE;
   return(c);
}


void main()
{

  // fprintf(Hrdw,"UART_TEST");
  // fprintf(Dbg,"UART_TEST");

   enable_interrupts(int_rda);
   enable_interrupts(global);
   printf("\r\n\Running...\r\n");

               // The program will delay for 10 seconds and then display
               // any data that came in during the 10 second delay

   do {
         //delay_ms(300);
         
         
         
         if (int_rda_STATEMENT == TRUE)
         {
            //disable_interrupts(int_rda);
            
            BYTE TEMP_next_in=next_in;
            
            //LAST_next_in = next_in;
            //fprintf(Dbg, "Check UART communication end\r\n");
            delay_ms(100);
            while(TEMP_next_in!=next_in){
            
               //fprintf(Dbg, "LAST in %d  ",LAST_next_in);
               //fprintf(Dbg, "Nexrt in %d    ", next_in);
               
               TEMP_next_in = next_in;
               delay_ms(100);
               
               //fprintf(Dbg, "UART data in progress \r\n");
            }
         
            

            if(buffer_overflow == TRUE)
            {
            buffer_overflow = FALSE;
            fprintf(Dbg, "!!! UART OVERFLOWED, repeat command \r\n");
            //delay_ms(3000);
            }      
            //else
            //{
            
            fprintf(Dbg, "UART data: ");
            while(bkbhit)
            {
              fputc(bgetc(), Dbg);
            }
            int_rda_STATEMENT = FALSE;
            fputs("",Dbg); 
            //fprintf(Dbg, "\r\n");
            
            
            next_in = 0;
            next_out = 0;
            
            
            
            //enable_interrupts(int_rda);
         }
         
         
         
         /*else
         {
         fprintf(Dbg, "No data   ");
         }*/
      
   } while (TRUE);
     
     
     /*
     
      do {
      //fprintf(Dbg, "\r\nBuffered data => ");
      if(buffer_overflow == TRUE)
      {
      buffer_overflow = FALSE;
      fprintf(Dbg, "\r\nBuffer overflowed, repeat command \r\n");
      //delay_ms(3000);
      }      
      //else
      //{
      while(bkbhit)
        fputc(bgetc(), Dbg);
      //}
        
      }
      
      
      
   } while (TRUE); */
   




}
