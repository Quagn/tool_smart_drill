TaskHandle_t Task1;
TaskHandle_t Task2;

uint8_t                                         // Pin inout
pin_trigger_direction = 39,                       // Trigger pin      
pin_trigger_power     = 34,

pin_esc_direction     = 33,                       // Esc pin
pin_esc_power         = 32;

uint32_t 
loop_timer,                                     // Loop timer
window_timer = 10000;                           // Windows time 100hz

uint16_t
power; 

bool
direction_,
trigger;

void setup() {
  Serial.begin(115200);

  pinMode(pin_trigger_direction, INPUT);                          // Trigger
  
  pinMode(pin_esc_direction, OUTPUT);                             // Esc
  pinMode(pin_esc_power, OUTPUT);                                 // Esc
  
  loop_timer = micros();                                          // Start timer
}

void loop() {
  // Read the state of pin_trigger_direction
  direction_ = (GPIO.in1.val & (1 << (pin_trigger_direction - 32))) != 0;
  
  // Map analog value to power level (power) from 0 to 10
  power = map(analogRead(pin_trigger_power), 0, 4095, 0, 10);

  // Set direction for ESC
  (direction_ ? GPIO.out1_w1ts.val : GPIO.out1_w1tc.val) = (1 << (pin_esc_direction - 32));

  if (power == 0) {
    // If power is 0, turn on ESC and set trigger to false
    GPIO.out1_w1ts.val = (1 << (pin_esc_power - 32));
    trigger = false;
  } else if (power < 3 && !trigger) {
    // If power is between 1 and 2 and trigger hasn't been activated
    GPIO.out1_w1tc.val = (1 << (pin_esc_power - 32));    // Set ESC output HIGH
    delayMicroseconds(30000);
    GPIO.out1_w1ts.val = (1 << (pin_esc_power - 32));    // Set ESC output LOW
    trigger = true;
  } else if (power == 1) {
    // If power is 1
    GPIO.out1_w1tc.val = (1 << (pin_esc_power - 32));    // Set ESC output HIGH
    delayMicroseconds(2000);
    GPIO.out1_w1ts.val = (1 << (pin_esc_power - 32));    // Set ESC output LOW
    delayMicroseconds(8000);
  } else {
    // Default for other power values (including power = 10)
    GPIO.out1_w1tc.val = (1 << (pin_esc_power - 32));    // Set ESC output HIGH
    
    if (power < 10) {
      // If power is less than 10, delay according to power level
      delayMicroseconds(power * 1000);
      GPIO.out1_w1ts.val = (1 << (pin_esc_power - 32));  // Set ESC output LOW
      delayMicroseconds((10 - power) * 1000);
    }
  }
}
