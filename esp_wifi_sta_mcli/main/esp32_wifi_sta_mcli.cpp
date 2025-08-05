// Simple ESP32 WiFi CLI Example 

#include "mcli.h"
#include "mcli_esp32_wifi_sta.h"
#include "esp_log.h"
#include "esp_system.h"
#include "config.h"

// Application context
struct MyAppContext {
    ESP32WiFiIo& io;
    int client_socket;
    uint32_t session_start_time;
    MyAppContext(ESP32WiFiIo& io) :
        io(io),
        client_socket(io.get_client_socket()) {
        session_start_time = esp_log_timestamp();
    }
};

// Implemented commands
void cmd_hello(const mcli::CommandArgs args, MyAppContext* ctx) {
    ctx->io.printf("Hello from client socket %d!\r\n", ctx->client_socket);
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
    while (1) {
        // Create WiFi I/O adapter
        ESP32WiFiIo wifi_io(WIFI_SSID, WIFI_PASSWORD, 23);
        
        // Wait for a client to connect (blocks until someone connects)
        if (!wifi_io.wait_for_client()) {
            ESP_LOGE("main", "Failed to get client connection");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            continue;
        }
        
        // Create context and CLI engine
        MyAppContext ctx(wifi_io);
        mcli::CliEngine<MyAppContext> cli(wifi_io, ctx, app_commands);
        
        // Send welcome message
        wifi_io.println("███╗   ███╗ ██████╗ ██╗    ██████╗");
        wifi_io.println("████╗ ████║██╔═══██╗██║    ╚═██╔═╝");
        wifi_io.println("██╔████╔██║██║   ╚═╝██║      ██║  ");
        wifi_io.println("██║╚██╔╝██║██║   ██╗██║      ██║  ");
        wifi_io.println("██║ ╚═╝ ██║╚██████╔╝██████╗██████╗");
        wifi_io.println("╚═╝     ╚═╝ ╚═════╝ ╚═════╝╚═════╝");
        wifi_io.println("    === ESP32 WiFi Example ===    ");
        wifi_io.println("Type 'help' for available commands");
        wifi_io.println();
        
        // Run CLI until client disconnects
        while (wifi_io.is_connected()) {
            cli.process_input();
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        
        ESP_LOGI("main", "Client disconnected, waiting for next client...");
        // Loop back to wait for next client
    }
}