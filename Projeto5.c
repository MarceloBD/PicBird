#include <stdio.h>
#include <string.h>

// Coneccoes para o modulo GLCD
char GLCD_DataPort at PORTD;

sbit GLCD_CS1 at LATB0_bit;
sbit GLCD_CS2 at LATB1_bit;
sbit GLCD_RS  at LATB2_bit;
sbit GLCD_RW  at LATB3_bit;
sbit GLCD_EN  at LATB4_bit;
sbit GLCD_RST at LATB5_bit;

sbit GLCD_CS1_Direction at TRISB0_bit;
sbit GLCD_CS2_Direction at TRISB1_bit;
sbit GLCD_RS_Direction  at TRISB2_bit;
sbit GLCD_RW_Direction  at TRISB3_bit;
sbit GLCD_EN_Direction  at TRISB4_bit;
sbit GLCD_RST_Direction at TRISB5_bit;
// Fim das coneccoes para GLCD

// Coneccoes para a tela sensivel
sbit DriveA at LATC0_bit;
sbit DriveB at LATC1_bit;
sbit DriveA_Direction at TRISC0_bit;
sbit DriveB_Direction at TRISC1_bit;
// Fim de coneccoes para a tela sensivel 

#define NPIPES 1				// Numero de par de obstaculos na tela 
#define VPIPE 3					// Velocide dos obstaculos
#define GAP 20					// Distancia entre ostaculos
#define NSCORE 5				// Numero de pontuacoes salvas
#define MUSICKEYS 18			// Numero de notas que musica apresenta

unsigned int x_coord, y_coord; 	// Define coordenadas de pressão
unsigned int pos_x, pos_y;		// Posicao do jogador na tela
float acel = 0.5;				// Gravidade
float vel = 0.5;				// Velocidade inicial 
int gapPos;						// Posicao do espacamento entre obstaculos
int score;						// Pontuacao obtida

int pipes[NPIPES];				// Vetor com posicao dos obstaculos
int savedScore[NSCORE];			// Pontuacao salva 
char savedNickname[NSCORE][4];	// Nome salvo
char newNickname[4];			// Novo nome de 3 caracteres
int musicAux;					// Acessa nota a ser tocada
int musicTime;					// Espacamento entre notas


const int music[MUSICKEYS][2] =  {	// Matriz com notas musicais e intervalos
    {523, 1},
    {523, 1},
    {523, 1},
    {659, 3},
    {784, 2},
    {523, 1},
    {523, 1},
    {523, 1},
    {659, 3},
    {784, 2},
    {784, 2},
    {884, 1},
    {784, 1},
    {784, 1},
    {687, 1},
    {659, 1},
    {519, 1},
    {519, 2}
};

typedef enum  {erase, draw} action;	// Define apagamento ou escrita do objeto

char mode; 					// Configuracao de tela atual
							// 0 menu, 1 jogo, 2 fim de jogo, 3 pontuacao, 4 creditos

void Initialize() {
  ANSELA = 3;              // Configura portas AN0 e AN1 como analogicas
  ANSELB = 0;              // Configura porta PORTB como digital
  ANSELC = 0;              // Configura porta PORTC como digital
  ANSELD = 0;              // Configura porta PORTD como digital
  TRISA  = 3;              // Configura portas AN0 e AN1 pins como entrada

  Glcd_Init();             						   // Inicializa GLCD
  Glcd_Fill(0);                                    // Limpa GLCD

  ADC_Init();                                      // Inicializa ADC
  TP_Init(128, 64, 0, 1);                          // Inicializa tela sensivel a toque 
  TP_Set_ADC_Threshold(900);                       // Configura o limiar de ADC
}

/* Desenha ou apaga o personagem */
void drawBird(action act) {					
     Glcd_Box(pos_x-3,pos_y-3,pos_x+3,pos_y+3,act);
}

/* Som causado quando ha toque na tela */
void clickSound() {
     Sound_Play(200, 10);
}

/* Ocasiona um impulso no personagem (muda velocidade) */
void jump() {
     vel = -VPIPE;
     clickSound();
}

/* Desenha a pontuacao atual na tela de jogo */
 void printScore() {
      char aux[10];
      sprintf(aux, "%d", score);
     Glcd_Write_Text(aux, 100, 2, 1) ;
 }

 /* Incrementa pontuacao e chama funcao de desenha-la */
void makeScore() {
     score++;
     printScore();
}

/* Desenha os obstaculos na tela */
void drawPipes(action act) {
     int i;
     for(i = 0; i < NPIPES; i++){
                  if(act){
                          Glcd_Line(pipes[i],   0, pipes[i],   gapPos, draw);
                          Glcd_Line(pipes[i]+1, 0, pipes[i]+1, gapPos, draw);
                          
                          Glcd_Line(pipes[i],  gapPos+1+GAP, pipes[i],   127, draw);
                          Glcd_Line(pipes[i]+1,gapPos+1+GAP, pipes[i]+1, 127, draw);

                  }else{
                        Glcd_Line(pipes[i]+3, 0, pipes[i]+3, 127, erase);
                        Glcd_Line(pipes[i]+4, 0, pipes[i]+4, 127, erase);
                  }
                  if (pipes[i] > 80){
                     printScore();
                  }
     }
}

