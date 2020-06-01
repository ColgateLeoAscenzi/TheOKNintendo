#include<ILI9341_t3.h>
#include<font_Arial.h>
#include<XPT2046_Touchscreen.h>
#include<SPI.h>
#define CS_PIN 8
#define TFT_DC 9
#define TFT_CS 10
XPT2046_Touchscreen ts(CS_PIN);
#define TIRQ_PIN 2
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);

//for input
const int APIN = 16; const int BPIN = 14; const int LPIN = 18; const int RPIN = 15; const int UPIN = 19; const int DPIN = 17;
const int NUMKEYS = 6;
int pinnum[NUMKEYS] = {UPIN,LPIN,DPIN,APIN,RPIN,BPIN};
int heldkeys[NUMKEYS] = {0,0,0,0,0,0}; //UP, LEFT, DOWN, A, RIGHT, B
String keynames[NUMKEYS] = {"Up","Left","Down","A","Right","B"};

//for player
int px = 12*12;
int py = 16*12;
int oldpx = 12*12;
int oldpy = 16*12;
const float PSPEED = 6;
int xVel = 0;
int yVel = 0;

//color pallet

#define SKIN 0xAD55
#define EYE 0x0000
#define MAROON 0x8126
#define GRAY 0x5ACB
#define BLUE 0x32AE

#define PACYELLOW 0xFFE0
#define PACBLUE 0x18D4

int GAMEMODE = 0;
int GAMEMODE1 = 0;
int GAMEMODE2 = 0;
int mapnum = 0;

//2 maps, 8 walls each, x1 y1 x2 y2 bounding box
int maps[2][16][4];

//food
int food[2][77][2];

int MAXSCORE = 153;
int score = 0;

//for ghost
int gx = 12*12;
int gy = 9*12;
int oldgx = 12*12;
int oldgy = 9*12;
int GSPEED = 7;



//NEW METHOND
int FOOD = 4;
int WALLV = 1;
int WALLH = 2;
int WALLB = 3;
int P1 = 5;
int supermap[26][20];


boolean held = false;
int tick = 0;
boolean firstClick = true;
boolean gameover = false;

int facing = -1;
  
void setup() {

  tft.begin();
  tft.fillScreen(ILI9341_BLACK); // blanks the screen
  tft.setRotation(1); 

  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(3);
  tft.setCursor(20,20);
  //enable buttons
 
  pinMode(UPIN, INPUT); //up
  pinMode(LPIN, INPUT); //left
  pinMode(DPIN, INPUT); //down
  
  pinMode(APIN, INPUT); //A
  pinMode(BPIN, INPUT); //B
  pinMode(RPIN, INPUT); //right

  pinMode(20, INPUT); //recieves gamemode
  pinMode(21, INPUT);
  pinMode(22, OUTPUT);//sends signal

  //debug mode
  pinMode(3, INPUT);
  //pin 20 and 21
  //if pin 20 high pin 21 low, pokemon
  //if pin 21 high 20 low, pacman
  //both high, super mario


  digitalWrite(22, HIGH);
  delay(50);
  
  GAMEMODE1 = digitalRead(20);
  GAMEMODE2 = digitalRead(21);
  

  Serial.print("Read 20 at ");Serial.print(GAMEMODE1);Serial.print(" and pin 21 at ");Serial.println(GAMEMODE2);

  
  Serial.begin(9600);
  //while (! Serial); //
  if(GAMEMODE1 == 1 && GAMEMODE2 == 0){
    GAMEMODE = 0;
    Serial.println("Playing Pokemon Ho!");
  }
  else if(GAMEMODE1 == 0 && GAMEMODE2 == 1){
    GAMEMODE = 1;
    Serial.println("Playing Pac-Man!");
    initializeGrid(supermap);
    drawGrid(supermap);
    drawPac(px,py, oldpx, oldpy, facing, tick);
    drawGhost(gx,gy, oldgx, oldgy, facing, tick);
  
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    
    tft.setCursor(0*12, 0*12);
    tft.print("Pac-Man");

    tft.fillCircle(3*12+2, 18*12+8, 2, ILI9341_WHITE);
    tft.setCursor(4*12,18*12);
    tft.print("x");tft.print(score);
    
    
  }
  else{
    GAMEMODE = 2;
    Serial.println("Playing Super Segall Bros!");
  }

}



