#include <utils/utils.h>

bool debug_enabled = true;

string rawSerialMessage;
string messageHistory[100];
int historyIndex = -1;

// RX debugger
void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        char ch = uart_getc(UART_ID);
        rawSerialMessage += ch;
        if (ch == '\n') {
            debug("Serial received " + rawSerialMessage);
            historyIndex++;
            messageHistory[historyIndex] = rawSerialMessage;


            if (historyIndex >= 49) {
                historyIndex = 0;
            }

            //Remove old array
            rawSerialMessage = "";
        }
    }
}

void setupUart1() {
    info("UART interface initialization...");
    // Set uart0 pins function
    gpio_set_function(WIFI_MODULE_TX, GPIO_FUNC_UART);
    gpio_set_function(WIFI_MODULE_RX, GPIO_FUNC_UART);
    // Initialize uart
    uart_init(UART_ID, BAUD_RATE);
    // Set uart data sending format
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
    // Setup TX interrupt
    irq_set_exclusive_handler(UART1_IRQ, on_uart_rx);
    // Enable interrupt
    irq_set_enabled(UART1_IRQ, true);
    // Setup data
    uart_set_irq_enables(UART_ID, true, false);
    info("Done.");
}

void debug(string debug_message) {
    if (!debug_enabled) {
        return;
    }

    printf("DEBUG: %s \r\n", debug_message.c_str());
}

void info(string info_message) {
    printf("INFO: %s \r\n", info_message.c_str());
}

void sendSerialMessage(string message, uart_inst *uart_inst) {
    string command = message + "\r\n";
    uart_puts(uart_inst, command.c_str());
}

string getLatestSerialMessage() {
    return messageHistory[historyIndex];
}

bool waitUntilSerialOK(uint64_t timeout) {
    timeout *= 1000;
    uint64_t startTime = get_absolute_time();
    while (getLatestSerialMessage() != "OK\r\n") {
        uint64_t now = get_absolute_time();

        if (getLatestSerialMessage() == "ERROR\r\n") {
            return false;
        }

        if ((startTime + timeout) <= now) {
            return false;
        }

        watchdog_update();
    }

    messageHistory[historyIndex] = "";
    return true;
}

string waitUntilSerialFirstChar(char ch, uint64_t timeout) {
    timeout *= 1000;
    uint64_t startTime = get_absolute_time();
    while (getLatestSerialMessage().c_str()[0] != ch) {
        uint64_t now = get_absolute_time();

        if (getLatestSerialMessage() == "ERROR\r\n") {
            return "ERROR";
        }

        if ((startTime + timeout) <= now) {
            return "ERROR";
        }

        watchdog_update();
    }

    return messageHistory[historyIndex];
}

string waitUntilSerialStartsWith(string message, uint64_t timeout) {
    timeout *= 1000;
    uint64_t startTime = get_absolute_time();
    while (!getLatestSerialMessage().rfind(message, 0)) {
        uint64_t now = get_absolute_time();

        if (getLatestSerialMessage() == "ERROR\r\n") {
            return "ERROR";
        }

        if ((startTime + timeout) <= now) {
            return "ERROR";
        }

        watchdog_update();
    }

    return messageHistory[historyIndex];
}

bool waitUntilGetSerialMessage(string message, uint64_t timeout) {
    timeout *= 1000;
    uint64_t startTime = get_absolute_time();
    while (getLatestSerialMessage() != message) {
        uint64_t now = get_absolute_time();
        if ((startTime + timeout) <= now) {
            return false;
        }

        watchdog_update();
    }

    messageHistory[historyIndex] = "";
    return true;
}

list<string> collectSerialMessagesTillOK(uint64_t timeout) {
    timeout *= 1000;
    uint64_t startTime = get_absolute_time();
    int startHistoryIndex = historyIndex;

    while (getLatestSerialMessage() != "OK\r\n") {
        uint64_t now = get_absolute_time();

        if ((startTime + timeout) <= now) {
            return {};
        }

        watchdog_update();
    }

    messageHistory[historyIndex] = "";
    int endIndex = historyIndex;
    list<string> messages = {};
    for (int i = startHistoryIndex; i <= endIndex; i++) {
        messages.push_front(messageHistory[i]);
    }

    if (startHistoryIndex > endIndex) {
        for (int i = startHistoryIndex; i < 100; i++) {
            messages.push_front(messageHistory[i]);
        }

        for (int i = 0; i <= endIndex; i++) {
            messages.push_front(messageHistory[i]);
        }
    }

    return messages;
}

bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}
