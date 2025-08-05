// Simple ESP32 UART CLI Example 

#include "mcli.h"
#include "mcli_esp32_uart.h"
#include "esp_log.h"
#include "esp_system.h"

// Application context
struct MyAppContext {
    ESP32UartIo& io;
    uint32_t session_start_time;
    MyAppContext(ESP32UartIo& io)
    : io(io) {
        session_start_time = esp_log_timestamp();
    }
};

// Implemented commands
void cmd_hello(const mcli::CommandArgs args, MyAppContext* ctx) {
    ctx->io.println("Hello via UART!");
}

void cmd_status(const mcli::CommandArgs args, MyAppContext* ctx) {
    ctx->io.printf("Status: Free heap = %lu bytes\r\n", esp_get_free_heap_size());
}

void cmd_uptime(const mcli::CommandArgs args, MyAppContext* ctx) {
    uint32_t uptime_ms = esp_log_timestamp();
    ctx->io.printf("Uptime: %lu seconds\r\n", uptime_ms / 1000);
}

// Command table
const mcli::CommandDefinition<MyAppContext> app_commands[] = {
    {"hello", cmd_hello, "Say hello"},
    {"status", cmd_status, "Show system status"},
    {"uptime", cmd_uptime, "Show system uptime"}
};

extern "C" void app_main(void)
{
    // Create UART I/O adapter
    ESP32UartIo uart_io(UART_NUM_0, 115200, GPIO_NUM_1, GPIO_NUM_3);
    

    // Create context and CLI engine
    MyAppContext ctx(uart_io);
    mcli::CliEngine<MyAppContext> cli(uart_io, ctx, app_commands);
    
    // Send welcome message
    uart_io.println();
    uart_io.println("=== ESP32 UART CLI ===");
    uart_io.println("Type 'help' for available commands");
    uart_io.println();
    
    // Run CLI in infinite loop
    while (true) {
        cli.process_input();
        vTaskDelay(pdMS_TO_TICKS(10));  // Small delay for other tasks
        // Could do some other stuff here as well
    }

}