void loop() {
 if(!gameover){
  
  if(digitalRead(20) == 0 && digitalRead(21) == 0){
    GAMEMODE = -1;
  }
  delay(50);
  
  //get key inputs
  for(int i = 0; i < NUMKEYS; i++){
    heldkeys[i] = digitalRead(pinnum[i]);
    if(heldkeys[i] == 1){
      held = true;
    }
  }

  //apply key inputs
  if(held == true){
    for(int i = 0; i < NUMKEYS; i++){
      if(heldkeys[i] == 1){
        //Serial.println(keynames[i]+" is being held");
        if(keynames[i] == "Left"){
          if(px%12 == 0 && py%12 == 0){
            if(supermap[px/12-1][py/12] != 1 && supermap[px/12-1][py/12] != 2 && supermap[px/12-1][py/12] != 3){
              facing = 0;
              xVel = -PSPEED;
              yVel = 0;
            }
          }
        }

        if(keynames[i] == "Right"){
          if(px%12 == 0 && py%12 == 0){
            if(supermap[px/12+1][py/12] != 1 && supermap[px/12+1][py/12] != 2 && supermap[px/12+1][py/12] != 3){
              facing = 1;
              xVel = PSPEED;
              yVel = 0;
            }
          }
        }

        if(keynames[i] == "Up"){
          if(px%12 == 0 && py%12 == 0){
            if(supermap[px/12][py/12-1] != 1 && supermap[px/12][py/12-1] != 2 && supermap[px/12][py/12-1] != 3){
              facing = 2;
              yVel = -PSPEED;
              xVel = 0;
            }
          }
        }

        if(keynames[i] == "Down"){
          if(px%12 == 0 && py%12 == 0){
            if(supermap[px/12][py/12+1] != 1 && supermap[px/12][py/12+1] != 2 && supermap[px/12][py/12+1] != 3){
              facing = 3;
              yVel = PSPEED;
              xVel = 0;
            }
          }
        }

        if(keynames[i] == "A"){
          //Serial.print(px);Serial.print(" ");Serial.println(py);
//          drawGrid(supermap);
        }
        if(keynames[i] == "B"){
          Serial.println(score);
//          tft.fillScreen(ILI9341_BLACK);
        }

        
      }
    }
  }
  held = false;

  //redraw elements
  if(GAMEMODE == 0){
    
  }
  else if(GAMEMODE == 1){
    if(facing == 1){
      if(supermap[px/12+1][py/12] != 1 && supermap[px/12+1][py/12] != 2 && supermap[px/12+1][py/12] != 3){
         px += xVel;
         py += yVel;
      }
      
    }
    if(facing == 0){
      if(px%12 == 0 && py%12 == 0){
        if(supermap[px/12-1][py/12] != 1 && supermap[px/12-1][py/12] != 2 && supermap[px/12-1][py/12] != 3){
          px += xVel;
          py += yVel;
        }
      }
      else{
         px += xVel;
         py += yVel;
      }

    }
    if(facing == 2){
      if(px%12 == 0 && py%12 == 0){
        if(supermap[px/12][py/12-1] != 1 && supermap[px/12][py/12-1] != 2 && supermap[px/12][py/12-1] != 3){
           px += xVel;
           py += yVel;
        }
      }
      else{
        px+=xVel;
        py+=yVel;
      }
    }
    if(facing == 3){
    
      if(supermap[px/12][py/12+1] != 1 && supermap[px/12][py/12+1] != 2 && supermap[px/12][py/12+1] != 3){
         px += xVel;
         py += yVel;
      }
    }


    //drawWire(supermap);

  }
  else if(GAMEMODE == 2){

  }
  else if(GAMEMODE == -1){

      for(int x = 0; x < 320; x++){
        for(int y = 0; y < 240; y++){
          tft.fillRect(x,y,1,1,16777215*rand());
        }
      }
  }
 
  
  if(oldpx != px || oldpy != py){

//    Serial.print("REAL CO ");Serial.print(px);Serial.print(" ");Serial.println(py);
//    Serial.print("FAKE CO ");Serial.print(px/12);Serial.print(" ");Serial.println(py/12);
    //Serial.print(px/12);Serial.print(" ");Serial.println(py/12);
    //Pokemon Colgate
    if(GAMEMODE == 0){
      //draw character
      drawRaider(px,py, oldpx, oldpy, facing, tick);
    }
    //Pac-Man
    else if(GAMEMODE == 1){
      
      
      //CHECK COLLISION
      if(px%12 == 0 && py % 12 == 0){
        if(supermap[px/12][py/12] == 0){
          score+=1;
          
          tft.setTextSize(2);
          tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
          tft.setCursor(4*12,18*12);
          tft.print("x");tft.print(score);
          supermap[px/12][py/12] = -2;
        }

      }
      
      //draw walls
      drawGrid(supermap);

      //draw player last
      drawPac(px,py, oldpx, oldpy, facing, tick);

      
//      if(onFood(food[mapnum], px,py)){
//        score+=1;
//        tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
//        tft.setCursor(0,0);
//        tft.print(score);
//  
//      }
   
      if(MAXSCORE == score && MAXSCORE != 0){
        tft.setCursor(100,90);
        tft.setTextSize(3);
        tft.print("YOU WIN");
        gameover = true;
      }

      drawGhost(gx,gy, oldgx, oldgy, facing, tick);
      
    }
    else{
      drawGhost(gx,gy, oldgx, oldgy, facing, tick);
      
    }
    
  }


  

  //renew player coordinates
  oldpx = px;
  oldpy = py;
  //increase world tick
  tick+=1;
 }


}
//int FOOD = 4;
//int WALLV = 1;
//int WALLH = 2;
//int WALLB = 3;
//int P1 = 2;
//int supermap[26][20];

