#include <spdlog/spdlog.h>
#include <libaprs.h>
#include <argp.h>
#include <string>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <thread>
#include <chrono>

struct termios old;

void receive_raw_callback(char* data, uint32_t len)
{
    std::cout << data;
}

int main(int argc, char *argv[])
{
    spdlog::set_level(spdlog::level::off);
    APRS::init_ip("127.0.0.1", 8001, IFACE_KISS);
    APRS::set_receive_raw_callback(&receive_raw_callback);

    // Set the terminal to raw mode
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    tcgetattr(STDIN_FILENO, &old);
    raw.c_lflag &= ~(ECHO | ICANON); // TODO: Make echo togglable with an argument
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    // Transmit individual bytes from stdin
    char* buf = new char[128];
    int i = 0;
    int timer = 0;
    bool timerExpired = false;
    char c;
    std::cin.sync_with_stdio(true);
    while (true)
    {
        // Wait until there's a byte in stdin
        while (std::cin.peek() == EOF) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));      
            timer++;
            if(timer > 100) { // 1 second has passed, send what we have
                if(i > 0) { // Only send if we have something to send
                    timerExpired = true;
                    break;
                }
                else 
                    timer = 0;
            }
        } 
        timer = 0;

        // Read the byte from stdin if there is any byte
        // The condition is only here for if the timer expires
        if(std::cin.peek() != EOF) {
            std::cin.get(c);
            buf[i] = c;
            i++;
        }

        // If we've reached a string termination or new line but the buffer is not full, 
        // fill the remaining buffer with string terminations so it meets the minimum of 15 bytes
        // that direwolf expects.
        if((c == '\0' || c == '\n' || c == EOF) || timerExpired) {
            if(i < 15)
                for(i; i < 15; i++)
                    buf[i] = '\0';

            APRS::send_raw(buf, i);
            i = 0;
            std::cin.clear();
        }

        // If we've reached the end of the buffer, send it to direwolf
        if(i == 127) {
            APRS::send_raw(buf, 128);
            i = 0;
            std::cin.clear();
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
        if(i >= 128)
            i = 0;
    }
}