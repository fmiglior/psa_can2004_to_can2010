#include <SPI.h>
#include <mcp2515.h>
#include <avr/wdt.h>


MCP2515 CAN0(10); // recebe da rede can pino10
MCP2515 CAN1(9); // envia pro painel pino9
struct can_frame canMsgRcv;
struct can_frame canMsgSnd;
struct can_frame canMsgSndT; //envia mensagens da tela pra bsi
struct can_frame canMsgRcvT; //recebido can da tela
int id; //canMsgRcv.can_id
int len; //canMsgRcv.can_dlc
int idT; //canMsgRcvT.can_id
int lenT; //canMsgRcvT.can_dlc
unsigned char bnovo; //novo brilho
int rpm; //valor da rpm
unsigned char trip; //valor botão trip
unsigned char botao = 0; //bota troca tela

//-----------------------------------
//AREA DE CONFIGURACOES
bool shiftlight = true; //para ativar ou desativar shiftlight
int rpmmax = 5500; //rpm do corte, ajuste aqui
bool botaobrilho = false; //True para ajustar o brilho pelo botao lateral, false para a BSI ajustar o brilho
bool botaotela = true; //botao para troca de tela
bool debug = false; //ativar para monitorar o codigo
//-----------------------------------


void setup() {
wdt_enable(WDTO_8S); //tempo do watchdog, reset automatico em caso de travar o codigo

  SPI.begin();
  CAN0.reset();
  CAN0.setBitrate(CAN_125KBPS,MCP_8MHZ);
  CAN0.setNormalMode();
  CAN1.reset();
  CAN1.setBitrate(CAN_125KBPS,MCP_8MHZ);
  CAN1.setNormalMode(); 

  pinMode(8, INPUT_PULLUP); //pino do botao
  
  //while (!Serial);
  if (debug == true) {
  Serial.begin(115200);  
  Serial.println("RODANDO EM MODO DEPURAÇÃO!");
  }
  

  
 rpmmax = rpmmax * 0.0312;

}