void initializeGrid(int supermap[26][20]){
  int row = 20;
  int col = 26;

  //clear map
  for(int x = 0; x < col; x++){
    for(int y = 0; y < row; y++){
      supermap[x][y] = 0;
    }
  }

  //add walls
  for(int x = 1; x < col-1; x++){
    for(int y = 1; y < row-2; y++){
      if((x == 1 || x == 24) && (y == 1 || y == 17)){
        supermap[x][y] = WALLB;
      }
      else if(x == 1 || x == 24){
        supermap[x][y] = WALLV;
      }
      else if(y == 1 || y == 17){
        supermap[x][y] = WALLH;
      }

      if(y == 1){
        if(x == 12 || x == 13){
          supermap[x][y] = WALLB;
        }
      }
      else if(y == 2){
        if(x == 12 || x == 13){
          supermap[x][y] = WALLV;
        }
      }
      else if(y == 3){
        if(x == 12 || x == 13){
          supermap[x][y] = WALLV;
        }
        if(x == 3 || x == 6 || x == 8 || x == 10 || x == 15 || x == 17 || x == 19 || x == 22){
          supermap[x][y] = WALLB;
        }
        if(x == 4 || x == 5 || x == 9 || x == 16 || x == 20 || x == 21){
          supermap[x][y] = WALLH;
        }
      }
      else if(y == 4){
        if(x == 12 || x == 13){
          supermap[x][y] = WALLB;
        }
        if(x == 3 || x == 6 || x == 8 || x == 10 || x == 15 || x == 17 || x == 19 || x == 22){
          supermap[x][y] = WALLV;
        }
        if(x == 4 || x == 5 || x == 9 || x == 16 || x == 20 || x == 21){
          supermap[x][y] = -1;
        }
      }
      else if(y == 5){
        if(x == 3 || x == 6 || x == 8 || x == 10 || x == 15 || x == 17 || x == 19 || x == 22){
          supermap[x][y] = WALLV;
        }
        if(x == 4 || x == 5 || x == 9 || x == 16 || x == 20 || x == 21){
          supermap[x][y] = -1;
        }
      }
      else if(y == 6){
        if(x == 3 || x == 6 || x == 8 || x == 10 || x == 12 || x == 13 || x == 15 || x == 17 || x == 19 || x == 22){
          supermap[x][y] = WALLB;
        }
        if(x == 4 || x == 5 || x == 9 || x == 16 || x == 20 || x == 21){
          supermap[x][y] = WALLH;
        }
      }
      else if(y == 7){
       if(x == 12 || x == 13){
          supermap[x][y] = WALLV;
        }
      }
      else if(y == 8){
       if(x == 2 || x == 3 || x == 4 || x == 5 || x == 20 || x == 21 || x == 22 || x == 23){
          supermap[x][y] = WALLH;
        }
        if(x == 1 || x == 11 || x == 14 || x == 6 || x == 12 || x == 13 || x == 19 || x == 8 || x == 9 || x == 16 || x == 17 || x == 24){
          supermap[x][y] = WALLB;
        }
      }
      else if(y == 9){
       if(x == 2 || x == 3 || x == 4 || x == 5 || x == 6 || x == 19 || x == 20 || x == 21 || x == 22 || x == 23){
          supermap[x][y] = WALLH;
        }
        if(x == 8 || x == 9 || x == 16 || x == 17 || x == 11 || x == 14){
          supermap[x][y] = WALLV;
        }
        if(x == 1 || x == 6 || x == 19 || x == 24){
          supermap[x][y] = WALLB;
        }
        if(x == 12 || x == 13){
          supermap[x][y] = -1;
        }
      }
      else if(y == 10){
        if(x == 8 || x == 9 || x == 16 || x == 17 || x == 11 || x == 14){
          supermap[x][y] = WALLV;
        }
        if(x == 12 || x == 13){
          supermap[x][y] = -1;
        }
      }
      else if(y == 11){
        if(x == 3 || x == 6 || x == 8 || x == 9 || x == 11 || x == 14 || x == 16 || x == 17 || x == 19 || x == 22){
          supermap[x][y] = WALLB;
        }
        if(x == 12 || x == 13){
          supermap[x][y] = WALLH;
        }
        if(x == 4 || x == 5 || x == 20 || x == 21){
          supermap[x][y] = WALLH;
        }
      }
      else if(y == 12){
        if(x == 3 || x == 5 || x == 6 || x == 19 || x == 20 || x == 22){
          supermap[x][y] = WALLB;
        }
        if(x == 4 || x == 21){
          supermap[x][y] = WALLH;
        }
      }
      else if(y == 13){
        if(x == 5 || x == 6 || x == 8 || x == 17 || x == 19 || x == 20){
          supermap[x][y] = WALLV;
        }
        if(x > 10 && x < 15){
          supermap[x][y] = WALLH;
        }
        if(x == 10 || x == 15){
          supermap[x][y] = WALLB;
        }
      }
      else if(y == 14){
        if(x == 2 || x == 23 || x == 11 || x == 14){
          supermap[x][y] = WALLH;
        }
        if(x==1 || x == 3 || x == 10  || x == 22 || x == 12 || x == 13 || x == 15 || x == 24){
          supermap[x][y] = WALLB;
        }
        if(x == 5 || x == 6 || x == 8 || x == 17 || x == 19 || x == 20){
          supermap[x][y] = WALLV;
        }
      }
      else if(y == 15){
        if(x == 2 || x == 23){
          supermap[x][y] = WALLH;
        }
        if(x == 1 || x == 3 || x == 5 || x == 6 || x == 12 || x == 13 || x == 19 || x == 20 || x == 22 || x == 24){
          supermap[x][y] = WALLB;
        }
        if(x == 8 || x == 17){
          supermap[x][y] = WALLV;
        }
      }
      else if(y == 16){
        if(x == 10 || x == 15){
          supermap[x][y] = WALLV;
        }
      }
      else if(y == 17){
        if(x == 10 || x == 15){
          supermap[x][y] = WALLB;
        }
      }
//      else{
//        supermap[x][y] = FOOD;
//      }
    }
  }

  supermap[12][16] = P1;
}
void drawGrid(int supermap[26][20]){
  int row = 20;
  int col = 26;

  for(int x = 1; x < col-1; x++){
    for(int y = 1; y < row-2; y++){
      drawWallPiece(x,y, supermap);

    }
  }

}

