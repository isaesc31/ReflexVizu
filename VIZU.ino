
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


#define NUM_BOTOES 5



LiquidCrystal_I2C lcd(0x27, 16, 2);


int leds[NUM_BOTOES]   = {22, 23, 24, 25, 26};
int botoes[NUM_BOTOES] = {48, 49, 50, 51, 52};


int ledAtual;
int rodada = 1;
unsigned long tempoEspera = 2000;
unsigned long tempoMinimo = 500;
unsigned long reducao = 500;
int ordem[NUM_BOTOES];


void lcdRodada() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Rodada: ");
  lcd.print(rodada);
  lcd.setCursor(0, 1);
}


void lcdAguarda(int ledNum) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.setCursor(0, 1);
  lcd.print("Aguardando...");
}


void lcdErro(int errado, int correto) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ERRO! ");
  lcd.setCursor(0, 1);
}


void lcdReacao(unsigned long ms) {
  unsigned long segundos   = ms / 1000;
  unsigned long resto_ms   = ms % 1000;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reacao: ");
  lcd.print(segundos);
  lcd.print("s ");
  lcd.print(resto_ms);
  lcd.print("ms");

}


void lcdFimRodada() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("FIM DA RODADA");
  lcd.setCursor(0, 1);
  lcd.print("Rodada ");
  lcd.print(rodada);
  lcd.print(" OK!");
}


// ---------- embaralhar ----------


void embaralhar(int arr[], int n) {
  for (int i = n - 1; i > 0; i--) {
    int j = random(0, i + 1);
    int temp = arr[i];
    arr[i]   = arr[j];
    arr[j]   = temp;
  }
}


// ---------- setup ----------


void setup() {
  Serial.begin(9600);


  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("JOGO DE REACAO");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");


  for (int i = 0; i < NUM_BOTOES; i++) {
    pinMode(leds[i],   OUTPUT);
    pinMode(botoes[i], INPUT_PULLUP);
    digitalWrite(leds[i], LOW);
  }


  randomSeed(analogRead(A7));
  Serial.println("=== JOGO DE REACAO ===");
  delay(2000);
}


// ---------- loop ----------


void loop() {
  lcdRodada();
  Serial.println("--------------------------------");
  Serial.print("RODADA: ");      Serial.println(rodada);
  Serial.print("Tempo de espera: "); Serial.print(tempoEspera); Serial.println(" ms");
  delay(2000);


  for (int i = 0; i < NUM_BOTOES; i++) ordem[i] = i;
  embaralhar(ordem, NUM_BOTOES);


  for (int contador = 0; contador < NUM_BOTOES; contador++) {
    delay(tempoEspera);


    ledAtual = ordem[contador];
    digitalWrite(leds[ledAtual], HIGH);


    lcdAguarda(ledAtual + 1);


    // ---- aguarda pressão com detecção de botão errado ----
    unsigned long inicio = millis();
    bool erroOcorreu = false;


    while (true) {
      // Timeout opcional de 10s para não travar para sempre
      if (millis() - inicio > 10000UL) break;


      for (int b = 0; b < NUM_BOTOES; b++) {
        if (digitalRead(botoes[b]) == LOW) {          // algum botão pressionado
          if (b == ledAtual) {
            // Botão CORRETO
            goto botaoCorreto;
          } else {
            // Botão ERRADO
            if (!erroOcorreu) {
              erroOcorreu = true;
            }
            lcdErro(b + 1, ledAtual + 1);
            Serial.print("ERRO! Botao ");
         

            // Aguarda soltar o botão errado antes de continuar
            while (digitalRead(botoes[b]) == LOW) {}
            delay(50); // debounce


            // Volta a mostrar a mensagem de aguardo
            lcdAguarda(ledAtual + 1);
          }
        }
      }
    }


    botaoCorreto:
    unsigned long fim = millis();
    digitalWrite(leds[ledAtual], LOW);


    // Aguarda soltar o botão correto
    while (digitalRead(botoes[ledAtual]) == LOW) {}
    delay(50); // debounce


    unsigned long reacao = fim - inicio;
    lcdReacao(reacao);


    Serial.print("LED ");
    Serial.print(ledAtual + 1);
    Serial.print(" -> Reacao: ");
    Serial.print(reacao);
    Serial.print(" ms  (");
    if (erroOcorreu) Serial.println("  [erro cometido nesta etapa]");


    delay(1500); // tempo para ler o resultado na tela
  }


  lcdFimRodada();
  Serial.println("FIM DA RODADA");


  if (tempoEspera > tempoMinimo + reducao) {
    tempoEspera -= reducao;
  } else {
    tempoEspera = tempoMinimo;
  }

  rodada++;
  delay(2000);
}




