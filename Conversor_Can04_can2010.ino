#include <SPI.h>
#include <mcp2515.h>
#include <avr/wdt.h>


//variables
MCP2515 CAN0(10); // Pin10 recive can infos from the car
MCP2515 CAN1(9); // pin9 send info to the dashboard
struct can_frame canMsgRcv;
struct can_frame canMsgSnd;
struct can_frame canMsgSndT; 
struct can_frame canMsgRcvT; 
int id; //canMsgRcv.can_id
int len; //canMsgRcv.can_dlc
int idT; //canMsgRcvT.can_id
int lenT; //canMsgRcvT.can_dlc
unsigned char bnovo; //display brightness
int rpm; //car RPM valeu
unsigned char trip; //TRIP button value
unsigned char botao = 0; //external button to change the dashboard pages

//-----------------------------------
//AREA DE CONFIGURACOES
bool shiftlight = true; //activate the "shiftlight" function, change some graphics color to red 
int rpmmax = 5500; //rpm value to active the shiftlight
bool botaobrilho = false; //to activate the brightness button on the side of the 3008 display
bool botaotela = true; //activate a external button to change the display page
bool debug = false; //debug mode read values on serial.
//-----------------------------------


void setup() {
wdt_enable(WDTO_8S); //watchdog time set to 8 seconds

  SPI.begin();
  CAN0.reset();
  CAN0.setBitrate(CAN_125KBPS,MCP_8MHZ);
  CAN0.setNormalMode();
  CAN1.reset();
  CAN1.setBitrate(CAN_125KBPS,MCP_8MHZ);
  CAN1.setNormalMode(); 

  pinMode(8, INPUT_PULLUP); //external button to change pages pinout.
  
  //while (!Serial);
  if (debug == true) {
  Serial.begin(115200);  
  Serial.println("DEBUG MODE!");
  }
  

  
 rpmmax = rpmmax * 0.0312; //Calculate the rpm HEX value to activate the shiftlight

}