void drawWire(int supermap[26][20]){
  int row = 20;
  int col = 26;

  for(int x = 1; x < col-1; x++){
    for(int y = 1; y < row-2; y++){
      tft.drawRect(x*12,y*12,12,12,ILI9341_RED);

    }
  }

}



void drawWallPiece(int x, int y, int supermap[26][20]){
      if(supermap[x][y] == WALLV){
        tft.fillRect(x*12+4,y*12,4,12,ILI9341_BLUE);
      }
      else if(supermap[x][y] == WALLH){
        tft.fillRect(x*12,y*12+4,12,4,ILI9341_BLUE);
      }
      else if(supermap[x][y] == WALLB){
        int U = supermap[x][y-1];
        int D = supermap[x][y+1];
        int L = supermap[x-1][y];
        int R = supermap[x+1][y];

        //Bottom right
        if(U != 0 && (D == 0 || D == 5 || D == -2) && L != 0 && (R == 0 || R == 5 || R == -2)){
          tft.fillRect(x*12+4,y*12,4,6,ILI9341_BLUE);
          tft.fillRect(x*12,y*12+4,6,4,ILI9341_BLUE);
        }
        //Bottom left
        else if(U != 0 && (D == 0 || D == 5 || D == -2) && (L == 0 || L == 5 || L == -2) && R != 0){
          tft.fillRect(x*12+4,y*12,4,6,ILI9341_BLUE);
          tft.fillRect(x*12+6,y*12+4,6,4,ILI9341_BLUE);
        }
        //top right
        else if((U == 0 || U == 5 || U == -2) && D != 0 && L != 0 && (R == 0 || R == 5 || R == -2)){
          tft.fillRect(x*12+4,y*12+6,4,6,ILI9341_BLUE);
          tft.fillRect(x*12,y*12+4,6,4,ILI9341_BLUE);
                 
        }
       //top left
        else if((U == 0 || U == 5 || U == -2) && D != 0 && (L == 0 || L == 5 || L == -2) && R != 0){
          tft.fillRect(x*12+4,y*12+6,4,6,ILI9341_BLUE);
          tft.fillRect(x*12+6,y*12+4,6,4,ILI9341_BLUE);
        }
         //bottom T
        else if(U != 0 && (D == 0 || D == 5 || D == -2) && L != 0 && R != 0){
          tft.fillRect(x*12+4,y*12,4,6,ILI9341_BLUE);
          tft.fillRect(x*12,y*12+4,12,4,ILI9341_BLUE);
        }
        //right T
         else if(U != 0 && D != 0 && L != 0 && (R == 0 || R == 5 || R == -2)){
          tft.fillRect(x*12+4,y*12,4,12,ILI9341_BLUE);
          tft.fillRect(x*12,y*12+4,6,4,ILI9341_BLUE);
        }
       //left T
         else if(U != 0 && D != 0 && (L == 0 || L == 5 || L == -2) && R != 0){
          tft.fillRect(x*12+4,y*12,4,12,ILI9341_BLUE);
          tft.fillRect(x*12+6,y*12+4,6,4,ILI9341_BLUE);
        }
        //top T
         else if((U == 0 || U == 5 || U == -2) && D != 0 && L != 0 && R != 0){
          tft.fillRect(x*12+4,y*12+6,4,6,ILI9341_BLUE);
          tft.fillRect(x*12,y*12+4,12,4,ILI9341_BLUE);
        }
        //SPECIAL CASES
        //bottom hole
        else if(U != 0 && D == -1 && L != 0 && R != 0){
          tft.fillRect(x*12+4,y*12,4,6,ILI9341_BLUE);
          tft.fillRect(x*12,y*12+4,12,4,ILI9341_BLUE);
        }
          //top horiz
         else if(U == WALLH && D != 0 && L != 0 && R != 0){
          tft.fillRect(x*12+4,y*12+6,4,6,ILI9341_BLUE);
          tft.fillRect(x*12,y*12+4,12,4,ILI9341_BLUE);
        }
        else{
          tft.fillRect(x*12+4,y*12,4,12,ILI9341_BLUE);
          tft.fillRect(x*12,y*12+4,12,4,ILI9341_BLUE);
        }

       // tft.fillRect(x*12,y*12,12,12,ILI9341_GREEN);
      }
      else if(supermap[x][y] == 0){
        tft.fillCircle(x*12+6,y*12+6,2,ILI9341_WHITE);
      }

}

