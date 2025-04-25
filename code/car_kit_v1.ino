#include <Bluepad32.h>

#define r_pwm 33
#define l_pwm 32
#define RF 13
#define LF 26
#define RB 12
#define LB 27
#define trigP 5
#define echoP 18
#define light 19
#define SOUND_SPEED 0.034

int incoming;
int speed = 200;
long duration;
float distanceCm;
bool lightIsTurnOn = false;

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedController(ControllerPtr ctl) {
    bool foundEmptySlot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
            // Additionally, you can get certain gamepad properties like:
            // Model, VID, PID, BTAddr, flags, etc.
            ControllerProperties properties = ctl->getProperties();
            Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id,
                           properties.product_id);
            myControllers[i] = ctl;
            foundEmptySlot = true;
            break;
        }
    }
    if (!foundEmptySlot) {
        Serial.println("CALLBACK: Controller connected, but could not found empty slot");
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    bool foundController = false;

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
            myControllers[i] = nullptr;
            foundController = true;
            break;
        }
    }

    if (!foundController) {
        Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
    }
}


void processGamepad(ControllerPtr ctl) {
    // There are different ways to query whether a button is pressed.
    // By query each button individually:
    //  a(), b(), x(), y(), l1(), etc...
    if (ctl->a()) {
      Serial.println("BUtton A pressed");
    }

    if (ctl->b()) {
      Serial.println("BUTTON B pressed");
    }

    if (ctl->x()) {
     Serial.println("Button X pressed");      
    }
  
    if(ctl->dpad()  == 0X01) {
       Serial.println("Button UP pressed");
    }

     if(ctl->dpad() == 0X02) {
       Serial.println("Button DOWN pressed");
    }

     if(ctl->dpad() == 0X04) {
       Serial.println("Button RIGHT pressed");
    }

     if(ctl->dpad() == 0X08) {
       Serial.println("Button LEFT pressed");
    }
}

void processControllers() {
    for (auto myController : myControllers) {
        if (myController && myController->isConnected() && myController->hasData()) {
            if (myController->isGamepad()) {
                processGamepad(myController);
            } else {
                Serial.println("Unsupported controller");
            }
        }
    }
}


// Arduino setup function. Runs in CPU 1
void setupBL() {
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    // Setup the Bluepad32 callbacks
    BP32.setup(&onConnectedController, &onDisconnectedController);
}

// Arduino loop function. Runs in CPU 1.
void loopBL() {
    // This call fetches all the controllers' data.
    // Call this function in your main loop.
    bool dataUpdated = BP32.update();
    if (dataUpdated)
        processControllers();

    delay(150);
}

void stopC(){
  digitalWrite(RF, LOW);
  digitalWrite(LB, LOW);
  digitalWrite(LF, LOW);
  digitalWrite(RB, LOW);                     
}

float calDistance(){
  digitalWrite(trigP, LOW);
  delayMicroseconds(2);
  digitalWrite(trigP, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigP, LOW);
  
  duration = pulseIn(echoP, HIGH);
  distanceCm = duration * SOUND_SPEED/2;
  return distanceCm;
}

void setup(){
  Serial.begin(115200);

  setupBL();

  pinMode(RF, OUTPUT);
  pinMode(LF, OUTPUT);
  pinMode(LB, OUTPUT);
  pinMode(RB, OUTPUT);
  digitalWrite(RF, LOW);
  digitalWrite(LB, LOW);
  digitalWrite(LF, LOW);
  digitalWrite(RB, LOW);    

  pinMode(trigP, OUTPUT);
  pinMode(echoP, INPUT);
  pinMode(light, OUTPUT);
}

void loop(){
  loopBL();
  
  ControllerPtr ctl = myControllers[0];
  if(ctl != nullptr){

     float dis = calDistance();
     Serial.println(dis); 
    
      if(ctl->b()){  // stop all
        digitalWrite(RF,LOW);
        digitalWrite(LB,LOW);
        digitalWrite(LF,LOW);
        digitalWrite(RB,LOW); 
      }

      if(ctl->a()){  // stop all
       lightIsTurnOn = !lightIsTurnOn;
        digitalWrite(light,lightIsTurnOn ? HIGH : LOW); 
      }

      if(ctl->r1()){  
        digitalWrite(RF,LOW);
        digitalWrite(LB,LOW);
        digitalWrite(LF,HIGH);
        digitalWrite(RB,HIGH);    
        analogWrite(l_pwm,speed);
        analogWrite(r_pwm,speed);  
      }

      if(ctl->r2()){  
        digitalWrite(RF,HIGH);
        digitalWrite(LB,LOW);
        digitalWrite(LF,HIGH);
        digitalWrite(RB,LOW);    
        analogWrite(l_pwm,255);
        analogWrite(r_pwm,190);    
      }


      if(ctl->l1()){  
        digitalWrite(RF, HIGH);
         digitalWrite(LB, HIGH);
         digitalWrite(LF, LOW);
         digitalWrite(RB, LOW);  
         analogWrite(l_pwm, speed);
         analogWrite(r_pwm, speed);    
      }

      if(ctl->l2()){  
        digitalWrite(RF,HIGH);
        digitalWrite(LB,LOW);
        digitalWrite(LF,HIGH);
        digitalWrite(RB,LOW);  
        analogWrite(l_pwm,150);
        analogWrite(r_pwm,255);     
      }

      if(ctl-> dpad() == 0X02) { // back
        digitalWrite(RF,LOW);
        digitalWrite(LB,HIGH);
        digitalWrite(LF,LOW);
        digitalWrite(RB,HIGH);
        analogWrite(l_pwm,speed);
        analogWrite(r_pwm,speed);                    
      }

      if(dis <= 30){
        digitalWrite(RF,LOW);
        digitalWrite(LB,LOW);
        digitalWrite(LF,HIGH);
        digitalWrite(RB,HIGH);    
        analogWrite(l_pwm,170);
        analogWrite(r_pwm,170);
      }
      
       if(ctl->y()) {
        if(dis > 10 ) {
            digitalWrite(RF, HIGH);
            digitalWrite(LB, LOW);
            digitalWrite(LF, HIGH);
            digitalWrite(RB, LOW);  
            analogWrite(l_pwm, 255);
            analogWrite(r_pwm, 255);        
        } else {
          stopC();
        }
      }
      
      if(ctl->dpad() == 0X01) {
        if(dis > 10 ) {
            digitalWrite(RF, HIGH);
            digitalWrite(LB, LOW);
            digitalWrite(LF, HIGH);
            digitalWrite(RB, LOW);  
            analogWrite(l_pwm, speed);
            analogWrite(r_pwm, speed);        
        } else {
          stopC();
        }
      }
      
      if(ctl->dpad()  == 0X08) { // left
        digitalWrite(RF, HIGH);
        digitalWrite(LB, LOW);
        digitalWrite(LF, HIGH);
        digitalWrite(RB, LOW);  
        analogWrite(l_pwm, 150);
        analogWrite(r_pwm, 255);                      
      }
     
      if(ctl->dpad()  == 0X04) { // right
        digitalWrite(RF,HIGH);
        digitalWrite(LB,LOW);
        digitalWrite(LF,HIGH);
        digitalWrite(RB,LOW);    
        analogWrite(l_pwm,255);
        analogWrite(r_pwm,150);                    
      }
           
     
  }
    fflush(stdin);
}
