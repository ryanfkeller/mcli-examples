#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

namespace mcli {

    // =============================================================================
    // CLI TYPES
    // =============================================================================

    // Configuration constants
    constexpr int MAX_ARGS = 5;
    constexpr int MAX_ARG_LENGTH = 16;
    constexpr size_t CMD_BUFFER_SIZE = 128;
    constexpr const char* DEFAULT_PROMPT = "mcli> ";

    // Calculate total CommandArgs memory usage at compile time
    constexpr size_t COMMAND_ARGS_SIZE = sizeof(int) + (MAX_ARGS * MAX_ARG_LENGTH);
    static_assert(COMMAND_ARGS_SIZE <= 300, "CommandArgs too large for constrained systems");

    // Command argument structure
    struct CommandArgs {
        int argc;
        char argv[MAX_ARGS][MAX_ARG_LENGTH];

        CommandArgs() : argc(0) {
            memset(argv, 0, sizeof(argv));
        }
    };

    // Generic command function signature
    template<typename ContextType>
    using CommandFunction = void(*)(const CommandArgs args, ContextType* ctx);

    // Command definition structure
    template<typename ContextType>
    struct CommandDefinition {
        const char* name;
        CommandFunction<ContextType> execute;
        const char* help;
    };

    // Command generation function
    template<typename ContextType>
    constexpr CommandDefinition<ContextType> make_command(
            const char* name,
            CommandFunction<ContextType> func,
            const char* help) {
        return {name, func, help};
    }

    // =============================================================================
    // CLI I/O INTERFACE
    // =============================================================================

    /**
     * Abstract interface for CLI I/O operations
     * Platform-specific implementations should inherit from this interface
     */
    class CliIoInterface {
    public:
        virtual ~CliIoInterface() = default;

        /**
         * Core interface -- derived classes must implement these
         */
        virtual void put_byte(char c) = 0;
        virtual char get_byte() = 0;
        virtual bool byte_available() = 0;

        /**
         * Bulk interface -- recommend override for packet-based interfaces
         */
        virtual void put_bytes(const char* data, size_t len) {
            for (size_t count = 0; count < len; count++) {
                put_byte(data[count]);
            }
        }

        virtual size_t get_bytes(char* buffer, size_t max_len) {
            size_t count = 0;
            while (count < max_len && byte_available()) {
                buffer[count++] = get_byte();
            }
            return count;
        }

        /**
         * High level functions -- default behavior can be overridden if needed
         */
        virtual void print(const char* str) {
            if (!str) return;

            size_t len = strlen(str);
            if (len > 0) {
                put_bytes(str, len);
            }
        }
        virtual void println() {
            print("\r\n");
        }
        virtual void println(const char* str) {
            print(str);
            println();
        }
       virtual void printf(const char* fmt, ...) {
            if (!fmt) return;
    
            va_list args;
            va_start(args, fmt);
    
            // Small stack-based buffer for embedded systems
            char buffer[64];
            int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
            va_end(args);
    
            if (len > 0) {
                // Ensure null termination and handle truncation
                if (len >= sizeof(buffer)) {
                    len = sizeof(buffer) - 1;
                    buffer[len] = '\0';
                }
                put_bytes(buffer, len);
            }
        }

        virtual void flush() {
            // Default implementation is no-op since byte-level interface
            // assumes immediate transmission. Override if buffering is used.
        }
        
        // Optional terminal control methods
        // (Can be overridden for enhanced/alternate functionality)
        virtual void clear_screen() {
            print("\x1b[2J\r\n");
        }

        virtual void send_prompt(const char* prompt = DEFAULT_PROMPT) {
            print(prompt);
        }

        virtual void send_backspace() {
            print("\b \b");
        }
    };

    // =============================================================================
    // CLI ENGINE
    // =============================================================================

    /**
     * Generic CLI engine that handles command parsing, dispatch, and I/O. 
     * Template parameter allows for any application-specific context type.
     */
    template<typename ContextType>
    class CliEngine {
        public:
            /**
             * Constructor
             * @param io Reference to I/O interface implementation
             * @param context Reference to application-specific context
             * @param prompt Custom prompt string (optional)
             */
            CliEngine(CliIoInterface& io, ContextType& context, const char* prompt = DEFAULT_PROMPT);

            /**
             * Register command with the CLI engine
             * @param commands Array of command definitions
             * @param count Number of commands in the array
             */
            void register_commands(const CommandDefinition<ContextType>* commands, size_t count);

            /**
             * Main CLI loop -- runs indefinitely processing commands
             */
            void process_input();

            /**
             * Process a single command line (useful for testing or non-interactive use)
             * @param command_line String containing the command and arguments
             * @return true if command was found and executed, false otherwise
             */
            bool execute_command(const char* command_line);

            /**
             * Print available commands
             */
            void print_help();
        
        private:
            // Parse command line into argc/argv format
            CommandArgs parse_command_line(const char* input);

