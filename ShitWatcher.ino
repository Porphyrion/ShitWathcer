#include "Pager.h"
#include "InputPinArray.h"


//#define DEBUG_LOGIC true


enum Status
{
  Init, 
  Starting,
  Good,
  Bad
};

Status status = Status::Init;
 
unsigned long GoodTimerEvent  = 7200000;
unsigned long BadTimerEvent   = 0;

const unsigned long DayDelta       = 7200000;
const unsigned long SixHourstDelta = 3600000;


InputPinArray pins;
Pager pager;

void setup(){
  pins.addPin(4, "KNS1");
  pins.addPin(7, "KNS2");
  pins.initPins();

  #ifdef DEBUG_SMS
    Serial.begin(9600);
  #endif

  gsm.begin(9600);
  
  pager.phoneBook.addNumber("+79636556042");
  pager.phoneBook.addNumber("+79265527150");
  pins.pager = &pager;
}


void timerEventInit()
{
#ifdef DEBUG_LOGIC
  Serial.println("Init event");
#endif

  if(simInit())
  {
    status = Status::Starting;
    delay(4000);
    pager.sendAllSms(F("Control system restarted")); 
  }
}


void timerEventStarting()
{ 
#ifdef DEBUG_LOGIC
  Serial.println("Starting event");
#endif
  if(pins.checkPins())
  {
    status = Status::Good;
    pager.sendAllSms(F("KNS1,KNS2 are working normally"));
  }
  else
  {
    status = Status::Bad;
    pager.sendAllSms(F("Check kns control cabinet for problems"));  
    BadTimerEvent = millis() + SixHourstDelta;
    GoodTimerEvent = 0;
    return;
  }
}


void timerEventGood()
{
#ifdef DEBUG_LOGIC
  Serial.println("Good event");
#endif
  if(!pins.checkPins())
  {
    status = Status::Bad;
    BadTimerEvent = millis() + SixHourstDelta;
    GoodTimerEvent = 0;
    return;
  }
  
  if(millis() > GoodTimerEvent)
  {
     pager.sendAllSms(F("KNS1,KNS2 are working normally"));  
     GoodTimerEvent = millis() + DayDelta;
  }
}


void timerEventBad()
{
#ifdef DEBUG_LOGIC
  Serial.println("Bad event");
#endif
  if(pins.checkPins())
  {
    status = Status::Good;
    GoodTimerEvent = millis() + DayDelta;
    pager.sendAllSms("Control system returned to work");
    return;
  }

  if(millis() > BadTimerEvent)
  {    
     pager.sendAllSms(F("Check kns control cabinet for problems"));  
     BadTimerEvent = millis() + SixHourstDelta;
  }
}


void loop() {
  switch(status)
  {
    case Status::Init:
      timerEventInit();
      break;
    case Status::Starting:
      timerEventStarting();
      break;
    case Status::Good:
      timerEventGood();
      break;
    case Status::Bad:
      timerEventBad();
      break;
     default:
      break;
  }
};
