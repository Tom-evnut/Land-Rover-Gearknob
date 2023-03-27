// CAN Receive Example
//

#include <mcp_can.h>
#include <SPI.h>

long unsigned int rxId;
uint32_t relayid = 0;
uint8_t relayidb [4];
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];                        // Array to store serial string

uint8_t Dir = 1;

#define Park 0
#define Reverse 1
#define Neutral 2
#define Drive 3
#define Sport 7

uint8_t KnobPos, KnobPosDes = 0;

#define Lower 0
#define Raise 1

uint8_t Knoblock = 0;

#define Unlocked 0x40
#define Locked 0x00

bool routine = 0;
unsigned long looptime = 0;
uint16_t chargevoltage = 3950;
uint16_t chargecurrent = 200;
bool candebug = 0;
uint16_t Cnt02C = 0;
uint8_t Cnt0E0;

#define CAN0_INT 2                              // Set INT to pin 2
MCP_CAN CAN0(9);                               // Set CS to pin 9 de[ending on shield used

char mes[8] = {0, 0, 0, 0, 0, 0, 0, 0};

char byte4[16] = {0, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0};
char byte5P[16] = {0x21, 0xBC, 0x06, 0x9B, 0x6F, 0xF2, 0x48, 0xD5, 0xBD, 0x20, 0x9A, 0x07, 0x0F3, 0x6E, 0xD4, 0x49};
char byte5R[16] = {0x67, 0xFA, 0x40, 0xDD, 0x29, 0xB4, 0x0E, 0x93, 0xFB, 0x66, 0xDC, 0x41, 0xB5, 0x28, 0x92, 0x0F};
char byte5N[16] = {0xFB, 0x76, 0xCC, 0x51, 0xA5, 0x38, 0x82, 0x1F, 0x77, 0xEA, 0x52, 0xCD, 0x39, 0xA4, 0x1E, 0x83};
char byte5D[16] = {0xE8, 0x75, 0xCF, 0x52, 0xA6, 0x3B, 0x81, 0x1C, 0x74, 0xE9, 0x53, 0xCE, 0x3A, 0xA7, 0x1D, 0x80};
char byte5S[16] = {0xE2, 0x7F, 0xC5, 0x58, 0xAC, 0x31, 0x8B, 0x16, 0x7E, 0xE3, 0x59, 0xC4, 0x30, 0xAD, 0x17, 0x8A};



void setup()
{
  Serial.begin(115200);

  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  while (CAN_OK != CAN0.begin(CAN_500KBPS, MCP_16MHZ))    // init can bus : baudrate = 500k  8MHZ crystal
  {
    Serial.println("CAN BUS FAIL!");
    delay(100);
  }
  Serial.println("CAN BUS OK!");
  Serial.println("Time Stamp,ID,Extended,LEN,D1,D2,D3,D4,D5,D6,D7,D8");
}

void loop()
{
  if (CAN0.checkReceive() == 3)                        // If CAN0_INT pin is low, read receive buffer
  {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
    if (rxId == 0x0E0)
    {
      Cnt0E0 = rxBuf[4];
      Dir = rxBuf[4] >> 4;
    }

    if (candebug == 1)
    {

      //if (rxId > 0x079 && rxId < 0x090)
      // {
      Serial.print(millis());
      if ((rxId & 0x80000000) == 0x80000000)    // Determine if ID is standard (11 bits) or extended (29 bits)
        sprintf(msgString, ",0x%.8lX,true, %1d", (rxId & 0x1FFFFFFF), len);
      else
        sprintf(msgString, ",0x%.3lX,false,%1d", rxId, len);

      Serial.print(msgString);

      if ((rxId & 0x40000000) == 0x40000000)
      { // Determine if message is a remote request frame.
        sprintf(msgString, " REMOTE REQUEST FRAME");
        Serial.print(msgString);
      } else
      {
        for (byte i = 0; i < len; i++) {
          sprintf(msgString, ", 0x%.2X", rxBuf[i]);
          Serial.print(msgString);
        }
      }
      Serial.println();
      //}

      /*
          if (rxId == 0x080)
          {
            Serial.println();
            sendcan();
            Serial.println("command recieved");
          }
      */
    }
  }

  if (Serial.available() > 0)
  {
    Serialcomms();
  }

  if (millis() - looptime > 10)
  {
    looptime = millis();

    sendcan();
    Cnt02C ++;
    if (Cnt02C > 0xF)
    {
      Cnt02C = 0x00;
    }
  }

}


void sendcan()
{
  uint16_t id = 0x02C;
  mes[0] = 0x00; //0x1E;
  mes[1] = 0x00;
  /*
    if (KnobPosDes == KnobPos)
    {
      //mes[2] = 0x2F;//Movement in here
      mes[2] = 0x00;
    }
    else
    {
  */
  if (KnobPosDes == Raise)
  {
    mes[2] = 0x09;
  }
  if (KnobPosDes == Lower)
  {
    mes[2] = 0x10;
  }
  /*
    KnobPos = KnobPosDes;
    }
  */

  if (Dir == Park)
  {
    mes[3] = Knoblock | 0x12;
    mes[4] = byte4[Cnt02C];
    mes[5] = byte5P[Cnt02C];
    mes[6] = 0x00;
    mes[7] = 0x80;
  }
  if (Dir == Reverse)
  {
    mes[3] = Knoblock | 0x00;
    mes[4] = byte4[Cnt02C];
    mes[5] = byte5R[Cnt02C];
    mes[6] = 0x01;
    mes[7] = 0x00;
  }

  if (Dir == Neutral)
  {
    mes[3] = Knoblock | 0x01;
    mes[4] = byte4[Cnt02C];
    mes[5] = byte5N[Cnt02C];
    mes[6] = 0x02;
    mes[7] = 0x00;
  }

  if (Dir == Drive)
  {
    mes[3] = Knoblock | 0x04;
    mes[4] = byte4[Cnt02C] | 0x0B;
    mes[5] = byte5D[Cnt02C];
    mes[6] = 0x04;
    mes[7] = 0x00;
  }

  if (Dir == Sport)
  {
    mes[3] = Knoblock | 0x04;
    mes[4] = byte4[Cnt02C] | 0x0B;
    mes[5] = byte5S[Cnt02C];
    mes[6] = 0x08;
    mes[7] = 0x00;
  }

  CAN0.sendMsgBuf(id, 0, 8, mes);

}

void RaiseKnob()
{
  Knoblock = Unlocked;
      KnobPosDes = Raise;
}

void LowerKnob()
{
  KnobPosDes = Lower;
}

void Serialcomms()
{
  byte incomingByte = Serial.read();
  switch (incomingByte)
  {
    case 's': //Start
      RaiseKnob();
      break;

    case 'q': //Stop
      LowerKnob();
      break;

    case 'r':
      KnobPosDes = 1;
      break;

    case 'e':
      KnobPosDes = 0;
      break;

    case 'w':
      if (Knoblock == Locked)
      {
        Knoblock = Unlocked;
      }
      else
      {
        Knoblock = Locked;
      }

    default:
      // if nothing else matches, do the default
      // default is optional
      break;
  }
}


/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
