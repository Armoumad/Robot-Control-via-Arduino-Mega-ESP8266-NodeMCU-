//code2   exemple de communication uart entre arduino mega et nodemcu v1
#include <Arduino_FreeRTOS.h>

//---------------structure ----------------------------------------
typedef struct _vFlag
{
  uint8_t LEDFlag = 1;
  uint8_t SendFlag = 0;
} vFlag;
vFlag flag;

//----------uart--------------
#define LINE_BUFFER_LENGTH 64

typedef struct _vUart
{
  char c;
  int lineIndex = 0;
  bool lineIsComment; 
  bool lineSemiColon;
  char line[64];
  String inputString;
  int Buffer[64];
} vUart;
vUart Uart;

const uint8_t LED_PIN = 13;
TaskHandle_t hled;
TaskHandle_t huart0;

// Déclaration préalable des fonctions
void processCommand(char *data);
void vLEDFlashTask(void *pvParameters);
void vUARTTask(void *pvParameters);

void setup()
{
  Uart.inputString.reserve(60);
  Serial.begin(9600);    // Communication avec le PC
  Serial1.begin(115200); // Communication avec NodeMCU
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Création des tâches
  xTaskCreate(vUARTTask, "UART Task", configMINIMAL_STACK_SIZE, NULL, 1, &huart0);
  xTaskCreate(vLEDFlashTask, "LED Task", configMINIMAL_STACK_SIZE, NULL, 2, &hled);
  
  Serial.println("System Started!");
  vTaskStartScheduler();
}

void loop()
{
  // Ne rien mettre ici car FreeRTOS gère les tâches
}

void vLEDFlashTask(void *pvParameters)
{
  pinMode(LED_PIN, OUTPUT);
  for (;;)
  {
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay((150L * configTICK_RATE_HZ) / 1000L);
    digitalWrite(LED_PIN, LOW);
    vTaskDelay((150L * configTICK_RATE_HZ) / 1000L);
  }
}

void vUARTTask(void *pvParameters)
{
  Uart.lineIsComment = false;
  Uart.lineSemiColon = false;

  while(1)
  {
    // Lecture des données depuis le PC
    while (Serial.available() > 0)
    {
      Uart.c = Serial.read();

      if ((Uart.c == '\n') || (Uart.c == '\r'))
      {
        if (Uart.lineIndex > 0)
        {
          Uart.line[Uart.lineIndex] = '\0';
          
          // Envoie au NodeMCU
          Serial1.write(Uart.line);
          Serial1.write("\r\n");

          processCommand(Uart.line);
          
          Uart.lineIndex = 0;
          Uart.inputString = "";
        }
        Uart.lineIsComment = false;
        Uart.lineSemiColon = false;
        Serial.println(F("ok>"));
      }
      else
      {
        if (Uart.lineIndex >= LINE_BUFFER_LENGTH - 1)
        {
          Serial.println("ERROR - Buffer overflow");
          Uart.lineIsComment = false;
          Uart.lineSemiColon = false;
        }
        else if (Uart.c >= 'a' && Uart.c <= 'z')
        {
          Uart.line[Uart.lineIndex] = Uart.c - 'a' + 'A';
          Uart.lineIndex++;
          Uart.inputString += (char)(Uart.c - 'a' + 'A');
        }
        else
        {
          Uart.line[Uart.lineIndex] = Uart.c;
          Uart.lineIndex++;
          Uart.inputString += Uart.c;
        }
      }
    }

    // Lecture des données depuis NodeMCU
    while (Serial1.available() > 0)
    {
      Uart.c = Serial1.read();
      
      if ((Uart.c == '\n') || (Uart.c == '\r'))
      {
        if (Uart.lineIndex > 0)
        {
          Uart.line[Uart.lineIndex] = '\0';
          processCommand(Uart.line);
          Serial.println(Uart.line);
          
          Uart.lineIndex = 0;
          Uart.inputString = "";
        }
        Uart.lineIsComment = false;
        Uart.lineSemiColon = false;
      }
      else
      {
        if (Uart.lineIndex >= LINE_BUFFER_LENGTH - 1)
        {
          Serial.println("ERROR - Buffer overflow");
          Uart.lineIsComment = false;
          Uart.lineSemiColon = false;
        }
        else if (Uart.c >= 'a' && Uart.c <= 'z')
        {
          Uart.line[Uart.lineIndex] = Uart.c - 'a' + 'A';
          Uart.lineIndex++;
          Uart.inputString += (char)(Uart.c - 'a' + 'A');
        }
        else
        {
          Uart.line[Uart.lineIndex] = Uart.c;
          Uart.lineIndex++;
          Uart.inputString += Uart.c;
        }
      }
    }
  }
}

void processCommand(char *data)
{
  if (strstr(data, "VER") != NULL)
  {
    Serial.println(F("MEGA_UART_V1.0"));
  }
}