//*************************************************************************
// Integrated Autonomous Tester - PROJ325 - main.cpp
// 
// Configure the Nucleo F401RE board, Start Bluetooth thread
// Initialize LCD and start LCD threads
// 
//*************************************************************************
#include "mbed.h"
#include "Bluetooth.h"
#include "LCD.h"

int main(void) 
{
    pc.baud(38400);
    HC05_Slave.baud(38400);
    
    pc.printf("\n\r**************Start**************\n\r");    
    
    //Start Threads 
    Bluetooth_thread.start(Bluetooth_Rx_Data);
    //Initialize LCD
    LCD_init();    
    LCD_thread.start(LCD_Display);
    
    while(1)
    {
      }//end while
}//end main
