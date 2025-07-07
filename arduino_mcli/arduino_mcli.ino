// arduino_cli_example.ino
// Example Arduino sketch using the CLI Core library

#include "mcli.h"
#include "arduino_serial_io.h"

// Application context for Arduino
struct ArduinoContext {
    int led_pin;
    bool led_state;
    unsigned long uptime_start;
    
    ArduinoContext(int pin) : led_pin(pin), led_state(false), uptime_start(millis()) {}
};

// Command implementations
void cmd_led_on(const mcli::CommandArgs args, ArduinoContext* ctx) {
    digitalWrite(ctx->led_pin, HIGH);
    ctx->led_state = true;
    Serial.println("LED turned ON");
}

void cmd_led_off(const mcli::CommandArgs args, ArduinoContext* ctx) {
    digitalWrite(ctx->led_pin, LOW);
    ctx->led_state = false;
    Serial.println("LED turned OFF");
}

void cmd_led_toggle(const mcli::CommandArgs args, ArduinoContext* ctx) {
    ctx->led_state = !ctx->led_state;
    digitalWrite(ctx->led_pin, ctx->led_state ? HIGH : LOW);
    Serial.print("LED ");
    Serial.println(ctx->led_state ? "ON" : "OFF");
}

void cmd_led_status(const mcli::CommandArgs args, ArduinoContext* ctx) {
    Serial.print("LED is currently: ");
    Serial.println(ctx->led_state ? "ON" : "OFF");
    Serial.print("LED pin: ");
    Serial.println(ctx->led_pin);
}

void cmd_uptime(const mcli::CommandArgs args, ArduinoContext* ctx) {
    unsigned long uptime = millis() - ctx->uptime_start;
    unsigned long seconds = uptime / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    
    Serial.print("Uptime: ");
    Serial.print(hours);
    Serial.print("h ");
    Serial.print(minutes % 60);
    Serial.print("m ");
    Serial.print(seconds % 60);
    Serial.println("s");
}

void cmd_analog_read(const mcli::CommandArgs args, ArduinoContext* ctx) {
    if (args.argc < 2) {
        Serial.println("Usage: aread <pin>");
        return;
    }
    
    int pin = atoi(args.argv[1]);
    if (pin < 0 || pin > 7) {
        Serial.println("Error: Pin must be 0-7 (A0-A7)");
        return;
    }
    
    int value = analogRead(A0 + pin);
    Serial.print("A");
    Serial.print(pin);
    Serial.print(" = ");
    Serial.print(value);
    Serial.print(" (");
    Serial.print((value * 5.0) / 1024.0, 2);
    Serial.println("V)");
}

void cmd_digital_read(const mcli::CommandArgs args, ArduinoContext* ctx) {
    if (args.argc < 2) {
        Serial.println("Usage: dread <pin>");
        return;
    }
    
    int pin = atoi(args.argv[1]);
    if (pin < 0 || pin > 13) {
        Serial.println("Error: Pin must be 0-13");
        return;
    }
    
    pinMode(pin, INPUT);
    int value = digitalRead(pin);
    Serial.print("D");
    Serial.print(pin);
    Serial.print(" = ");
    Serial.println(value ? "HIGH" : "LOW");
}

void cmd_digital_write(const mcli::CommandArgs args, ArduinoContext* ctx) {
    if (args.argc < 3) {
        Serial.println("Usage: dwrite <pin> <value>");
        Serial.println("  value: 0/1 or LOW/HIGH");
        return;
    }
    
    int pin = atoi(args.argv[1]);
    if (pin < 0 || pin > 13) {
        Serial.println("Error: Pin must be 0-13");
        return;
    }
    
    int value;
    if (strcmp(args.argv[2], "HIGH") == 0 || strcmp(args.argv[2], "1") == 0) {
        value = HIGH;
    } else if (strcmp(args.argv[2], "LOW") == 0 || strcmp(args.argv[2], "0") == 0) {
        value = LOW;
    } else {
        Serial.println("Error: Value must be HIGH/LOW or 1/0");
        return;
    }
    
    pinMode(pin, OUTPUT);
    digitalWrite(pin, value);
    Serial.print("D");
    Serial.print(pin);
    Serial.print(" set to ");
    Serial.println(value ? "HIGH" : "LOW");
}

void cmd_reset(const mcli::CommandArgs args, ArduinoContext* ctx) {
    Serial.println("Resetting Arduino...");
    delay(100);
    // Software reset by jumping to address 0
    asm volatile ("jmp 0");
}

// Command table
const mcli::CommandDefinition<ArduinoContext> arduino_commands[] = {
    mcli::make_command("led_on", cmd_led_on, "Turn LED on"),
    mcli::make_command("led_off", cmd_led_off, "Turn LED off"),
    mcli::make_command("toggle", cmd_led_toggle, "Toggle LED state"),
    mcli::make_command("status", cmd_led_status, "Show LED status"),
    mcli::make_command("uptime", cmd_uptime, "Show system uptime"),
    mcli::make_command("aread", cmd_analog_read, "Read analog pin (aread <pin>)"),
    mcli::make_command("dread", cmd_digital_read, "Read digital pin (dread <pin>)"),
    mcli::make_command("dwrite", cmd_digital_write, "Write digital pin (dwrite <pin> <value>)"),
    mcli::make_command("reset", cmd_reset, "Reset Arduino"),
};

const size_t arduino_command_count = sizeof(arduino_commands) / sizeof(arduino_commands[0]);

// Global objects
const int LED_PIN = 13;  // Built-in LED
ArduinoContext app_context(LED_PIN);
ArduinoSerialIo adapter(Serial);
mcli::CliEngine<ArduinoContext> cli_engine(adapter, app_context);

void setup() {
    // Initialize serial communication
    Serial.begin(9600);
    while (!Serial) {
        ; // Wait for serial port to connect (needed for Leonardo/Micro)
    }
    
    // Initialize LED pin
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    // Register commands with CLI
    cli_engine.register_commands(arduino_commands, arduino_command_count);

    // Print startup message
    Serial.println();
    Serial.println("=== Arduino CLI Example ===");
    Serial.println("Type 'help' for available commands");
    Serial.println();
}

void loop() {
    // Run the CLI - this will block and handle commands
    //Serial.println(app_context.led_pin);
    cli_engine.process_input();
}