void drawRaider(int x, int y, int oldx, int oldy, int facing, int frame){
  
  tft.fillRect(oldx-4,oldy,8,15,ILI9341_BLACK);//draw shirt
  tft.fillCircle(oldx,oldy, 5, ILI9341_BLACK);//draw head
  tft.fillCircle(oldx-2,oldy-2,1, ILI9341_BLACK); //eyes
  tft.fillCircle(oldx+2,oldy-2,1, ILI9341_BLACK);
  tft.fillRect(oldx-8,oldy-8,16,4, ILI9341_BLACK);  //hat
  tft.fillRect(oldx-5,oldy-12,10,4, ILI9341_BLACK);
  
  tft.fillRect(x-4,y,8,15,MAROON);//draw shirt
  tft.fillCircle(x,y, 5, SKIN);//draw head
  tft.fillCircle(x-2,y-2,1, EYE); //eyes
  tft.fillCircle(x+2,y-2,1, EYE);
  tft.fillRect(x-8,y-8,16,4, MAROON);  //hat
  tft.fillRect(x-5,y-12,10,4, MAROON);
}

void drawPac(int x, int y, int oldx, int oldy, int facing, int frame){
  tft.fillCircle(oldx+6,oldy+6, 8, ILI9341_BLACK);
  
  tft.fillCircle(x+6,y+6, 8, ILI9341_YELLOW);
  if(frame %2 == 0){
    //right
    if(facing == 1){
      tft.fillTriangle(x+6,y+6,x+6+6,y+6-6.5,x+6+6,y+6+6.5,ILI9341_BLACK);
      tft.fillTriangle(x+6+9.5,y+6,x+6+6,y+6-6.5,x+6+6,y+6+6.5,ILI9341_BLACK);
    }
    //left
    if(facing == 0){
      tft.fillTriangle(x+6,y+6,x+6-6,y+6-6.5,x+6-6,y+6+6.5,ILI9341_BLACK);
      tft.fillTriangle(x+6-9.5,y+6,x+6-6,y+6-6.5,x+6-6,y+6+6.5,ILI9341_BLACK);
    }
    //up
    if(facing == 2){
      tft.fillTriangle(x+6,y+6,x+6-6.5,y+6-6,x+6+6.5,y+6-6,ILI9341_BLACK);
      tft.fillTriangle(x+6,y+6-9.5,x+6-6.5,y+6-6,x+6+6.5,y+6-6,ILI9341_BLACK);
    }
    //down
    if(facing == 3){
      tft.fillTriangle(x+6,y+6,x+6-6.5,y+6+6,x+6+6.5,y+6+6,ILI9341_BLACK);
      tft.fillTriangle(x+6,y+6+9.5,x+6-6.5,y+6+6,x+6+6.5,y+6+6,ILI9341_BLACK);
    }

  }

}

