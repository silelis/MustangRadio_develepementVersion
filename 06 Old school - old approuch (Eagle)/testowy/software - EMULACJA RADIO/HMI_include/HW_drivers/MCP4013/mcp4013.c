#include <mcp4013.h>

void MCP4013_increment(void)
{
   output_high(MCP4013_UD);
   delay_us(1);      //tLUC delay
   MCP4013_active();
   delay_us(1);      //tLCUF delay
   output_low(MCP4013_UD);
   delay_us(1);      //tLO delay
   output_high(MCP4013_UD);
   delay_us(1);      //tLUC delay
   MCP4013_not_active();
   delay_ms(toggle_freq_delay);
   output_low(MCP4013_UD);
}

void MCP4013_decrement(void)
{
   output_low(MCP4013_UD);
   delay_us(1);      //tLUC delay
   MCP4013_active();
   delay_us(1);      //tLCUR delay
   output_high(MCP4013_UD);
   delay_us(1);      //tHI delay
   delay_us(1);      //tLUC delay
   MCP4013_not_active();
   delay_ms(toggle_freq_delay);
   output_low(MCP4013_UD);
}
