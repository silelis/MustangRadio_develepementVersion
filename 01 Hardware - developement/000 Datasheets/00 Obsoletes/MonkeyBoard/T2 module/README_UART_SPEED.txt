http://www.mikrocontroller.net/topic/315982

Hallo,

nach vielen misslungen Versuchen habe ich nun endlich rausgefunden wie 
man das Board per HTerm und HEX Befehlen zum spielen bekommt :-)

Porteinstellungen:
Baud: 57600 Data: 8 Stop: 1 Party: None

Dann sieht der Befehl für das Abspielen der UKW Frequenz 104,4Mhz wie 
folgt aus:

FE 01 00 01 00 05 01 00 01 97 D0 FD

Aufschlüsslung:

FE = Start-Byte
01 = Command type
00 = Command id
01 = Serial Number
00 = Length Byte 4 (MSB)
05 = Length Byte 5 (LSB)
01 = Stream mode = FM
00 = Frequenz in Hex MSB
01 = Frequenz in Hex
97 = Frequenz in Hex
D0 = Freqzenz in Hex LSB
FD = Stop-Byte

Das Modul antwortet dann:

FE 00 01 01 00 00 FD

Diese Antwort heißt "Command suceeds"

Sollte das Modul mit

FE 00 02 01 00 01 00 FD

antworten, bedeutet das "Command fails"

Falls vom Modul gar keine Antwort kommt, stimmt die Anzahl der 
Datenbytes nicht, welche über die beiden "Length Bytes" eingestellt wird 
nicht.

Hier gibt es jetzt zwei Fallen, es muss immer die Length eingestellt 
werden die in der Tabelle steht, und diese Byte müssen auch gesendet 
werden!

Und genau in der Lenght-Spalte liegt in der Excel-Tabelle eine böse 
Falle, hier steht nur das LSB drin, das MSB (h00) steht nicht drin! Muss 
allerdings zwingend gesendet werden, sonst macht das Modul nichts!

Hoffe es ist verständlich was ich meine.

Grüße Holger

Markierten Text zitieren Antwort



/******************************************************************************
 * Function:        void InitializeUSART(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine initializes the UART to 19200
 *
 * Note:            
 *
 *****************************************************************************/
void InitializeUSART(void)
{
    #if defined(__18CXX)
	    unsigned char c;
        #if defined(__18F14K50)
    	    //ANSELHbits.ANS11 = 0;	// Make RB5 digital so USART can use pin for Rx
            ANSELH = 0;
            #ifndef BAUDCON
                #define BAUDCON BAUDCTL
            #endif
        #endif
        UART_TRISRx=1;				// RX
        UART_TRISTx=0;				// TX
        TXSTA = 0x24;       	// TX enable BRGH=1
        RCSTA = 0x90;       	// Single Character RX
        SPBRG = 0x71;
        SPBRGH = 0x02;      	// 0x0271 for 48MHz -> 19200 baud
        BAUDCON = 0x08;     	// BRG16 = 1
        c = RCREG;				// read 
    #endif

    #if defined(__C30__)
        #if defined( __PIC24FJ256GB110__ ) || defined( PIC24FJ256GB210_PIM )
            // PPS - Configure U2RX - put on pin 49 (RP10)
            RPINR19bits.U2RXR = 10;

            // PPS - Configure U2TX - put on pin 50 (RP17)
            RPOR8bits.RP17R = 5;
        #elif defined(__PIC24FJ64GB004__)
            // PPS - Configure U2RX - put on RC3/pin 36 (RP19)
            RPINR19bits.U2RXR = 19;

            // PPS - Configure U2TX - put on RC9/pin 5 (RP25)
            RPOR12bits.RP25R = 5;
        #elif defined(__PIC24FJ64GB502__)
            // PPS - Configure U2RX - put on RB7 (RP7)
            RPINR19bits.U2RXR = 7;

            // PPS - Configure U2TX - put on RB8 (RP8)
            RPOR4bits.RP8R = 5;
        #elif defined(__PIC24FJ256DA210__)
            // PPS - Configure U2RX - put on RD0 pin 72 (RP11)
            // Make sure jumper JP12 is in 2-4 position if using the 
            // PIC24FJ256DA210 Development Board (microchipDIRECT.com DM240312).
            // Also make sure JP16 and JP17 are in the 1-2 position on this board.
            RPINR19bits.U2RXR = 11; //Assign U2RX to RD0/RP11 pin.  
    
            // PPS - Configure U2TX - put on RF3/pin 51 (RP16)
            RPOR8bits.RP16R = 5;

            TRISFbits.TRISF3 = 0;
        #elif defined(__dsPIC33EP512MU810__) || defined (__PIC24EP512GU810__)
            // The dsPIC33EP512MU810 features Peripheral Pin
            // select. The following statements map UART2 to 
            // device pins which would connect to the the 
            // RX232 transciever on the Explorer 16 board.

             RPINR19 = 0;
             RPINR19 = 0x64;
             RPOR9bits.RP101R = 0x3;

        #else
            #error Verify that any required PPS is done here.
        #endif

        UART2Init();
    #endif

    #if defined(__C32__)
        UART2Init();
    #endif

}//end InitializeUSART

#if defined(__18CXX)
    #define mDataRdyUSART() PIR1bits.RCIF
    #define mTxRdyUSART()   TXSTAbits.TRMT
#elif defined(__C30__) || defined(__C32__)
    #define mDataRdyUSART() UART2IsPressed()
    #define mTxRdyUSART()   U2STAbits.TRMT
#endif
