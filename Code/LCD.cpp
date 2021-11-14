//************************************************************************/
// Integrated Autonomous Tester - PROJ325 - LCD.cpp
// 
// Responsible for displaying the test results to the user
//
//************************************************************************/

#include "LCD.h"

//LCD Initialization
SPI_TFT_ILI9341 TFT(PB_15, PB_14, PB_13, PB_12, PB_1, PB_2,"TFT"); // mosi, miso, sclk, cs, reset, dc

//Serial USB Connection
Serial  pc(USBTX, USBRX);

//Functions
void LCD_Display();
void LCD_init();

//Inputs
AnalogIn input(A0);

//Variables
float samples[10];
float Sum=0;
float average;
float Average_Voltage;
float BatteryLevel=0.0;

//Flags Initialization
bool LCD_Ready= false;
bool LCD_Clear= false;
bool Display_StopTest= true;
bool Print_StopTest = true;

//Timer
Timer RunTest;

//Thread
Thread LCD_thread;


//************************************************************************
// LCD_init() 
// Responsible for Initializing the TFT LCD 
// Display the Boot up Message
// Parameters - void
// Return - void
//************************************************************************
void LCD_init()
{
    int i;   
    TFT.claim(stdout);          // send stdout to the TFT display
    //TFT.claim(stderr);        // send stderr to the TFT display
    
    // Determine battery level
    for(int i=0; i<10; i++) 
    {
        samples[i] = input.read();
        Sum=Sum+samples[i];
        wait(0.001f);
    }
    
    average= Sum/10;
    Average_Voltage= average*3.3;
    
    BatteryLevel=(Average_Voltage/3)*100;
    
    //Boot up Message
    TFT.set_orientation(3);
    TFT.background(White);    
    TFT.foreground(Red);    
    TFT.cls(); 
    TFT.set_font((unsigned char*) Neu42x35);
    
    TFT.locate(50,5);
    printf("Integrated ");
    TFT.locate(20,40);
    printf("Auto Tester");
    TFT.line(0,80,320,80,Black);      //x1,y1,x2,y2
    TFT.locate(42,100);
    printf("Y Rostom");
    TFT.locate(35,140);
    printf("10563669");
    
    //Display Battery Level
    TFT.locate(75,220);
    TFT.set_font((unsigned char*) Arial12x12);
    printf("Battery Level=%2.0f%%",BatteryLevel);
    
    //Set Flag
    LCD_Clear= true;
    
}//end LCD init

