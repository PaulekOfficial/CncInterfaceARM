#include "utils.h"

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

void clearHistoryBuffer() {
    historyIndex = 0;
    messageHistory->clear();
}

void setupUart() {
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
    uint64_t startTime = get_absolute_time()._private_us_since_boot;
    while (getLatestSerialMessage() != "OK\r\n") {
        uint64_t now = get_absolute_time()._private_us_since_boot;

        watchdog_update();

        if (getLatestSerialMessage() == "FAIL\r\n") {
            return false;
        }

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

bool waitUntilSerialReady(uint64_t timeout) {
    timeout *= 1000;
    uint64_t startTime = get_absolute_time()._private_us_since_boot;
    while (getLatestSerialMessage() != "ready\r\n") {
        uint64_t now = get_absolute_time()._private_us_since_boot;

        watchdog_update();

        if (getLatestSerialMessage() == "ready\r\n") {
            return true;
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
    uint64_t startTime = get_absolute_time()._private_us_since_boot;
    while (getLatestSerialMessage().c_str()[0] != ch) {
        uint64_t now = get_absolute_time()._private_us_since_boot;

        watchdog_update();

        if (getLatestSerialMessage() == "FAIL\r\n") {
            return "ERROR";
        }

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
    uint64_t startTime = get_absolute_time()._private_us_since_boot;
    while (!getLatestSerialMessage().rfind(message, 0)) {
        uint64_t now = get_absolute_time()._private_us_since_boot;

        watchdog_update();

        if (getLatestSerialMessage() == "FAIL\r\n") {
            return "ERROR";
        }

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
    uint64_t startTime = get_absolute_time()._private_us_since_boot;
    while (getLatestSerialMessage() != message) {
        uint64_t now = get_absolute_time()._private_us_since_boot;
        watchdog_update();
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
    uint64_t startTime = get_absolute_time()._private_us_since_boot;
    int startHistoryIndex = historyIndex;

    while (getLatestSerialMessage() != "OK\r\n") {
        uint64_t now = get_absolute_time()._private_us_since_boot;

        watchdog_update();

        if (getLatestSerialMessage() == "FAIL\r\n") {
            break;
        }

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

float round(float var)
{
    // 37.66666 * 100 =3766.66
    // 3766.66 + .5 =3767.16    for rounding off value
    // then type cast to int so value is 3767
    // then divided by 100 so the value converted into 37.67
    float value = (int)(var * 100 + .5);
    return (float)value / 100;
}

void initADC() {
    info("ADC interface initialization...");
    adc_init();

    adc_set_temp_sensor_enabled(true);
    adc_gpio_init(ADC_SYSTEM_BATTERY);
    adc_gpio_init(ADC_EXTERNAL_BATTERY);
    info("Done.");
}
void initI2C() {
    info("I2C register interface initialization...");
    i2c_init(I2C_ID, 400 * 1000);
    info("setting gpio i2c functions");
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    info("activating internal pullup's");
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}
void initGPIO() {
    info("GPIO interface initialization...");
    gpio_init(RELAY_POWER_24V);
    gpio_init(RELAY_BAT_0);
    gpio_init(RELAY_BAT_1);
    gpio_init(MOSFET_BUZZER);
    gpio_init(MOSFET_LCD);
    gpio_init(V_24_SENSE);
    //gpio_set_function(MPU_LED, GPIO_FUNC_PWM);

    // Setup pwm
    uint slice_num = pwm_gpio_to_slice_num(MPU_LED);
    pwm_config config = pwm_get_default_config();
    // Set divider, reduces counter clock to sysclock/this value
    pwm_config_set_clkdiv(&config, 4.f);
    // Load the configuration into our PWM slice, and set it running.
    pwm_init(slice_num, &config, true);

    gpio_init(BUZZER);
    gpio_set_dir(RELAY_POWER_24V, GPIO_OUT);
    gpio_set_dir(RELAY_BAT_0, GPIO_OUT);
    gpio_set_dir(RELAY_BAT_1, GPIO_OUT);
    gpio_set_dir(MOSFET_BUZZER, GPIO_OUT);
    gpio_set_dir(MOSFET_LCD, GPIO_OUT);
    //gpio_set_dir(MPU_LED, GPIO_OUT);
    gpio_set_dir(BUZZER, GPIO_OUT);


    gpio_set_dir(V_24_SENSE, GPIO_IN);
}

void initPullUps() {
    info("Pull ups initialization...");
    gpio_pull_up(BUZZER);
}

std::string gen_random_string(const int len) {
    static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
}

