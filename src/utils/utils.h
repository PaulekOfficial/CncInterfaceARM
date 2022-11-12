#include <string>
#include <list>
#include <pico/stdlib.h>
#include <sys/time.h>
#include <ctime>
#include <hardware/regs/intctrl.h>
#include <hardware/irq.h>
#include <hardware/watchdog.h>

using namespace std;

#ifndef CNCSMARTINTERFACE_UTILS_H
#define CNCSMARTINTERFACE_UTILS_H

#endif //CNCSMARTINTERFACE_UTILS_H

#define UART_ID              uart1
#define BAUD_RATE            115200
#define DATA_BITS            8
#define STOP_BITS            1
#define PARITY               UART_PARITY_NONE

#define WIFI_MODULE_TX      8
#define WIFI_MODULE_RX      9

void setupUart();
void debug(string debug_message);
void info(string info_message);
void sendSerialMessage(string message, uart_inst *uart_inst);
bool waitUntilGetSerialMessage(string message, uint64_t timeout);
bool waitUntilSerialOK(uint64_t timeout);
string waitUntilSerialStartsWith(string message, uint64_t timeout);
string waitUntilSerialFirstChar(char ch, uint64_t timeout);
list<string> collectSerialMessagesTillOK(uint64_t timeout);
bool replace(std::string& str, const std::string& from, const std::string& to);
void replaceAll(std::string& str, const std::string& from, const std::string& to);
void clearHistoryBuffer();
