#include <SoftwareSerial.h> // Biblioteca para a comunicação serial
#include <Sim800l.h>  // Biblioteca específica para o Módulo GSM SIM800L by Cristian Steib
#include <LiquidCrystal.h> // Biblioteca da tela LCD

Sim800l Sim800l; // Definindo o dispositivo para a biblioteca

LiquidCrystal lcd(8, 9, 10, 11, 12, 13); // Portas usadas pelo LCD    
  
#define LED_VERDE 7
#define LED_AMARELO 6
#define PERIGO 5
#define S_FOGAO A0
#define BOTAO 4
#define MAX_VERDE 100
#define MAX_AMARELO 150

char numero[] = "+5583981824163"; // Número do dono
char mensagem[] = "Vazamento de gas na sua residencia"; // Mensagem enviada no vazamento

void LeBotao(bool *desligado){ // Leitura do botao de Desligar/Ligar
  if(digitalRead(BOTAO) == HIGH){ 
    if(!*desligado){
      *desligado = 1;
      delay(1000);
      }else{
      *desligado = 0;
      delay(1000);  
      }
    }
  }

void MostrarLigado(){ // Informa no LCD que está ligado 
  lcd.setCursor(0,0);
  lcd.print("Contravazamento");
  lcd.setCursor(5, 1);
  lcd.print("LIGADO");
  }

void MostrarDesligado(){ // Informa no LCD que está desligado
  lcd.setCursor(0,0);
  lcd.print("Contravazamento");
  lcd.setCursor(4, 1);
  lcd.print("DESLIGADO");
  }

void MostrarAviso(){ // Informa no LCD que está Mandando o SMS
  lcd.setCursor(0,0);
  lcd.print("Vazamento detec.");
  lcd.setCursor(3, 1);
  lcd.print("Avisando...");
  }

void setup() { 
  Serial.begin(9600); // Inicializando o monitor serial para debug

  Sim800l.begin(); // Inicializando o Módulo GSM SIM800L
  
  lcd.begin(16, 2); // Inicializando a Tela LCD
  lcd.clear();

  pinMode(S_FOGAO, INPUT); // Sensor de gás

  // Leds de situação
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARELO, OUTPUT);
  pinMode(PERIGO, OUTPUT);

  pinMode(BOTAO, INPUT); // Power Off/On

  delay(10000); // Tempo para Inicialização do Módulo GSM 
}
void loop() {
  static bool powerOff = 0; // 0 -> Ligado. 1 -> Desligado  
  int concentracao; 

  LeBotao(&powerOff);
  
  concentracao = analogRead(S_FOGAO);
  //Serial.println(concentracao); 
  //Serial.print("Desligado: ");
  //Serial.println(powerOff);

  if (!powerOff){ // Sistema ligado
    digitalWrite(LED_VERDE, concentracao <= MAX_VERDE ? HIGH : LOW);
    digitalWrite(LED_AMARELO, concentracao > MAX_VERDE && concentracao <= MAX_AMARELO ? HIGH : LOW);
  
    if (concentracao > MAX_AMARELO){ // Estado de perigo
      
      digitalWrite(PERIGO, HIGH);
      MostrarAviso();
      
      for(int count = 1; concentracao > MAX_AMARELO && !powerOff; count++){ // Loop de envio de mensagens
        concentracao = analogRead(S_FOGAO);
        LeBotao(&powerOff);
        //Serial.println(concentracao);
        if(count % 10 == 0 || count == 1){ // A cada 10 segundos uma mensagem é enviada
          //Serial.println("Enviando Mensagem...");
          Sim800l.sendSms(numero, mensagem);  
        }
        delay(1000);
      }         
    }else{
      digitalWrite(PERIGO, LOW);
      MostrarLigado();
    }
  }else{ // Sistema desligado
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_AMARELO, LOW);
    digitalWrite(PERIGO, LOW);
    MostrarDesligado();
  }
     
  delay(1000);
  lcd.clear(); 
}