/* Movimenta os obstaculos na tela */
void movePipes() {
     int i;
     pipes[0] -=  3;
     if(pipes[0] < 0)   {
       pipes[0] = 127;
       gapPos = (rand() % 33) + 10;
       Glcd_Line(0, 0, 0, 127, erase);
        Glcd_Line(1, 0, 1, 127, erase);
         Glcd_Line(2, 0, 2, 127, erase);
       }
}

/* Som ao se pontuar */
void scoreSound() {
     Sound_Play(820, 50);
}

/* Som ao termino do jogo */
void gameOverSound() {
     Sound_Play(880, 200);
     Sound_Play(440, 200);
     Sound_Play(220, 200);
     Sound_Play(110, 200);
}

/* Desenha todas as maiores pontuacoes presentes na SRAM */
void printAllScores() {
     int i;
     char aux[10];
     for (i = 0; i < NSCORE; i++){
         if (savedScore[4-i] != 0) {
               sprintf(aux, "%d", savedScore[4-i]);
               Glcd_Write_Text(aux, 100, i+2, 1) ;
               Glcd_Write_Text(savedNickname[4-i], 40, i+2, 1) ;
         }
     }
}

/* Orderna as maiores pontuacoes */
void orderScore() {
     int i, j, pos, aux;
     char auxName[4];
     int smallest;
     for (i = 0; i < NSCORE; i++ )  {
         pos = i;
         smallest = savedScore[i];
       for (j = i; j < NSCORE; j++) {
           if (savedScore[j] < smallest){
              pos = j;
              smallest = savedScore[j];
           }
       }
       aux = savedScore[pos];
       savedScore[pos] = savedScore[i];
       savedScore[i] = aux;
       
       strcpy(auxName, savedNickname[pos]);
       strcpy(savedNickname[pos],savedNickname[i]);
       strcpy(savedNickname[i], auxName);
     }
}

/* Verifica se a pontuacao nova eh recorde e em caso positivo a adiciona */
int isHigh() {
    int i;
    int pos = -1;
    int aux = score;
    for (i = 0; i < NSCORE; i++) {
        if (savedScore[i] < aux){
           aux = savedScore[i];
           pos = i;
        }
    }
    if (pos != -1){
           savedScore[pos] = score;
           strcpy(savedNickname[pos], newNickname) ;
           orderScore();
           return 1;
    }
    return 0;
}

/* Le a EEPROM para obter pontuacoes salvas e nomes de usuarios */
void readRom() {
     int i;
     for (i = 0; i < NSCORE; i++){
         savedScore[i] = EEPROM_Read(0xf0 + 2 * i);

         savedNickname[i][0] = EEPROM_Read(0xE0 + 3 * i );
         savedNickname[i][1] = EEPROM_Read(0xE0 + 3 * i + 1);
         savedNickname[i][2] = EEPROM_Read(0xE0 + 3 * i + 2);
         savedNickname[i][3] = '\0';
     }

}

/* Salva na EEPROM as maiores pontuacoes e os nomes de usuarios */
void saveRom() {
     int i;
     for (i = 0; i < NSCORE; i++){
         EEPROM_Write(0xf0 + 2 * i, savedScore[i]);
         EEPROM_Write(0xE0 + 3 * i , savedNickname[i][0]);
         EEPROM_Write(0xE0 + 3 * i + 1 , savedNickname[i][1]);
         EEPROM_Write(0xE0 + 3 * i + 2 , savedNickname[i][2]);
     }

}

/* Salva a nova pontuacao na EEPROM caso ela seja mais alta 
tambem fornece um nome de usuario aleatorio */
void saveScore() {
     int high;
     newNickname[0] = 'A' + (rand() % 26);
     newNickname[1] = 'A' + (rand() % 26);
     newNickname[2] = 'A' + (rand() % 26);
     newNickname[3] = '\0';
     
     high = isHigh();
     if (high){
        saveRom();
     }

}

/* Acao realizada ao fim do jogo */
void gameOver () {
     Glcd_Write_Text("GAME OVER",32,3,1);
     gameOverSound();
     mode = 0;
     Glcd_Fill(0);
     pipes[0] = 127;
     pos_y = 40;
     saveScore();
}

/* Checa se ha colisao do personagem com os limitantes de tela
ou com os obstaculos */
void checkColision() {
     if (pos_x + 3 >= pipes[0] && pos_x - 3 <= pipes[0]) {
        if(pos_y - 3 <= gapPos || pos_y + 3 >= gapPos + GAP){
              gameOver();
        } else {
               if(pos_x == pipes[0]){
                        makeScore();
                        scoreSound();
               }
        }
     } else 
     if (pos_y >= 63 - 3){
       pos_y = 63 - 3;
       vel = 0;
       gameOver();
       }else 
       if (pos_y <=  3){
             pos_y = 3;
               vel = 0;
               gameOver();
       }
}

