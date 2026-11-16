#include "clock.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <string>

    int s = 00; // seconds
    int m = 00; // minuts
    int h = 00; // hours
    int d = 00; // days

std::string NewClock() {
    
    for (;;) {
        s++;
        if (s == 60) {
            s = 0;
            m++;
        }

        if (m == 60) {
            m = 0;
            h++;
        }   

        if (h == 24) {
            h = 0;
            d++;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
    }
    




    return  std::to_string(h) + ":" +
            std::to_string(m) + ":" +
            std::to_string(s);

}