void drawGhost(int x, int y, int oldx, int oldy, int facing, int frame){
  tft.fillCircle(oldx+6,oldy+6, 7, ILI9341_BLACK);

  tft.fillCircle(x+6,y+6, 7, ILI9341_RED);
  tft.fillCircle(x+6-3,y+6-3, 2, ILI9341_WHITE);
  tft.fillCircle(x+6+3,y+6-3, 2, ILI9341_WHITE);

  if(facing == 1){
    tft.fillRect(x+6-3,y+6-3, 2,2, ILI9341_BLUE);
    tft.fillRect(x+6+3,y+6-3, 2,2, ILI9341_BLUE);
  }
  else if(facing == 0){
    tft.fillRect(x+6-3-2,y+6-3, 2,2, ILI9341_BLUE);
    tft.fillRect(x+6+3-2,y+6-3, 2,2, ILI9341_BLUE);
  }
  else if(facing == 2){
    tft.fillRect(x+6-3-1,y+6-3-2, 2,2, ILI9341_BLUE);
    tft.fillRect(x+6+3-1,y+6-3-2, 2,2, ILI9341_BLUE);
  }
  else if(facing == 3){
    tft.fillRect(x+6-3,y+6-3+1, 2,2, ILI9341_BLUE);
    tft.fillRect(x+6+3,y+6-3+1, 2,2, ILI9341_BLUE);
  }

  


}

