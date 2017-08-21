//Version .9.1
//todo
// command:??? 
// accents
// position of /10
// comments & cleanup
// data verification: role ???

#include <Wire.h>
#include <SPI.h>   
#include <U8g2lib.h>
extern "C" { 
  #include "utility/twi.h"  // from Wire library, so we can do bus scanning
}
#include "agriculteurs.h"
#include "agro.h"
#include "biomasse.h"
#include "charbon.h"
#include "commerces.h"
#include "energie.h"
#include "eolienne.h"
#include "gaz.h"
#include "hydro.h"
#include "hydrogene.h"
#include "industriels.h"
#include "petrole.h"
#include "photo.h"
#include "residences.h"
#include "tic.h"
#include "offline.h"

#define TCAADDR0 0x70  // multiplexer i2c Address
#define TCAADDR1 0x71

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2a(U8G2_R0, 43, 44); 

const byte numChars = 32;
const byte numPlayers = 12;
const byte numRoles = 12;   
const int iconHeight = 48;
const int iconWidth = 48;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use by strtok() function
int player[12][3];             // player role, value a, value b for each player
// list of icon xbm arrays corellating to playerName (role)
const char *playerIcon[] = {petrole_bits,charbon_bits,gaz_bits,hydro_bits,photo_bits,biomasse_bits,eolienne_bits,hydrogene_bits,energie_bits,industriels_bits,residences_bits,commerces_bits,agriculteurs_bits,tic_bits,agro_bits,offline_bits};           
const char *playerName[] = {"Pétrole","Charbon","Gaz","Hydraulique","Photovoltaique","Biomasse","Éolienne","Hydrogène","Énergie","Industriels","Résidences","Commerces","Agriculteurs","TIC","Agroalimentaire","offline"};
      // variables to hold the parsed data
char command;
int target = 0;
char data[numChars] = {0};

boolean newData = false;

//============

void setup() {
  Serial.begin(115200);
  while (!Serial);
  delay(500);
  Wire.begin();
  Serial.println(F("setting up..."));
  char pid[2];
  for (int i=0; i < numPlayers; i++) {   //initialise screens
       if ( screenSelect(i) ) {
      u8g2a.begin();
    }
    player[i][0]=i+1;
    player[i][1]=i;
    player[i][2]=12;

    char name[9]="Joueur 1";
    if (i>9) {
      itoa(i-9,&name[8],10);
    } else { 
      itoa(i+1,&name[7],10);
    }
    oledWrite(i, &name[0]); 
  }
  Serial.println(F("Instruction format: command, an integer and a string [32]"));
  Serial.println(F(" <command,screen,data>  "));
  Serial.println(F("p ping, u update all, a b change screen values, w write text to one screen, t write text to all screens, r change role, d draw one player"));
}

//============

void loop() {
    recvWithStartEndMarkers();
    if (newData == true) {
        strcpy(tempChars, receivedChars);
            // this temporary copy is necessary to protect the original data
            //   because strtok() replaces the commas with \0
        parseData();
  showParsedData();
        doCommand();
        newData = false;
    }
}

//============

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();
       if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }  else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        } else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

//============

void parseData() {
      // split the data into its parts
    char * strtokIndx; // this is used by strtok() as an index

    strtokIndx = strtok(tempChars,",");      // get the first part - the string
    command = *strtokIndx;                  // assign it to command
    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    target = atoi(strtokIndx);     // convert this part to an integer
    strtokIndx = strtok(NULL, ",");
    strcpy(data, strtokIndx);     // convert this part to a float
}

//============

void showParsedData() {
    Serial.print(F("Command "));
    Serial.println(command);
    Serial.print(F("For Screen "));
    Serial.println(target);
    Serial.print(F("Data "));
    Serial.println(data);
}

void doCommand() {
  switch(command) {
    case 'p':                             // ping
      Serial.println(F("42"));
    break;
    case 'w':
      oledWrite(target, &data[0]);        // write to one screen
    break;
    case 't':
      for (int i=0;i<numPlayers;i++){    // write to all screeens
        oledWrite(i, &data[0]);
      }
    break;
    case 'a':
      player[target][1]=atoi(data);       // change curent value
      drawPlayer(target);
    break;
    case 'b':
      player[target][2]=atoi(data);       // change max value
      drawPlayer(target);
    break;
    case 'u':
      for (int i=0;i<numPlayers;i++){       // update all screens
         drawPlayer(i);
      }
    break;  
    case 'r':
     player[target][0]=atoi(data);       // change role
      drawPlayer(target);
    break;
    case 'd':
     drawPlayer(target);              // draw 1 player
    break;
  }
}

void oledWrite(int target, char *text) {
  if (screenSelect(target)) {
      u8g2a.clearBuffer();
      u8g2a.setDrawColor(1);
      u8g2a.setFont(u8g2_font_ncenB14_tr);
      u8g2a.drawUTF8(0,20, text);
      u8g2a.sendBuffer();
  }
}

void drawPlayer(int target) {
  Serial.print(F("draw player ")); Serial.println(target);
      char value[3];
    if (target <= numPlayers) {
      if (screenSelect(target)) {
       u8g2a.clearBuffer();
        u8g2a.setDrawColor(1);
        u8g2a.setFont(u8g2_font_6x13_tf);
        u8g2a.drawUTF8(0, 10, "Centrale");         // print name
        u8g2a.drawUTF8(0, 22, playerName[player[target][0]]);
        itoa(player[target][1],&value[0],10);               // print value a
        u8g2a.setFont(u8g2_font_inb24_mn);
        u8g2a.drawStr(0,64, &value[0]);
        itoa(player[target][2],&value[0],10);             // print value b
        u8g2a.setFont(u8g2_font_inb16_mn);
        u8g2a.drawStr(40 ,64, "/");
        u8g2a.drawStr(54,64, &value[0]);
        u8g2a.setDrawColor(0);                                       // print icon
        u8g2a.drawXBMP(80, 0, iconHeight, iconWidth, playerIcon[player[target][0]]);  
        u8g2a.sendBuffer();
       }
     }
     Serial.println(F(" finished"));
}

bool screenSelect(uint8_t target) {
  uint8_t i, mxaddr;
  if (target < 6 ) {
    mxaddr = TCAADDR0;
    i = target;
    // disable the other:
    Wire.beginTransmission(TCAADDR1);
    Wire.write(0);  // no channel selected
    Wire.endTransmission();
  } else {
    mxaddr = TCAADDR1;
    i = target-6;
    Wire.beginTransmission(TCAADDR0);
    Wire.write(0);  // no channel selected
    Wire.endTransmission();
  }
  if (i > 7) return false;
  Wire.beginTransmission(mxaddr);
  Wire.write(1 << i);
  Wire.endTransmission();  
  uint8_t testData;
   uint8_t addr = 60; //screen i2c address (0-127)
    if (! twi_writeTo(addr, &testData, 0, 1, 1)) {
      //Serial.print(F("Found screen at bus "));  Serial.print(target); Serial.print(F(", address ")); Serial.println(addr); Serial.print(F(", mx ")); Serial.println(mxaddr);
      return true;
    } 
  return false;
}
