#include <Arduino_FreeRTOS.h>
#include <Wire.h>
#include <IRremote.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>
#define STEPS 2048

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
Stepper stepper(STEPS, 8, 10, 9, 11);
int buzzer = 5;
int RECV_PIN = 6; 
int button = 2;
int button_int = 0;  // Interrupt 0 is on pin 2
IRrecv irrecv(RECV_PIN);
decode_results results;
void enterPassword( void *pvParameters );
void setPassword( void *pvParameters );
char password[20]="12345678", input[20]={'\0'}, ret;
int now=0, count=0, comeback=0, man=0, door=0;
TaskHandle_t set = NULL;
const int trigPin = 4;  const int echoPin = 3;
long duration, distance;
unsigned long now_time, start_time;

void setup()
{ 
  Serial.begin(9600);
  irrecv.enableIRIn(); 
  pinMode(button, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  attachInterrupt(button_int, handle_click, RISING);
  lcd.begin(16, 2);
  lcd.noBacklight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set password:");
  stepper.setSpeed(5);
  xTaskCreate(enterPassword, (const portCHAR *)"enterPassword", 128, NULL, 2, NULL);
  xTaskCreate(setPassword, (const portCHAR *)"setPassword", 128, NULL, 3, &set);
  vTaskSuspend(set);
}
void loop()
{ 
  
}

void setPassword( void *pvParameters )
{
  (void) pvParameters;
  int i;
  for (;;)
  {
    now = 0;
    count = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set password:");
    for(i=0; i<20; i++) password[i] = '\0';
    for(;;)
    {
      if (irrecv.decode(&results))
      {   
        translateIR();
        if(ret != 'X')
        {
          if(ret == '+')
          {
            if(now == 0)
            {
              lcd.setCursor(0, 1);
              lcd.print("No input!");
            }
            else
            {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Enter password:");
              break;
            }
          }
          else
          {
            if(now == 0)
            {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Set password:");
            }
            if(now < 16)
            {
              password[now] = ret;
              lcd.setCursor(now, 1);
              lcd.print(ret);
              now++;
            }
          }
        }
        delay(500);
        irrecv.resume();
      }
    }
    now = 0;
    comeback = 1;
    vTaskSuspend(set);
  }
}

void enterPassword( void *pvParameters )
{
  (void) pvParameters;
  int i;
  for (;;)
  {
    digitalWrite(trigPin, LOW); // Clears the trigPin
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = duration*0.034/2;
    if(distance < 50)
    {
      if(man==0)
      {
        lcd.clear();
        lcd.backlight();
        lcd.setCursor(0, 0);
        lcd.print("Enter password:");
        now = 0;
        count = 0;
        man = 1;
      }
      man = 1;
    }
    else
    {
      if(man==1)
      {
        man = 2;
        start_time = millis();
      }
      if(man==2)
      {
        now_time = millis();
        if(now_time - start_time > 5000)
        {
          man = 0;
          lcd.noBacklight();
          if(door == 1)
          {
            stepper.step(-512);
            door = 0;
          }
        }
      }
    }
    if (irrecv.decode(&results))
    {   
      translateIR();
      if(ret != 'X')
      {
        if(ret == '+')
        {
          if(comeback == 0)
          {
            if(strcmp(password, input) == 0)
            {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Pass!");
              stepper.step(512);
              door = 1;
              count = 0;
            }
            else
            {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Incorrect!");
              lcd.setCursor(0, 1);
              lcd.print("Please try again");
              count++;
              if(count >= 5)
              {
                lcd.noBacklight();
                for(i=4; i>0; i--)
                {
                  tone(buzzer, 500, 500);
                  delay(1000);
                }
                delay(6000);
                lcd.backlight();
                count = 0;
              }
            }
            for(i=0; i<20; i++) input[i] = '\0';
            now = 0;
          }
        }
        else
        {
          if(now == 0)
          {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Enter password:");
          }
          if(now < 16)
          {
            input[now] = ret;
            lcd.setCursor(now, 1);
            lcd.print(ret);
            now++;
          }
        }
      }
      comeback = 0;
      delay(500);
      irrecv.resume();
   }
  }
}

void translateIR() // takes action based on IR code received describing Car MP3 IR codes 
{
  switch(results.value){
  case 0xFFA857:  
    ret = '+';
    break;
  case 0xA3C8EDDB:  
    ret = '+';
    break;
  case 0xFF6897:  
    ret = '0';
    break;
  case 0xC101E57B:  
    ret = '0';
    break;
  case 0xFF30CF:  
    ret = '1';
    break;
  case 0x9716BE3F:  
    ret = '1';
    break;
  case 0xFF18E7:  
    ret = '2';
    break;
  case 0x3D9AE3F7:  
    ret = '2';
    break;
  case 0xFF7A85:  
    ret = '3';
    break;
  case 0x6182021B:  
    ret = '3';
    break;
  case 0xFF10EF:  
    ret = '4';
    break;
  case 0x8C22657B:  
    ret = '4';
    break;
  case 0xFF38C7:  
    ret = '5';
    break;
  case 0x488F3CBB:  
    ret = '5';
    break;
  case 0xFF5AA5:  
    ret = '6';
    break;
  case 0x449E79F:  
    ret = '6';
    break;
  case 0xFF42BD:  
    ret = '7';
    break;
  case 0x32C6FDF7:  
    ret = '7';
    break;
  case 0xFF4AB5:  
    ret = '8';
    break;
  case 0x1BC0157B:  
    ret = '8';
    break;
  case 0xFF52AD:  
    ret = '9';
    break;
  case 0x3EC3FC1B:  
    ret = '9';
    break;
    case 0xFFFFFFFF:  
    ret = 'X';
    break;
  case 0xFF:  
    ret = 'X';
    break;
  default: 
    ret = 'X';
  }
} 

void handle_click()
{
  vTaskResume(set);
}