/* Comeca uma nova partida */
void newGame() {
  mode = 1;
  Glcd_Fill(0);
}

/* Ouve acoes do usuario na tela menu */
void menuListener() {
     if (TP_Get_Coordinates(&x_coord, &y_coord) == 0)  {
          if(x_coord >= 20 && y_coord >= 24 && x_coord <= 50 && y_coord <= 32) {
                newGame();
          }
          else if(x_coord >= 20 && y_coord >= 34 && x_coord <= 60 && y_coord <= 40){
                   mode = 3;
                   readRom();
                   Glcd_Fill(0);
          }
           else if(x_coord >= 20 && y_coord >= 46 && x_coord <= 60 && y_coord <= 56){
                   mode = 4;
                   Glcd_Fill(0);
          }
          
     }

}

/* Ouve acoes do usario na tela creditos */
void creditsListener() {
      if (TP_Get_Coordinates(&x_coord, &y_coord) == 0)  {
           if(x_coord >= 80 && y_coord >= 57 && x_coord <= 110 && y_coord <= 70) {
                Glcd_Fill(0);
                mode = 0;
          }
     }
}

/* Ouve acoes do usuario na tela pontuacoes */
void scoreListener() {
     if (TP_Get_Coordinates(&x_coord, &y_coord) == 0)  {
          if(x_coord >= 80 && y_coord >= 57 && x_coord <= 110 && y_coord <= 70) {
                Glcd_Fill(0);
                mode = 0;
          }
     }
}

/* Indica a nota a ser tocada em cada iteracao */
void menuMusic() {
  if(musicAux > MUSICKEYS - 1) {
              musicAux = 0;
  }
   if (musicTime < 8*music[musicAux][1]){
    musicTime++;
    return;
 }else{
       musicTime = 0;
 }
  Sound_Play(music[musicAux][0], 11);
                 musicAux ++;
}

/* Acao tomada na primeira vez que o PIC é ligado (uma unica vez por programacao) */
void firstInitialization(){
	for (i = 0; i < NSCORE; i++){
          EEPROM_Write(0xf0 + 2 * i, 0x00);
          EEPROM_Write(0xf0 + 2 * i + 1 , 0x00);
      }
      EEPROM_Write(0xD0, 0x00);
}

/* Inicia variaveis de jogo e de ambiente */
void initializeVariables(){
   pos_x = 40;
   pos_y = 40;
   musicAux = 0;
   musicTime = 0;
  
   savedScore[0] = 0;
   savedScore[1] = 0;
   savedScore[2] = 0;
   savedScore[3] = 0;
   savedScore[4] = 0;
   
   strcpy(savedNickname[0], "");
   strcpy(savedNickname[1], "");
   strcpy(savedNickname[2], "");
   strcpy(savedNickname[3], "");
   strcpy(savedNickname[4], "");

   Sound_Init(&PORTE, 1);
	
	pipes[0] = 127;
	
    gapPos = 30;
    mode = 0;
    score = 0;
}

/* Programa principal */
void main() {
   int i, running;

   if (EEPROM_Read(0xD0) == 255){
		firstInitialization();
    }

   initializeVariables();

   Initialize();
     
   TP_Set_Calibration_Consts(60, 912, 132, 871);           

   Glcd_Fill(0);
	
	running = 1;
   
  while (running) {
        switch(mode) {
            case 1:
              drawPipes(erase);
              drawPipes(draw);
              drawBird(draw);
              if (TP_Press_Detect()) {
                 jump();
              }
              checkColision();
              Delay_ms(50);
              drawBird(erase);
              pos_y += vel;
              vel += acel;
              movePipes();
              break;
            case 0:
              Glcd_Write_Text("PIC BIRD", 20, 2, 1);
              Glcd_Write_Text("Play", 20, 3, 1);
              Glcd_Write_Text("Scores", 20, 4, 1);
              Glcd_Write_Text("Credits", 20, 5, 1) ;
              if (TP_Press_Detect()){
                 menuListener();
              }
              score = 0;
              menuMusic();
            break;
            case 3:
              Glcd_Write_Text("Scores", 40, 0, 1);
              Glcd_Write_Text("Back", 80, 7, 1);
              printAllScores();
              if (TP_Press_Detect()){
                scoreListener();
              }
              break;
			case 4:
              Glcd_Write_Text("Credits", 40, 0, 1);
              Glcd_Write_Text("Luis H Pegurin", 20, 3, 1);
              Glcd_Write_Text("Marcelo B Diani", 20, 4, 1);
              Glcd_Write_Text("Back", 80, 7, 1);
              if (TP_Press_Detect()){
                  creditsListener();
              }
              break;
        }
	}
}