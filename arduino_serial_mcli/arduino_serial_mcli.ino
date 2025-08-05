// Simple Arduino CLI Example

#include "mcli.h"
#include "mcli_arduino_serial.h"
#include <avr/pgmspace.h>

// Application context
struct MyAppContext {
    ArduinoSerialIo& io_;
    int led_pin;
    bool led_state;
    unsigned long uptime_start;
    
    MyAppContext(ArduinoSerialIo& io, int led_pin) 
    : led_pin(led_pin), led_state(false), uptime_start(millis()), io_(io) {}
};

// Implemented commands
void cmd_led(const mcli::CommandArgs args, MyAppContext* ctx) {
    if (args.argc < 2) {
        ctx->io_.println("Usage: led <on|off|status>");
        return;
    }
    
    if (strcmp(args.argv[1], "on") == 0) {
        digitalWrite(ctx->led_pin, HIGH);
        ctx->led_state = true;
        ctx->io_.println("LED ON");
    } else if (strcmp(args.argv[1], "off") == 0) {
        digitalWrite(ctx->led_pin, LOW);
        ctx->led_state = false;
        ctx->io_.println("LED OFF");
    } else if (strcmp(args.argv[1], "status") == 0) {
        ctx->io_.print("LED is currently: ");
        ctx->io_.println(ctx->led_state ? "ON" : "OFF");
        ctx->io_.print("LED pin: ");
        ctx->io_.println(ctx->led_pin);
    } else {
        ctx->io_.println("Unknown argument! Usage: led <on|off|status>");
    }
}

void cmd_uptime(const mcli::CommandArgs args, MyAppContext* ctx) {
    unsigned long uptime = millis() - ctx->uptime_start;
    unsigned long total_seconds = uptime / 1000;
    unsigned long hours = total_seconds / 3600;
    unsigned long minutes = (total_seconds % 3600) / 60;
    unsigned long seconds = total_seconds % 60;
    
    ctx->io_.printf("Uptime: %02lu:%02lu:%02lu\r\n", hours, minutes, seconds);
}

void cmd_analog_read(const mcli::CommandArgs args, MyAppContext* ctx) {
    if (args.argc < 2) {
        ctx->io_.println("Usage: aread <pin>");
        return;
    }
    
    int pin = atoi(args.argv[1]);
    if (pin < 0 || pin > 7) {
        ctx->io_.println("Error: Pin must be 0-7 (A0-A7)");
        return;
    }
    
    int value = analogRead(A0 + pin);
    uint32_t millivolts = (value * 5000UL) / 1024;
    ctx->io_.printf("A%d = %d (%lu.%03luV)\r\n", pin, value, millivolts/1000, millivolts%1000);
}

void cmd_digital_read(const mcli::CommandArgs args, MyAppContext* ctx) {
    if (args.argc < 2) {
        ctx->io_.println("Usage: dread <pin>");
        return;
    }
    
    int pin = atoi(args.argv[1]);
    if (pin < 0 || pin > 13) {
        ctx->io_.println("Error: Pin must be 0-13");
        return;
    }
    
    pinMode(pin, INPUT);
    int value = digitalRead(pin);
    ctx->io_.printf("D%d = %s\r\n", pin, value ? "HIGH" : "LOW");
}

void cmd_digital_write(const mcli::CommandArgs args, MyAppContext* ctx) {
    if (args.argc < 3) {
        ctx->io_.println("Usage: dwrite <pin> <value>");
        ctx->io_.println("  value: 0/1 or LOW/HIGH");
        return;
    }
    
    int pin = atoi(args.argv[1]);
    if (pin < 0 || pin > 13) {
        ctx->io_.println("Error: Pin must be 0-13");
        return;
    }
    
    int value;
    if (strcmp(args.argv[2], "HIGH") == 0 || strcmp(args.argv[2], "1") == 0) {
        value = HIGH;
    } else if (strcmp(args.argv[2], "LOW") == 0 || strcmp(args.argv[2], "0") == 0) {
        value = LOW;
    } else {
        ctx->io_.println("Error: Value must be HIGH/LOW or 1/0");
        return;
    }
    
    pinMode(pin, OUTPUT);
    digitalWrite(pin, value);
    ctx->io_.print("D");
    ctx->io_.print(pin);
    ctx->io_.print(" set to ");
    ctx->io_.println(value ? "HIGH" : "LOW");
}

void cmd_reset(const mcli::CommandArgs args, MyAppContext* ctx) {
    ctx->io_.println("Resetting Arduino...");
    delay(100);
    // Software reset by jumping to address 0
    asm volatile ("jmp 0");
}

// Command table
const mcli::CommandDefinition<MyAppContext> cli_commands[] = {
    {"led", cmd_led, "LED interface (led <on|off|status>)"},
    {"uptime", cmd_uptime, "Show system uptime"},
    {"aread", cmd_analog_read, "Read analog pin (aread <pin>)"},
    {"dread", cmd_digital_read, "Read digital pin (dread <pin>)"},
    {"dwrite", cmd_digital_write, "Write digital pin (dwrite <pin> <value>)"},
    {"reset", cmd_reset, "Reset Arduino"}
};


// Global objects
const int LED_PIN = 13;  // Built-in LED
ArduinoSerialIo adapter(Serial);
MyAppContext app_context(adapter, LED_PIN);
mcli::CliEngine<MyAppContext> cli_engine(adapter, app_context, cli_commands);

// Progmem foolery to get the nice splash to work 
const char splash0[] PROGMEM = "███╗   ███╗ ██████╗ ██╗    ██████╗";
const char splash1[] PROGMEM = "████╗ ████║██╔═══██╗██║    ╚═██╔═╝";
const char splash2[] PROGMEM = "██╔████╔██║██║   ╚═╝██║      ██║  ";
const char splash3[] PROGMEM = "██║╚██╔╝██║██║   ██╗██║      ██║  ";
const char splash4[] PROGMEM = "██║ ╚═╝ ██║╚██████╔╝██████╗██████╗";
const char splash5[] PROGMEM = "╚═╝     ╚═╝ ╚═════╝ ╚═════╝╚═════╝";
const char splash6[] PROGMEM = "   === Arduino CLI Example ===   ";

const char* const splash_lines[] PROGMEM = { splash0, splash1, splash2, splash3, splash4, splash5, splash6};

void printProgmemString(const char* ptr) {
  char buffer[64]; // Enough for each line
  strcpy_P(buffer, ptr);
  Serial.println(buffer);
}
void printSplash() {
  for (uint8_t i = 0; i < sizeof(splash_lines) / sizeof(splash_lines[0]); i++) {
    // Read pointer to string from PROGMEM array
    const char* line = (const char*)pgm_read_ptr(&(splash_lines[i]));
    printProgmemString(line);
  }
}

void setup() {
    Serial.begin(115200);
    delay(100);

    // Initialize LED pin
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    // Print startup message
    adapter.print("\033[2J\033[H"); //Clear screen
    printSplash();
    adapter.println("Type 'help' for available commands");
    adapter.println();
}

void loop() {
    // Run the CLI
    cli_engine.process_input();
}