void loop() {
wdt_reset(); //watchdog counting reset 
  
   if( CAN0.readMessage( & canMsgRcv) == MCP2515::ERROR_OK ){
    id = canMsgRcv.can_id;
    len = canMsgRcv.can_dlc;
      if(id == 182 && shiftlight == true) { //rpm value from the 0x0B6 can code
      rpm = canMsgRcv.data[0] ;
    }
  
   }
     if (id == 296 && len == 8)  { //ID 128
        
        canMsgSnd.data[0] = canMsgRcv.data[4];
        canMsgSnd.data[1] = canMsgRcv.data[6];
        canMsgSnd.data[2] = canMsgRcv.data[7];
        canMsgSnd.data[3] = canMsgRcv.data[3]; //handbrake light need adjust on the code
        canMsgSnd.data[4] = canMsgRcv.data[1];
        canMsgSnd.data[5] = canMsgRcv.data[0]; //need to adjuste the code fuel light no working
        canMsgSnd.data[6] = 0x04; //need to use this value to keep the display ON
        canMsgSnd.data[7] = 0x00;
        canMsgSnd.can_id = 0x128;
        canMsgSnd.can_dlc = 8; 
        CAN1.sendMessage(&canMsgSnd);
   
         
     }
     else if (id == 935) { // maintenance 
        canMsgSnd.data[0] = canMsgRcv.data[0]; //display the maintenance key signal
        canMsgSnd.data[1] = canMsgRcv.data[5]; // Value x255 +
        canMsgSnd.data[2] = canMsgRcv.data[6]; // Value x1 = Number of days till maintenance (FF FF if disabled)
        canMsgSnd.data[3] = canMsgRcv.data[3]; // Value x5120 +
        canMsgSnd.data[4] = canMsgRcv.data[4]; // Value x20 = km left till maintenance
        canMsgSnd.can_id = 0x3E7; // new id on can2010
        canMsgSnd.can_dlc = 5;
        CAN1.sendMessage(&canMsgSnd);
        
     }

     
     else if (id == 424 && len == 8) { //ID1A8  speed limiter, cruise control and instant trip values
        canMsgSnd.data[0] = canMsgRcv.data[1]; //speed
        canMsgSnd.data[1] = canMsgRcv.data[2]; //speed
        canMsgSnd.data[2] = canMsgRcv.data[0]; //cruise on pause 0x80, 0x88 on, speed limiter on pause 0x40, 0x48 on
        canMsgSnd.data[3] = 0x80; 
        canMsgSnd.data[4] = 0x00; 
        canMsgSnd.data[5] = 0x00; 
        canMsgSnd.data[6] = 0x00;
        canMsgSnd.data[7] = 0x00;
        canMsgSnd.can_id = 0x228; //new ID
        canMsgSnd.can_dlc = 8;
      CAN1.sendMessage(&canMsgSnd);
        canMsgSnd.data[0] = 0x00; 
        canMsgSnd.data[1] = 0x00; 
        canMsgSnd.data[2] = 0x00;
        canMsgSnd.data[3] = 0x00; 
        canMsgSnd.data[4] = 0x00; 
        canMsgSnd.data[5] = canMsgRcv.data[5]; 
        canMsgSnd.data[6] = canMsgRcv.data[6];
        canMsgSnd.data[7] = canMsgRcv.data[7];
        canMsgSnd.can_id = 0x1A8; //instant trip
        canMsgSnd.can_dlc = 8;
        CAN1.sendMessage(&canMsgSnd);
      
     }
       else if (id == 360 && len == 8) { //ID168 other dashboard lights
        canMsgSnd.data[0] = canMsgRcv.data[0]; 
        canMsgSnd.data[1] = canMsgRcv.data[1]; 
        canMsgSnd.data[2] = canMsgRcv.data[5];
        canMsgSnd.data[3] = canMsgRcv.data[3]; 
        canMsgSnd.data[4] = canMsgRcv.data[5];// need adjust
        canMsgSnd.data[5] = canMsgRcv.data[5];// need adjust
        canMsgSnd.data[6] = canMsgRcv.data[6];
        canMsgSnd.data[7] = canMsgRcv.data[7];
        canMsgSnd.can_id = 0x168;   
        canMsgSnd.can_dlc = 8;
      CAN1.sendMessage(&canMsgSnd);
     }
     else if (id == 608 && len == 8){ //language and units
        canMsgSnd.data[0] = 0xA1; //this HEX is for PT-BR, need change to another language
        canMsgSnd.data[1] = 0x1C; //this HEX is for C e KM/L, need change to use l/100km ou F 
        canMsgSnd.data[2] = 0x97;
        canMsgSnd.data[3] = 0x9B;
        canMsgSnd.data[4] = 0xE0;
        canMsgSnd.data[5] = 0xD0;
        canMsgSnd.data[6] = 0x00;
        canMsgSnd.can_id = 0x260;
        canMsgSnd.can_dlc = 7;
        CAN1.sendMessage( & canMsgSnd);      
     }
     
     else if (id == 54) { //ID036 
                      
         if (botaobrilho == true && (canMsgRcv.data[3] & 0xFF) >= 32) { //use the external button to adjust the brightness
        canMsgSnd.data[3] = (bnovo & 0xFF);  
          }
        else { //use the radio configuration to adjust the brightness
        canMsgSnd.data[3] = canMsgRcv.data[3];
          }
     
        canMsgSnd.data[0] = canMsgRcv.data[0]; //86 key turned off, 8E key ON
        canMsgSnd.data[1] = canMsgRcv.data[1];
        canMsgSnd.data[2] = canMsgRcv.data[2];
        canMsgSnd.data[4] = canMsgRcv.data[4];
        canMsgSnd.data[5] = canMsgRcv.data[5];
        canMsgSnd.data[6] = canMsgRcv.data[6];
        canMsgSnd.data[7] = canMsgRcv.data[7];
        canMsgSnd.can_id = 0x036; 
        canMsgSnd.can_dlc = 8;
        CAN1.sendMessage(&canMsgSnd);
     }

          

   
    
 else {   //any other code, sended to the display without changes
 CAN1.sendMessage( & canMsgRcv);
  }
      
        if (botaotela == true) { //external button to change the display page
        if (digitalRead(8) == LOW) {

        botao = botao + 1;
        canMsgSnd.data[0] = 0x00;
        canMsgSnd.data[1] = 0x00;
        canMsgSnd.data[2] = 0x00;
        canMsgSnd.data[3] = (botao & 0xFF); //rotary button on the car, increase 1 to select the next page
        canMsgSnd.data[4] = 0x00;
        canMsgSnd.can_id = 0x0A2; 
        canMsgSnd.can_dlc = 5;
        CAN1.sendMessage(&canMsgSnd);
      
        }
        }
  
 //Shiftligh and personal page dials 
 //VER SE O RADIO MANDA ALGUMA INFO NA CAN 0x2E9 e se manda o 0x1A9
        if (id == 745) { 
        }
        else {
        canMsgSnd.data[0] = 0x01;
         if (shiftlight == true && rpm >= rpmmax){
        canMsgSnd.data[1] = 0x57; // Show the RPM gauge and change the color to RED (shiftlight)
        }
         else {
        canMsgSnd.data[1] = 0x17; // show the RPM gauge
        }
        canMsgSnd.data[2] = 0x30; //radio gauge
        canMsgSnd.can_id = 0x2E9; 
        canMsgSnd.can_dlc = 3;
        CAN1.sendMessage(&canMsgSnd);
        }


        
 //------------------------------------
 //Read the display can values and sent to the car
 //----------------------------
 if( CAN1.readMessage( & canMsgRcvT) == MCP2515::ERROR_OK ){;
    idT = canMsgRcvT.can_id;
    lenT = canMsgRcvT.can_dlc;
 }
   if (idT == 535 && lenT == 8)  { //read the brightness value from the display
   bnovo = (((canMsgRcvT.data[0]) * 0.0625)+32);
   trip = (canMsgRcvT.data[1]);
   }  
  //To reset the trip holding the side button of the display
 
   if (trip == 130) {     
        canMsgSndT.data[0] = canMsgRcvT.data[0];
        canMsgSndT.data[1] = 0x82;
        canMsgSndT.data[2] = canMsgRcvT.data[2];
        canMsgSndT.data[3] = canMsgRcvT.data[3];
        canMsgSndT.data[4] = canMsgRcvT.data[4];
        canMsgSndT.data[5] = canMsgRcvT.data[5];
        canMsgSndT.data[6] = canMsgRcvT.data[6];
        canMsgSndT.data[7] = canMsgRcvT.data[7];
        canMsgSndT.can_id = 0x217; 
        canMsgSndT.can_dlc = 8;
       CAN0.sendMessage(&canMsgSndT);
       if (debug == true){
       Serial.println("reset trip");
       }
  }
  
   
 else{
 //CAN0.sendMessage(&canMsgRcvT); //send the dashboard infos to the car
 }
 if (debug == true) {
Serial.println(rpm);


 }
 
}
