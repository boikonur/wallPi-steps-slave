/*
  slave
*/

#include <Arduino.h>
#include <Chrono.h>

Chrono gameTimer(Chrono::SECONDS ); //2min timer
Chrono stepTimer(Chrono::MILLIS ); //1000 ms timer

#define NUM_STEPS 6
#define WRITE_EN_PIN 12
#define DIST_THRESH 200

//SYNC PINS
#define SYNC_OUTPUT_PIN 8
#define SYNC_INPUT_PIN 9

//BUTTON PINS
#define BUTTON_PIN 10
#define BUTTON_LED_PIN 11

//LEDS
#define STEP_LED_PIN1 2
#define STEP_LED_PIN2 3
#define STEP_LED_PIN3 4
#define STEP_LED_PIN4 5
#define STEP_LED_PIN5 6
#define STEP_LED_PIN6 7

//SENSOR PINS
#define STEP_SENSOR_PIN1 A0
#define STEP_SENSOR_PIN2 A1
#define STEP_SENSOR_PIN3 A2
#define STEP_SENSOR_PIN4 A3
#define STEP_SENSOR_PIN5 A4
#define STEP_SENSOR_PIN6 A5

boolean enabled_game = false;
int steps_stage = 0;
int step_index = 0;
int correctSteps = 0;
int stepAttempts = 0;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(SYNC_INPUT_PIN, INPUT);

  pinMode(BUTTON_LED_PIN, OUTPUT);
  pinMode(SYNC_OUTPUT_PIN, OUTPUT);
  pinMode(STEP_LED_PIN1, OUTPUT);
  pinMode(STEP_LED_PIN2, OUTPUT);
  pinMode(STEP_LED_PIN3, OUTPUT);
  pinMode(STEP_LED_PIN4, OUTPUT);
  pinMode(STEP_LED_PIN5, OUTPUT);
  pinMode(STEP_LED_PIN6, OUTPUT);

  digitalWrite(BUTTON_LED_PIN, HIGH);
  digitalWrite(SYNC_OUTPUT_PIN, LOW);

  digitalWrite(STEP_LED_PIN1, LOW);
  digitalWrite(STEP_LED_PIN2, LOW);
  digitalWrite(STEP_LED_PIN3, LOW);
  digitalWrite(STEP_LED_PIN4, LOW);
  digitalWrite(STEP_LED_PIN5, LOW);
  digitalWrite(STEP_LED_PIN6, LOW);

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("INIT");
}

void loop() {

//testStepSensors();
//testStepLEDS();  
  if ( enabled_game == false)
  {
    if (digitalRead(BUTTON_PIN) == LOW)
    {
      delay(100);
      if (digitalRead(BUTTON_PIN) == LOW)
      {

        Serial.println("Button pressed");
        digitalWrite(BUTTON_LED_PIN, LOW);
        digitalWrite(SYNC_OUTPUT_PIN, HIGH);

        if (digitalRead(SYNC_INPUT_PIN) == HIGH)
        { delay(100);
          if (digitalRead(SYNC_INPUT_PIN) == HIGH)
          {
            digitalWrite(BUTTON_LED_PIN, LOW);
            enabled_game = true;
            steps_stage = 0;
          }
        }
        else
          digitalWrite(BUTTON_LED_PIN, HIGH);
      }
    }
  }

  if ( enabled_game == true)
  {
    if (stepsGame())
    {
      Serial.println("END GAME after 10s");
      digitalWrite(SYNC_OUTPUT_PIN, LOW);
      enabled_game = false;
      endBlink();
    }

  }
}


void endBlink() {
  for (int i = 0; i < 5 ; i++)
  {
    digitalWrite(BUTTON_LED_PIN, HIGH);
    delay(100);
    digitalWrite(BUTTON_LED_PIN, LOW);
    delay(100);
  }
  digitalWrite(BUTTON_LED_PIN, HIGH);
}

