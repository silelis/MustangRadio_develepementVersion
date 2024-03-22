#ifndef _MCP4013_
   #define _MCP4013_
    
   /*#define  tCSLO 5     //us min
   #define  tCSHI 500   //ms min
   #define  tLUC  500   //ns min
   #define  tLCUF 500   //ns min
   #define  tLCUR 3     //us min
   #define  tHI   500   //ns min
   #define  tLO   500   //ns min*/
   //UP/down toggle freq is max 1MHz so 1 operation should take at least  ms
   #define  toggle_freq_delay  1 //ms
   
   #define MCP4013_not_active()     output_high(MCP4013_CS)
   #define MCP4013_active()         output_low(MCP4013_CS)
   void MCP4013_increment(void);
   void MCP4013_decrement(void);
  
#endif