void drawWall(int xi, int yi, int xf, int yf){
  //vertical
  if(xi == xf){
    tft.fillRect(xi-2.5,yi-2.5,2,yf-yi+5, PACBLUE);
    tft.fillRect(xi+2.5,yi-2.5,2,yf-yi+5, PACBLUE);
  }
  //horizontal
  if(yi == yf){
    tft.fillRect(xi-2.5,yi-2.5,xf-xi+5,2, PACBLUE);
    tft.fillRect(xi-2.5,yi+2.5,xf-xi+5,2, PACBLUE);
  }
}



boolean inWall(int walls[16][4], int px, int py){
  int TOL = 15;
  for(int i = 0; i < 16; i++){
    //x1, y1, x2, y2
    //vertical
    if(walls[i][0] == walls[i][2]){
      if(px > walls[i][0] - TOL && px < walls[i][2] + TOL
        && py > walls[i][1] - TOL/2 && py < walls[i][3] + TOL/2){

        return true;
      }
    }
    else{
      if(px > walls[i][0] - TOL/2 && px < walls[i][2] + TOL/2
        && py > walls[i][1] - TOL && py < walls[i][3] + TOL){
       
        return true;
      }
    }

  }
  return false;
}

boolean onFood(int foods[77][2], int px, int py){
  int TOL = 10;
  for(int i = 0; i < 77; i++){
    //x1, y1, x2, y2
    //vertical

    if(px > foods[i][0] - TOL && px < foods[i][0] + TOL
      && py > foods[i][1] - TOL && py < foods[i][1] + TOL){
        //tft.drawRect(foods[i][0] -1, foods[i][1] -1, 2, 2, ILI9341_RED);
        foods[i][0] = -100;
        foods[i][1] = -100;
        return true;
    }
  }
  return false;
}


void drawWalls(int mapNum){

  
  if(mapNum == 0){
      int mapWall[16][4]= 
  {{10,10,305,10}, {10,226,305,226}, {10,10,10,226}, {305,10,305,226},
  {157,10,157,50}, {157,176,157,226}, {105,90,210,90}, {105,136,210,136},
  {60,50,105,50}, {60,50,60,86}, {60,176,105,176}, {60,150,60,176},
  {210,50,255,50},{255,50,255,86},{210,176,255,176},{255,140,255,176}};
  
    //top
    drawWall(10,10,305,10);

    //bottom
    drawWall(10,226,305,226);

    //left
    drawWall(10,10,10,226);

    //right
    drawWall(305,10,305,226);

    //mid top, mid bot, mid upper, mid lower
    drawWall(157,10,157,50);

    drawWall(157,176,157,226);

    drawWall(105,90,210,90);

    drawWall(105,136,210,136);

    //TL TR BL BR
    drawWall(60,50,105,50);

    drawWall(60,50,60,86);

    drawWall(60,176,105,176);
    
    drawWall(60,140,60,176);

// flipped
    drawWall(210,50,255,50);

    drawWall(255,50,255,86);

    drawWall(210,176,255,176);
    
    drawWall(255,140,255,176);
    


    //
  
    //walls
    for(int i = 0; i < 16; i++){
      //x and y values
      for(int j = 0; j < 4; j++){
        maps[mapNum][i][j] = mapWall[i][j];
      }
      
    }
  }

}

//
////NEW METHOND
//int FOOD = 0;
//int WALLV = 1;
//int WALLH = 2;
//int P1 = 2;
//int supermap[20][26];

  


