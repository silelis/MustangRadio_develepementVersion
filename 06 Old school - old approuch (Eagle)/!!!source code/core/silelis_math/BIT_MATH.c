#ifndef _BIT_MATH_
      
      #include </BIT_MATH.h>
      
      
      //BIT MATHEMATIC
      //http://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit-in-c-c
      
      /*
      Setting a bit
      
      Use the bitwise OR operator (|) to set a bit.
      
      number |= 1 << x;
      That will set bit x.
      
      Clearing a bit
      
      Use the bitwise AND operator (&) to clear a bit.
      
      number &= ~(1 << x);
      That will clear bit x. You must invert the bit string with the bitwise NOT operator (~), then AND it.
      
      Toggling a bit
      
      The XOR operator (^) can be used to toggle a bit.
      
      number ^= 1 << x;
      That will toggle bit x.
      
      Checking a bit
      
      You didn't ask for this but I might as well add it.
      
      To check a bit, shift the number x to the right, then bitwise AND it:
      
      bit = (number >> x) & 1;
      That will put the value of bit x into the variable bit.
      
      Changing the nth bit to x
      
      Setting the nth bit to either 1 or 0 can be achieved with the following:
      
      number ^= (-x ^ number) & (1 << n);
      */
      
      unsigned int8 nth_bit_to_x(unsigned int8 var_to_change, int8 x, unsigned int8 nth_bit)
      {
      /* zmienia na x konkretny bit*/
      var_to_change = (var_to_change & ~(1<<nth_bit)) | (x<<nth_bit);
      
      return var_to_change;
      }
      
      
      unsigned int8 n_value_bits_to_x(unsigned int8 var_to_change, int8 x, unsigned int8 n_value)
      {
      /* zmienia na x bity daj¹ce wartoœæ n_value eg. 65 zminei bity 0?00000?*/
         // x ON = 1
         // x OFF= 0
         // nth as bitmask for ex. 7th bit 1<<7
         var_to_change^= (-x ^ var_to_change) & n_value;
      return var_to_change;
      }
      
      
      unsigned int8 set_bit(unsigned int8 var_to_change, unsigned int8 nth)
      {
         var_to_change |= 1 << nth;
         return var_to_change;
      }
      
      
      unsigned int8 clear_bit(unsigned int8 var_to_change, unsigned int8 nth)
      {
         var_to_change &= ~(1 << nth);
         return var_to_change;
      }
      
      unsigned int8 toggle_bit(unsigned int8 var_to_change, unsigned int8 nth)
      {
         var_to_change ^= 1 << nth;
         return var_to_change;
      }
      
      int1 check_bit(unsigned int8 var_to_check, unsigned int8 nth)
      {
         int1 bit = (var_to_check >> nth) & 1;
         return bit;
      }
      
      unsigned int MSB_to_LSM(unsigned int num)
         /* function change bit order - i.e. required between PIC and 8051 mcu communication*/
      {
          unsigned int  NO_OF_BITS = sizeof(num) * 8;
          unsigned int reverse_num = 0, i, temp;
       
          for (i = 0; i < NO_OF_BITS; i++)
          {
              temp = (num & (1 << i));
              if(temp)
                  reverse_num |= (1 << ((NO_OF_BITS - 1) - i));
          }
        
          return reverse_num;
      }
      
#endif
