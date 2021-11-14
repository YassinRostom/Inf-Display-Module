//*************************************************************************
// Integrated Autonomous Tester - PROJ325 - Bluetooth.cpp
// 
// Responsible for receiving the test results from the 
// Auto Measuring Module (Master) via Bluetooth serial port protocol module
//
//*************************************************************************

#include "Bluetooth.h"

Serial  HC05_Slave(PA_11,PA_12);

//Function
void Reset_Connection();

//Variables
int     State=0;
int     RxData_Counter = 0;
int     TP_Counter;
int     Check_RxData;

char    Sync;
char    RxBuffer[RX_MAX];

//Low Range Resistance Measurement
float   resistance_lowrange;
int     resistance_lowrange_int;
int     resistance_lowrange_fract;
//Low Range Voltage Measurement
float   voltage_lowrange;    
int     voltage_lowrange_int;
int     voltage_lowrange_fract;
//Current Measurement
float   current;
int     current_int;
int     current_frac;
//High Range Resistance Measurement
float   resistance_highrange;
int     resistance_highrange_int;
int     resistance_highrange_fract;
//High Range Voltage Measurement
float   voltage_highrange;    
int     voltage_highrange_int;
int     voltage_highrange_fract;
//Bespoke Continuity Test
int     Bespoke_State=0;

//Flags Initialization
bool    UPDATE_LCD = false;
bool    ReSync_BT= false;

//TimeOut
Timeout Bluetooth_TimeOut;

//Thread
Thread Bluetooth_thread;

Mutex BluetoothLock;
Mutex DataLock;

//************************************************************************
// Bluetooth_Rx_Data() 
// Receive the test results from the Auto Measuring Module (Master Module)
// Signals the LCD thread 
// Parameters - void
// Return - void
//************************************************************************
void Bluetooth_Rx_Data()
{
    while(1)
    {
        while (HC05_Slave.readable()) 
        {
            //Sync the Bluetooth Modules
            Sync = HC05_Slave.getc();
            pc.printf("Sync=%c\n", Sync);
           
           //Bluetooth Modules in Sync
            while(Sync == 'S')    
            {   
                //Acquire Lock
                BluetoothLock.lock();
                
                //Attach Timer
                Bluetooth_TimeOut.attach(Reset_Connection,5);       
                
                //Rx Buffer Check
                if (RxData_Counter < RX_MAX)        RxBuffer[RxData_Counter] = HC05_Slave.getc();
                else   
                {   
                    pc.printf("Error: Rx buffer overflow -> Some data lost!\r\n");
                    memset(RxBuffer, 0, sizeof(RxBuffer));
                    Sync='0'; 
                    break;
                }//end if
                
                //Increment Counter
                RxData_Counter++;
                
                //Transmission ended
                if (RxBuffer[RxData_Counter-1] == 'E')
                {
                    //Acquire Lock
                    DataLock.lock();

                    RxBuffer[RxData_Counter+1]='\0';
                    
                    //Reset Counter
                    RxData_Counter=0;
                    
                    pc.printf("%s\n\r", RxBuffer);     
                    
                   if (ReSync_BT)
                    {
                        Check_RxData = sscanf(RxBuffer, " ST%d %d %d %d.%d %d.%d %d.%d %d.%d %d.%dE", &TP_Counter, &State, &Bespoke_State,&resistance_lowrange_int, &resistance_lowrange_fract,&voltage_lowrange_int,&voltage_lowrange_fract,&current_int, &current_frac, &resistance_highrange_int, &resistance_highrange_fract,&voltage_highrange_int,&voltage_highrange_fract);                        
                        ReSync_BT = false;
                    }
                    else   Check_RxData = sscanf(RxBuffer, "T%d %d %d %d.%d %d.%d %d.%d %d.%d %d.%dE", &TP_Counter, &State, &Bespoke_State,&resistance_lowrange_int, &resistance_lowrange_fract,&voltage_lowrange_int,&voltage_lowrange_fract,&current_int, &current_frac, &resistance_highrange_int, &resistance_highrange_fract,&voltage_highrange_int,&voltage_highrange_fract);
                    
                    pc.printf("n=%d\n\r",Check_RxData);
                    
                    //Converting the Rx Data 
                    resistance_lowrange=(float)resistance_lowrange_int+((float)resistance_lowrange_fract/100);                    
                    voltage_lowrange=(float)voltage_lowrange_int+((float)voltage_lowrange_fract/1000);
                    current=(float)current_int+((float)current_frac/1000);
                    resistance_highrange=(float)resistance_highrange_int+((float)resistance_highrange_fract/100);
                    voltage_highrange=(float)voltage_highrange_int+((float)voltage_highrange_fract/1000);
                    
                    //Clearing the Rx Buffer
                    memset(RxBuffer, 0, sizeof(RxBuffer));
                    
                    //Start a timer
                    if(TP_Counter==4)    RunTest.start();
                    
                    //Signal LCD
                    UPDATE_LCD= true;
                    
                    //Detach Timer
                    Bluetooth_TimeOut.detach();
                    
                    //Reset Flag
                    Sync = '0';
                    
                    //Release Lock
                    DataLock.unlock();
                    
                }//end if
                
                //Detach timer
                Bluetooth_TimeOut.detach();
                
                //Release Lock
                BluetoothLock.unlock();
            
            }//end while Sync S
        }//end while (HC05_Slave.readable()) 
    }//end while(1)
}//end Bluetooth_Rx_Data

//************************************************************************
// Reset_Connection() 
// Function called by a TimeOut if the Master and Slave Bluetooth Module
// lost synchronization
// Parameters - void
// Return - void
//************************************************************************
void Reset_Connection()
{
    pc.printf("Bluetooth Connection Break!!\n\r");
    
    //Reset flag
    RxData_Counter=0;
    
    //Clear Rx buffer
    memset(RxBuffer, 0, sizeof(RxBuffer));
    
    //Set Flag
    ReSync_BT= true;
    
}//end Reset_Connection