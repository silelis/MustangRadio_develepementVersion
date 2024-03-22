#ifndef __silelis_math__
   #define __silelis_math__
   
   #include <STDLIB.H>
   #include </BIT_MATH.c>

   unsigned int random(unsigned int min, unsigned int  max)
   {
       unsigned int tmp;
       if (max>=min)
           max-= min;
       else
       {
           tmp= min - max;
           min= max;
           max= tmp;
       }
       return max ? (rand() % max + min) : min;
   }

   unsigned int32 round_closest(float dividend, float divisor)
   {
       return (dividend + (divisor / 2)) / divisor;
   }
      
   
#endif