int stepsGame() {

  switch (steps_stage) {

    case 0:
      step_index = 0;

      for (int i = 0; i < NUM_STEPS ; i++)
      {
        offStepLed(i);
      }
      steps_stage = 1;
      break;

    case 1:
      if (digitalRead(SYNC_INPUT_PIN) == HIGH)
      {
        delay(100);
        if (digitalRead(SYNC_INPUT_PIN) == HIGH)
        {
          Serial.println("game synced with master");
          gameTimer.restart();
          stepTimer.restart();

          steps_stage = 2;
          digitalWrite(SYNC_OUTPUT_PIN, LOW);
        }
      }
      break;
    case 2:

      onStepLed(step_index);
      steps_stage = 3;
      break;

    case 3:

      if (gameTimer.hasPassed(120))
      {
        steps_stage = 4;
        break;
      }

      if (stepTimer.hasPassed(1000)) //change step on 1s
      {
        offStepLed(step_index);

        step_index++;
        onStepLed(step_index);
        stepAttempts++;
        if (step_index > NUM_STEPS)
          step_index = 0;
        stepTimer.restart();

        steps_stage = 2;
      }
      else

        if (readStep(step_index))
        {
          if (readStep(step_index))
          {
            digitalWrite(SYNC_OUTPUT_PIN, HIGH);
            stepBlink(step_index);
            digitalWrite(SYNC_OUTPUT_PIN, LOW);
            offStepLed(step_index);
            //targetTimer.restart();
            //Serial.println("hit");

            step_index++;

            if (step_index > NUM_STEPS)
              step_index = 0;
            correctSteps++;
          }
        }
      break;
    case 4:

      for (int i = 0; i < NUM_STEPS; i++)
      {
        offStepLed(i);
      }

      digitalWrite(SYNC_OUTPUT_PIN, LOW);
      return 1;
      break;
  }
  return 0;

}


bool readStep(int step_num)
{
  switch (step_num)
  {
    case 0: if (analogRead(STEP_SENSOR_PIN1) < DIST_THRESH) return 1;  break;
    case 1: if (analogRead(STEP_SENSOR_PIN2) < DIST_THRESH) return 1;  break;
    case 2: if (analogRead(STEP_SENSOR_PIN3) < DIST_THRESH) return 1;  break;
    case 3: if (analogRead(STEP_SENSOR_PIN4) < DIST_THRESH) return 1; break;
    case 4: if (analogRead(STEP_SENSOR_PIN5) < DIST_THRESH) return 1; break;
    case 5: if (analogRead(STEP_SENSOR_PIN6) < DIST_THRESH) return 1;  break;
    default: break;
  }
  return 0;
}

void onStepLed(int step_num)
{
  switch (step_num)
  {
    case 5:  digitalWrite(STEP_LED_PIN1, HIGH); break;
    case 0:  digitalWrite(STEP_LED_PIN2, HIGH); break;
    case 2:  digitalWrite(STEP_LED_PIN3, HIGH); break;
    case 1:  digitalWrite(STEP_LED_PIN4, HIGH); break;
    case 3:  digitalWrite(STEP_LED_PIN5, HIGH); break;
    case 4:  digitalWrite(STEP_LED_PIN6, HIGH); break;
    default: break;
  }
}

void offStepLed(int step_num)
{
  switch (step_num)
  {
    case 5:  digitalWrite(STEP_LED_PIN1, LOW); break;
    case 0:  digitalWrite(STEP_LED_PIN2, LOW); break;
    case 2:  digitalWrite(STEP_LED_PIN3, LOW); break;
    case 1:  digitalWrite(STEP_LED_PIN4, LOW); break;
    case 3:  digitalWrite(STEP_LED_PIN5, LOW); break;
    case 4:  digitalWrite(STEP_LED_PIN6, LOW); break;
    default: break;
  }
}

void stepBlink(int num ) {
  for (int i = 0; i < 2; i++)
  {
    onStepLed(num);
    delay(100);
    //Serial.println("hit");
    offStepLed(num);
    delay(50);
  }
}



void testStepLEDS() {
  while (1) {
    for (int i = 0; i < 6; i++) {
      Serial.println(i);
      onStepLed(i);
      delay(2000);
      offStepLed(i);
      delay(1000);
    }
  }
}


void testStepSensors() {
  while (1) {
    for (int i = 0; i < 6; i++) {
     
      boolean at= readStep(i);
      if(at)
      {
         Serial.println(i);
         delay(200);
     
      }
      
    }
    delay(100);
  }
}
