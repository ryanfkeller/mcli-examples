// arduino_cli_example.ino
// Example Arduino sketch using the CLI Core library

#include "mcli.h"
#include "arduino_serial_io.h"

// Application context for Arduino
struct ArduinoContext {
    int led_pin;
    bool led_state;
    unsigned long uptime_start;
    ArduinoSerialIo& io_;
    
    ArduinoContext(int pin, ArduinoSerialIo& io) : led_pin(pin), led_state(false), uptime_start(millis()), io_(io) {}
};

// Command implementations
void cmd_led_on(const mcli::CommandArgs args, ArduinoContext* ctx) {
    digitalWrite(ctx->led_pin, HIGH);
    ctx->led_state = true;
    ctx->io_.println("LED turned ON");
}

void cmd_led_off(const mcli::CommandArgs args, ArduinoContext* ctx) {
    digitalWrite(ctx->led_pin, LOW);
    ctx->led_state = false;
    ctx->io_.println("LED turned OFF");
}

void cmd_led_toggle(const mcli::CommandArgs args, ArduinoContext* ctx) {
    ctx->led_state = !ctx->led_state;
    digitalWrite(ctx->led_pin, ctx->led_state ? HIGH : LOW);
    ctx->io_.print("LED ");
    ctx->io_.println(ctx->led_state ? "ON" : "OFF");
}

void cmd_led_status(const mcli::CommandArgs args, ArduinoContext* ctx) {
    ctx->io_.print("LED is currently: ");
    ctx->io_.println(ctx->led_state ? "ON" : "OFF");
    ctx->io_.print("LED pin: ");
    ctx->io_.println(ctx->led_pin);
}

void cmd_uptime(const mcli::CommandArgs args, ArduinoContext* ctx) {
    unsigned long uptime = millis() - ctx->uptime_start;
    unsigned long seconds = uptime / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    
    ctx->io_.print("Uptime: ");
    ctx->io_.print(hours);
    ctx->io_.print("h ");
    ctx->io_.print(minutes % 60);
    ctx->io_.print("m ");
    ctx->io_.print(seconds % 60);
    ctx->io_.println("s");
}

void cmd_analog_read(const mcli::CommandArgs args, ArduinoContext* ctx) {
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
    ctx->io_.printf("A%d = %d (%fV)\r\n", pin, value, (value * 5.0) / 1024.0);
}

void cmd_digital_read(const mcli::CommandArgs args, ArduinoContext* ctx) {
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

void cmd_digital_write(const mcli::CommandArgs args, ArduinoContext* ctx) {
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

void cmd_reset(const mcli::CommandArgs args, ArduinoContext* ctx) {
    ctx->io_.println("Resetting Arduino...");
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
ArduinoSerialIo adapter(Serial);
ArduinoContext app_context(LED_PIN, adapter);
mcli::CliEngine<ArduinoContext> cli_engine(adapter, app_context);


void setup() {
    Serial.begin(9600);


    // Initialize LED pin
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    // Register commands with CLI
    cli_engine.register_commands(arduino_commands, arduino_command_count);

    // Print startup message
    adapter.println();
    adapter.println("=== Arduino CLI Example ===");
    adapter.println("Type 'help' for available commands");
    adapter.println();
}

void loop() {
    // Run the CLI - this will block and handle commands
    //Serial.println(app_context.led_pin);
    cli_engine.process_input();
}