//************************************************************************
// LCD_Display()
// Display the test results to the user
// Parameters - void
// Return - void
//************************************************************************
void LCD_Display()
{
    while (1) 
    {
        //Checks if the (Master) Bluetooth Module have been Turned OFF
        if (RunTest.read_ms() > 15000.0 && Print_StopTest && State!=5)
        {
            //Clear LCD 
            TFT.cls();
                
            TFT.fillrect(0,0,320,90,Yellow);  //x1,y1,x2,y2
            TFT.fillrect(0,150,320,240,Yellow);  //x1,y1,x2,y2   
            
            TFT.background(White);                     
            TFT.foreground(Black);                 
            TFT.set_font((unsigned char*) Neu42x35);
            TFT.locate(10,105);                       
            printf("Test Stopped");
            
            //Set Flag
            LCD_Clear=true;
            
            //Reset Timer
            RunTest.stop();
            RunTest.reset();
            
            //Clear Flag
            Print_StopTest=false;
        }//end if 
        
        //Executed by the Bluetooth_Rx_Data Function
        while(UPDATE_LCD)
        { 
            if(TP_Counter==1)
            {
                //Start Timer
                RunTest.stop();
                RunTest.reset();
                
            }//end if
            
            if(Display_StopTest==false && (State==1 ||State==2 || State==3 || State==4) )
            {
                //Clear LCD
                TFT.cls();
                
                //Set Flags
                Display_StopTest= true;            
                Print_StopTest= true;

            }//end if

            if(Print_StopTest==false && (State==1 ||State==2 || State==3 || State==4) )
            {
                //Clear LCD
                TFT.cls();
                
                //Set Flag     
                Print_StopTest= true;

            }//end if
            
            //Clear LCD
            if (LCD_Clear)
            {
                //Clear LCD
                TFT.cls();
                
                //Reset Flag
                LCD_Clear= false;
                
            }//end if
            
            //LCD Display Settings
            TFT.foreground(Blue);    
            TFT.background(White);           
            TFT.set_font((unsigned char*) Arial28x28);
            
            TFT.locate(0,10);        
            
            //Print the Test Running
            if      (State==1)               printf("Low Range Test   ");
            else if (State==2)               printf("High Range Test  ");  
            else if (State==3)               printf("Continuity Test  ");
            else if (State==4)               printf("B-Continuity Test");
            
            if(State==1 ||State==2 || State==3 || State==4)    TFT.line(0,40,320,40,Black);
            
            //Acquire Lock
            DataLock.lock();
            
            TFT.foreground(Black);
            
            //Print Test Point
            if(TP_Counter == 1 && State != 5 && Check_RxData ==13)
            {
                TFT.locate(0,50);         
                printf("TP 1:");
                TFT.locate(85,50);
            }
            else if (TP_Counter == 2 && State != 5 && Check_RxData ==13)
            {
                TFT.locate(0,100);         
                printf("TP 2:");                        
                TFT.locate(85,100);
            }
            else if (TP_Counter == 3 && State != 5 && Check_RxData ==13)
            {
                TFT.locate(0,150);         
                printf("TP 3:");               
                TFT.locate(85,150);
            }
            else if (TP_Counter == 4 && State != 5 && Check_RxData ==13)
            {
                TFT.locate(0,200);         
                printf("TP 4:");
                TFT.locate(85,200);               
            }
            
            //Release Lock
            DataLock.unlock();
            
            //Print Test Results
            if (State==1 && Check_RxData ==13)
            {
                //Low Range Test 
                
                //Acquire Lock
                DataLock.lock();
                
                if (voltage_lowrange > 0.150f && voltage_lowrange < 0.280f)    
                {
                    TFT.foreground(Yellow);
                    printf("Out of Range ");  
                }
                else if (voltage_lowrange  >= 0.280f)                          
                {
                    TFT.foreground(Red);
                    printf("Open Circuit  ");
                }
                else if (voltage_lowrange < ShortC_Condition)                  
                {
                    TFT.foreground(Red);
                    printf("Short Circuit ");
                }
                else   
                {
                    TFT.foreground(Green);
                    printf("%2.2f Ohm   ", resistance_lowrange);
                }          
                
                //Release Lock          
                DataLock.unlock();
                
            }//end Low Range Test
            
            
            else if (State==2 && Check_RxData ==13)
            {
                 //High Range Test
                //Acquire Lock
                DataLock.lock();
                
                if (voltage_highrange  > 2.3f && voltage_highrange < 3.0f)    
                {
                    TFT.foreground(Yellow);
                    printf("Out of Range ");
                }
                else if (voltage_highrange  >= OpenC_Condition)
                {
                    TFT.foreground(Red);
                    printf("Open Circuit  ");
                }
                else if (voltage_lowrange < ShortC_Condition)
                {
                       TFT.foreground(Red);
                       printf("Short Circuit ");
                }
                else   
                {
                    TFT.foreground(Green);
                    printf("%2.2f Ohm   ", resistance_highrange);
                }
                
                //Release Lock
                DataLock.unlock();
                
            }//end High Range Test
            
            
            else if (State==3 && Check_RxData ==13)
            {
                //Continuity Test
                //Acquire Lock
                DataLock.lock();
                
                //Harness Test
                if (voltage_highrange  >= OpenC_Condition)        
                {
                    TFT.foreground(Red);
                    printf("Open Circuit  ");
                }
                else if (voltage_lowrange < ShortC_Condition)     
                {
                    TFT.foreground(Green);
                    printf("Pass         ");
                }
                else 
                {
                    TFT.foreground(Yellow);
                    if (voltage_lowrange < 0.150f)        printf("Fail %2.2f Ohm", resistance_lowrange);
                    else if (voltage_highrange < 2.3f)    printf("Fail %2.2f Ohm", resistance_highrange);
                    else                                  printf("Fail         "); 
                }                    
            
                //Release Lock
                DataLock.unlock();                    

            }//end Continuity Test
            
            
            else if (State==4 && Check_RxData ==13)
            {
                //Bespoke Continuity Test
                //Acquire Lock
                DataLock.lock();
                
                if (Bespoke_State==6)             
                {
                    TFT.foreground(Black);
                    printf("Measure      ");   
                }
                else if (Bespoke_State==7)
                {
                     TFT.foreground(Green);
                     printf("Pass         "); 
                }
                else if (Bespoke_State==8)
                {
                    TFT.foreground(Yellow);
                    printf("Fail         ");  
                }
                else if (Bespoke_State==9)
                {
                    TFT.foreground(Red);
                    printf("Open Circuit ");
                }
                
                //Release Lock
                DataLock.unlock();
                                   
            }//end Bespoke Continuity Test
            
            else if (State==5 && Display_StopTest)
            {
                //Test Stopeed
    
                //Clear screen
                TFT.cls();
                
                TFT.fillrect(0,0,320,90,Yellow);  //x1,y1,x2,y2
                TFT.fillrect(0,150,320,240,Yellow);  //x1,y1,x2,y2   
                
                TFT.background(White);                     
                TFT.foreground(Black);                 
                TFT.set_font((unsigned char*) Neu42x35);
                TFT.locate(10,105);                       
                printf("Test Stopped");
                
                //Clear flags
                Print_StopTest=false;
                Display_StopTest=false;
                
            }//end Stop
    
            //Reset flag
            UPDATE_LCD = false;
        
        }//end while
    }//end while(1)
}//end void serial_T
