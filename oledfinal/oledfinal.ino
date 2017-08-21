//Version .9
//todo
// command:??? 
// accents
// position of /10
// comments & cleanup
// data verification: role ???

#include <Wire.h>
#include <SPI.h>   
#include <U8g2lib.h>
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

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2c(U8G2_R0, 24, 22);   //good 0  scl sda
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2e(U8G2_R0, 25, 23);   //good 1 
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2d(U8G2_R0, 26, 28);   //good 2
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2f(U8G2_R0, 27, 29);   //good 3
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2a(U8G2_R0, 32, 30);   //good 4
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2g(U8G2_R0, 33, 31);   //good 5
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2b(U8G2_R0, 34, 36);   //good 6
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2h(U8G2_R0, 35, 37);   //good 7 
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2k(U8G2_R0, 40, 38);
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2j(U8G2_R0, 41, 39);   //good 9
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2l(U8G2_R0, 42, 44);   //good 10
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2i(U8G2_R0, 43, 45); 

U8G2_SSD1306_128X64_NONAME_F_SW_I2C screen[] = {u8g2a, u8g2b, u8g2c, u8g2d, u8g2e, u8g2f, u8g2g, u8g2h, u8g2i, u8g2j, u8g2k, u8g2l};

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
  
  char pid[2];
  for (int i=0; i < numPlayers; i++) {   //initialise screens
    screen[i].begin();
    
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
    
  Serial.println("This demo expects 3 pieces of data - command, an integer and a string [32]");
  Serial.println("Enter data in this style <command,screen,data>  ");
  Serial.println();
  //Serial.println(F("42"));
}

//============

void loop() {
    recvWithStartEndMarkers();
    if (newData == true) {
        strcpy(tempChars, receivedChars);
            // this temporary copy is necessary to protect the original data
            //   because strtok() replaces the commas with \0
        parseData();
  //      showParsedData();
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
    Serial.print("Command ");
    Serial.println(command);
    Serial.print("For Screen ");
    Serial.println(target);
    Serial.print("Data ");
    Serial.println(data);
}

void doCommand() {
  switch(command) {
    case 'p':
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
      screen[target].clearBuffer();
      screen[target].setDrawColor(1);
      screen[target].setFont(u8g2_font_ncenB14_tr);
      screen[target].drawUTF8(0,20, text);
      screen[target].sendBuffer();
}

void drawPlayer(int target) {
      char value[3];
    if (target <= numPlayers) {
        screen[target].clearBuffer();
        screen[target].setDrawColor(1);
        screen[target].setFont(u8g2_font_6x13_tf);
        screen[target].drawUTF8(0, 10, "Centrale");         // print name
        screen[target].drawUTF8(0, 22, playerName[player[target][0]]);
        itoa(player[target][1],&value[0],10);               // print value a
        screen[target].setFont(u8g2_font_inb24_mn);
        screen[target].drawStr(0,64, &value[0]);
        itoa(player[target][2],&value[0],10);             // print value b
        screen[target].setFont(u8g2_font_inb16_mn);
        screen[target].drawStr(40 ,64, "/");
        screen[target].drawStr(54,64, &value[0]);
        screen[target].setDrawColor(0);                                       // print icon
        screen[target].drawXBMP(80, 0, iconHeight, iconWidth, playerIcon[player[target][0]]);  
        screen[target].sendBuffer();
     }
}