void loop() {
wdt_reset();  
  
   if( CAN0.readMessage( & canMsgRcv) == MCP2515::ERROR_OK ){
    id = canMsgRcv.can_id;
    len = canMsgRcv.can_dlc;
      if(id == 182 && shiftlight == true) { //pega o valor do rpm
      rpm = canMsgRcv.data[0] ;
    }
  
   }
     if (id == 296 && len == 8)  { //ID 128
        
        canMsgSnd.data[0] = canMsgRcv.data[4];
        canMsgSnd.data[1] = canMsgRcv.data[6];
        canMsgSnd.data[2] = canMsgRcv.data[7];
        canMsgSnd.data[3] = canMsgRcv.data[3];
        canMsgSnd.data[4] = canMsgRcv.data[1];
        canMsgSnd.data[5] = canMsgRcv.data[0]; //ajustar por causa da luz de combustivel
        canMsgSnd.data[6] = 0x04;// mantem o painel ligado
        canMsgSnd.data[7] = 0x00;
        canMsgSnd.can_id = 0x128;
        canMsgSnd.can_dlc = 8; 
        CAN1.sendMessage(&canMsgSnd);
   
         
     }
     else if (id == 935) { // Manutençao
        canMsgSnd.data[0] = canMsgRcv.data[0];
        canMsgSnd.data[1] = canMsgRcv.data[5]; // Value x255 +
        canMsgSnd.data[2] = canMsgRcv.data[6]; // Value x1 = Number of days till maintenance (FF FF if disabled)
        canMsgSnd.data[3] = canMsgRcv.data[3]; // Value x5120 +
        canMsgSnd.data[4] = canMsgRcv.data[4]; // Value x20 = km left till maintenance
        canMsgSnd.can_id = 0x3E7; // novo ID para manutencao
        canMsgSnd.can_dlc = 5;
        CAN1.sendMessage(&canMsgSnd);
        
     }

     
     else if (id == 424 && len == 8) { //ID1A8  cruise control
        canMsgSnd.data[0] = canMsgRcv.data[1]; //velocidade
        canMsgSnd.data[1] = canMsgRcv.data[2]; //velocidade
        canMsgSnd.data[2] = canMsgRcv.data[0];
        canMsgSnd.data[3] = 0x80; //cruise ou limitador
        canMsgSnd.data[4] = 0x00;
        canMsgSnd.data[5] = 0x00; 
        canMsgSnd.data[6] = 0x00;
        canMsgSnd.data[7] = 0x00;
        canMsgSnd.can_id = 0x228; //novo id
        canMsgSnd.can_dlc = 8;
      CAN1.sendMessage(&canMsgSnd);
        canMsgSnd.data[0] =  0x00; 
        canMsgSnd.data[1] = 0x00; 
        canMsgSnd.data[2] = 0x00;
        canMsgSnd.data[3] = 0x00; 
        canMsgSnd.data[4] = 0x00; 
        canMsgSnd.data[5] = canMsgRcv.data[5];
        canMsgSnd.data[6] = canMsgRcv.data[6];
        canMsgSnd.data[7] = canMsgRcv.data[7];
        canMsgSnd.can_id = 0x1A8; //trip parcial
        canMsgSnd.can_dlc = 8;
        CAN1.sendMessage(&canMsgSnd);
      
     }
       else if (id == 360 && len == 8) { //ID168 outras luzes
        canMsgSnd.data[0] = canMsgRcv.data[0]; 
        canMsgSnd.data[1] = canMsgRcv.data[1]; 
        canMsgSnd.data[2] = canMsgRcv.data[5];
        canMsgSnd.data[3] = canMsgRcv.data[3]; 
        canMsgSnd.data[4] = canMsgRcv.data[5];
        canMsgSnd.data[5] = canMsgRcv.data[5]; 
        canMsgSnd.data[6] = canMsgRcv.data[6];
        canMsgSnd.data[7] = canMsgRcv.data[7];
        canMsgSnd.can_id = 0x168;   
        canMsgSnd.can_dlc = 8;
      CAN1.sendMessage(&canMsgSnd);
     }
     else if (id == 608 && len == 8){ //linguagem e regulagem km/l
        canMsgSnd.data[0] = 0xA1; //esta fixa em PT-BR
        canMsgSnd.data[1] = 0x1C; //C e KM/L
        canMsgSnd.data[2] = 0x97;
        canMsgSnd.data[3] = 0x9B;
        canMsgSnd.data[4] = 0xE0;
        canMsgSnd.data[5] = 0xD0;
        canMsgSnd.data[6] = 0x00;
        canMsgSnd.can_id = 0x260;
        canMsgSnd.can_dlc = 7;
        CAN1.sendMessage( & canMsgSnd);      
     }
     
     else if (id == 54) { //ID036 brilho e chave ligada
                      
         if (botaobrilho == true && (canMsgRcv.data[3] & 0xFF) >= 32) { //ajusta brilho da tela
        canMsgSnd.data[3] = (bnovo & 0xFF);  
          }
        else {
        canMsgSnd.data[3] = canMsgRcv.data[3];
       }
     
        canMsgSnd.data[0] = canMsgRcv.data[0];
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

          

   
    
 else {   //final dois ID a receberem De PARA
 CAN1.sendMessage( & canMsgRcv);
  }
      
        if (botaotela == true) { //botao para trocar a tela
        if (digitalRead(8) == LOW) {

        botao = botao + 1;
        canMsgSnd.data[0] = 0x00;
        canMsgSnd.data[1] = 0x00;
        canMsgSnd.data[2] = 0x00;
        canMsgSnd.data[3] = (botao & 0xFF);
        canMsgSnd.data[4] = 0x00;
        canMsgSnd.can_id = 0x0A2; 
        canMsgSnd.can_dlc = 5;
        CAN1.sendMessage(&canMsgSnd);
      
        }
        }
  
 //envia a informacao dos outros mostradores e o shiftlight
 //VER SE O RADIO MANDA ALGUMA INFO NA CAN 0x2E9 e se manda o 0x1A9
        if (id == 745) { 
        }
        else {
        canMsgSnd.data[0] = 0x01;
         if (shiftlight == true && rpm >= rpmmax){
        canMsgSnd.data[1] = 0x57; // RPM + cor vermelha
        }
         else {
        canMsgSnd.data[1] = 0x17; // RPM + cor azul  
        }
        canMsgSnd.data[2] = 0x30; // mostrador direita radio
        canMsgSnd.can_id = 0x2E9; 
        canMsgSnd.can_dlc = 3;
        CAN1.sendMessage(&canMsgSnd);
        }


        
 //------------------------------------
 //le as infos da can0
 //----------------------------
 if( CAN1.readMessage( & canMsgRcvT) == MCP2515::ERROR_OK ){;
    idT = canMsgRcvT.can_id;
    lenT = canMsgRcvT.can_dlc;
 }
   if (idT == 535 && lenT == 8)  { //recebe valor brilho botao tela
   bnovo = (((canMsgRcvT.data[0]) * 0.0625)+32);
   trip = (canMsgRcvT.data[1]);
   }  
  //Resetar o TRIP pelo botao lateral
 
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
 //CAN0.sendMessage(&canMsgRcvT); 
 }
 if (debug == true) {
Serial.println(rpm);

Serial.print("bnovo: ");
Serial.println(bnovo);
Serial.print("botao: ");
Serial.println(botao);
 }
 
}
