/* 
create: 17.4.2019
autor: Miriam Nold
PSoC Serial Interfaces
Exercise 4
*/

#include <m8c.h>        // part specific constants and macros
#include "PSoCAPI.h"    // PSoC API definitions for all User Modules
#include <stdlib.h>

//parameter for Analog Temperature
long int stemp = 0;			// the sum of added data that a gave from the EzADC
long int ntemp = 0;			// the number of added data (EzADC
long int valueAnalog = 0;		// the average of the data
int index = 0;				// the number of calculated data

//Paramter for OneWire
long int valueOneWire = 0;		// the average of the data
int readOneWire = 0;
int index2 = 0;

// Parameter for UART
char *strPtr;				// Parameter pointer to read the Input


// function for the lcd output
void lcdOutput(int y,int x,int content){
	char intToString[15];
	itoa(intToString,content,10);
	LCD_Position(y,x);
	LCD_PrString(intToString);
}

// function to clear the lcd
void lcdClean(void){
	LCD_Position(0,0);
	LCD_PrCString("                ");
	LCD_Position(1,0);
	LCD_PrCString("                ");
}


void main(void)
{
	M8C_EnableGInt ; // enable global interrupts 
	  
	PGA_Start(PGA_HIGHPOWER);      	// Turn on the PGA
    LPF2_Start(LPF2_HIGHPOWER);		// Turn on the LPF
	
	Counter8_WritePeriod(0x07);	// set period to eight clocks
    Counter8_WriteCompareValue(0x03);   // generate a 50% duty cycle
    Counter8_EnableInt();               // ensure interrupt is enabled
    Counter8_Start();                   // start the counter!
	
	LCD_Start();               	// Initialize LCD
	OneWire_Start();      		// Initialize 1-Wire pin
	
	UART_CmdReset();                // Initialize receiver/cmd buffer
    UART_IntCntl(UART_ENABLE_RX_INT);   // Enable RX interrupts
	UART_Start(UART_PARITY_NONE);   // Enable UART
	UART_CPutString("\n Welcome\n");


	
	while (1){
		if(UART_bCmdCheck()) {     				// Wait for command from PC  

			if(strPtr = UART_szGetParam()) {		// More than delimiter"
            	UART_CPutString("Found valid command\r\nCommand =>");	// verify Input
				UART_PutString(strPtr);   		// Print out command
				UART_CPutString("\n");

				if ((strPtr[0] == 'a') || (strPtr[0] == 'A')){ 	// if receive "a" or "A"
					EzADC_Stop();				// Stop ADC, so that the LCD output is not over written 
					lcdClean();
					LCD_Position(0,0);          		// Place LCD cursor at row 0, col 0
					LCD_PrCString("NoldPSoClab4");  	// Print "NoldPSoClab4" on the LCD
				}
				if ((strPtr[0] == 's') || (strPtr[0] == 'S')) { // if receive "s" or "S"
					EzADC_Start(EzADC_HIGHPOWER);       	// Apply power to ADC
					lcdClean();				// clean LCD
					
					// to calculate one sample of the analog Value
					EzADC_GetSamples(1);			// to show just 1 Sample (Analog are 2 Samples)
					valueAnalog = EzADC_iGetDataClearFlag(); 
					valueAnalog = (valueAnalog*400)/(4096);
					LCD_Position(0,0);          		// Place LCD cursor at row 0, col 0
					LCD_PrCString("Analog:");  		// Print "Analog:" on the LCD
					lcdOutput(0,8,valueAnalog);		// call the function for LCD output
					lcdOutput(0,13,index++);		// call the function for LCD output

					readOneWire = 2;			// Analog is calculating the average from 50 samples
				}
				if ((strPtr[0] == 'c') || (strPtr[0] == 'c')){
					EzADC_Start(EzADC_HIGHPOWER);       	// Apply power to ADC
					lcdClean();				// clean LCD
					EzADC_GetSamples(0);                	// Have ADC run continuously
				}
				if ((strPtr[0] == 'x') || (strPtr[0] == 'X')){
					EzADC_Stop();				// Stop ADC
					lcdClean();				// clean LCD
					LCD_Position(0,0);			// LCD output
					LCD_PrCString("Stop sampling");C
				}
			} 
			UART_CmdReset();                        		// Reset command buffer
		} 
		
		
		if (ntemp >= 50){				//by 50 data the average will be calculated
			
			valueAnalog = stemp/ntemp;		// calculate average of 50 data
			ntemp = 0;				// set the number and addition to zero
			stemp = 0;
			index++;				// increment the number of calculated data
			
			valueAnalog = (valueAnalog*400)/(4096);	// calculate the temperature in celsius
								// T = (4V * Value)/(4096 * 0,01V)
								//4096 couse of the 12 bit by the ADC
			
			LCD_Position(0,0);          		// Place LCD cursor at row 0, col 0
			LCD_PrCString("Analog:");  		// Print "Analog:" on the LCD
			lcdOutput(0,8,valueAnalog);		// call the function for LCD output
			lcdOutput(0,13,index);			// call the function for LCD output

			readOneWire++;				//increment readOneWire
		}
		
		if( readOneWire == 2 ){
			
			readOneWire = 0;
			index2++;
			
			OneWire_fReset();        		// reset the 1-Wire device  
			OneWire_WriteByte(0xCC); 		// "skip ROM" command  
			OneWire_WriteByte(0xBE); 		// "convert temperature" command  
			
			valueOneWire = (OneWire_bReadByte())/2;  // read temeperature and send to valueOneWire
			
			LCD_Position(1,0);          		// Place LCD cursor at row 1, col 0
			LCD_PrCString("OneWire:");		// Print "OneWire:" on the LCD
			lcdOutput(1,9,(int)valueOneWire);	// call the function for LCD output
			lcdOutput(1,13,index2);			// call the function for LCD output
			
			//evtl noch 750 ms warten
			OneWire_fReset();        		// reset the 1-Wire device  
			OneWire_WriteByte(0xCC); 		// "skip ROM" command  
			OneWire_WriteByte(0x44);		// "read scratchpad" command 
		}
	}
}


//called when ADC conversion is ready
void CalcMean(void){

	ntemp++;
	stemp += EzADC_iGetDataClearFlag();  
	
}