            // Get command input from user with echo and backspace support
            CommandArgs get_command_input();

            // Find and execute a command
            bool dispatch_command(const CommandArgs& args);

            // Member variables
            CliIoInterface& io_;
            ContextType& context_;
            const char* prompt_;
            const CommandDefinition<ContextType>* commands_;
            size_t command_count_;

            // Input buffer for command parsing
            char input_buffer_[CMD_BUFFER_SIZE];
    };

    // =============================================================================
    // CLI ENGINE IMPLEMENTATION
    // =============================================================================

    template<typename ContextType>
    CliEngine<ContextType>::CliEngine(CliIoInterface& io, ContextType& context, const char* prompt)
        : io_(io), context_(context), prompt_(prompt), commands_(nullptr), command_count_(0) {}

    template<typename ContextType>
    void CliEngine<ContextType>::register_commands(const CommandDefinition<ContextType>* commands, size_t count) {
        commands_ = commands;
        command_count_ = count;
    }

    template<typename ContextType>
    void CliEngine<ContextType>::process_input() {
        if (!io_.byte_available()) return;
        CommandArgs args = get_command_input();
        if (args.argc > 0) {
            if (!dispatch_command(args)) {
                    io_.print("Command \"");
                    io_.print(args.argv[0]);
                    io_.println("\" not found. Type 'help' for available commands.");
                }
        }
    }

    template<typename ContextType>
    bool CliEngine<ContextType>::execute_command(const char* command_line) {
        CommandArgs args = parse_command_line(command_line);
        return dispatch_command(args);
    }

    template<typename ContextType>
    void CliEngine<ContextType>::print_help() {
        io_.println();
        io_.println("Available commands:");
    
        // Show built-in commands
        io_.printf("  %-15s -- %s\r\n", "help", "Show available commands");
        
        // Show user commands
        if (commands_ && command_count_ > 0) {
            for (size_t i = 0; i < command_count_; i++) {
                io_.printf("  %-15s -- %s\r\n", commands_[i].name, commands_[i].help);
            }
        } else {
            io_.println("  (No additional commands registered)");
        }
        io_.println();
    }

    template<typename ContextType>
    CommandArgs CliEngine<ContextType>::get_command_input() {
        memset(input_buffer_, 0, sizeof(input_buffer_));
        char* buffer_ptr = input_buffer_;
        size_t char_count = 0;
        uint8_t in_char = 0;

        io_.send_prompt(prompt_);

        while (true) {
            in_char = io_.get_byte();

            // Handle backspace
            if (in_char == 8 || in_char == 127) {
                if (char_count > 0) {
                    buffer_ptr--;
                    char_count--;
                    io_.send_backspace();
                }
                continue;
            }

            // Handle CRLF
            if (in_char == '\r' || in_char == '\n') {
                io_.println();
                // Only break if we've typed something
                if (char_count > 0) {
                    break;
                } else {
                    io_.send_prompt(prompt_);
                    continue;
                }
            }

            // Handle regular characters
            io_.put_byte(in_char);
            *buffer_ptr = in_char;
            buffer_ptr++;
            char_count++;

            // Prevent buffer overflow
            if (char_count >= CMD_BUFFER_SIZE - 1) {
                break;
            }
        }

        *buffer_ptr = '\0';
        return parse_command_line(input_buffer_);
    }

    template<typename ContextType>
    CommandArgs CliEngine<ContextType>::parse_command_line(const char* input) {
        CommandArgs args;

        // Work with a local copy for tokenization
        char temp_buffer[CMD_BUFFER_SIZE];
        strncpy(temp_buffer, input, CMD_BUFFER_SIZE-1);
        temp_buffer[CMD_BUFFER_SIZE - 1] = '\0';

        char* token = temp_buffer;
        while (*token && args.argc < MAX_ARGS - 1) {
            // Skip leading spaces
            while (*token == ' ') token++;
            if (!*token) break;

            // Token start
            // Copy token directly into args.argv[args.argc]
            char* dest = args.argv[args.argc];
            int char_count = 0;
            while (*token && *token != ' ' && char_count < MAX_ARG_LENGTH - 1) {
                *dest++ = *token++;
                char_count++;
            }
            *dest = '\0';

            args.argc++;

            // Skip trailing spaces
            while (*token == ' ') token++;
        }

        return args;
    }

    template<typename ContextType>
    bool CliEngine<ContextType>::dispatch_command(const CommandArgs& args) {
        if (args.argc == 0) {
            return false;
        }

        // Handle built-in commands first
        if (strcmp(args.argv[0], "help") == 0) {
            print_help();
            return true;
        }

        // Handle user-registered commands
        if (!commands_) {
            return false;
        }

        for (size_t i = 0; i < command_count_; i++) {
            if (strcmp(args.argv[0], commands_[i].name) == 0) {
                commands_[i].execute(args, &context_);
                return true;
            }
        }

        return false;
    }
}