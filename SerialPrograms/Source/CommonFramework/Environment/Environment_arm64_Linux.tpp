/*  Environment (arm64 Linux)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Currently only used for M-series Apple environment
 */


#include <stdio.h>
#include <stdint.h>

#ifdef __APPLE__
#include <sys/sysctl.h>
#else
#include <set>
#include <map>
#include <thread>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Time.h"
#include "CommonFramework/Logging/Logger.h"
#endif

#include "Environment.h"

namespace PokemonAutomation{

#ifdef __APPLE__
uint64_t get_cpu_freq()
{
    uint64_t freq = 0;
    size_t size = sizeof(freq);

    if (sysctlbyname("hw.cpufrequency", &freq, &size, NULL, 0) < 0)
    {
        perror("sysctl");
    }
    return freq;
}

std::string get_processor_name(){
    char name_buffer[100] = "";
    size_t size = 100;
    if (sysctlbyname("machdep.cpu.brand_string", name_buffer, &size, NULL, 0) < 0)
    {
        perror("sysctl");
    }
    return name_buffer;
}

ProcessorSpecs get_processor_specs(){
    ProcessorSpecs specs;
    specs.name = get_processor_name();
    specs.base_frequency = get_cpu_freq();
    specs.threads = std::thread::hardware_concurrency();

    return specs;
}
#else

uint64_t arm_rdtsc(){
    uint64_t val;

    /*
     * According to ARM DDI 0487F.c, from Armv8.0 to Armv8.5 inclusive, the
     * system counter is at least 56 bits wide; from Armv8.6, the counter
     * must be 64 bits wide.  So the system counter could be less than 64
     * bits wide and it is attributed with the flag 'cap_user_time_short'
     * is true.
     */
    asm volatile("mrs %0, cntvct_el0" : "=r" (val));

    return val;
}

uint64_t arm_measure_rdtsc_ticks_per_sec(){
//    Time::WallClock w_start = Time::WallClock::Now();
    auto w_start = current_time();
    uint64_t r_start = arm_rdtsc();
    while (current_time() - w_start < std::chrono::microseconds(62500));
    auto w_end = current_time();
//    while (w_start.SecondsElapsed() < 0.0625);
//    Time::WallClock w_end = Time::WallClock::Now();
    uint64_t r_end = arm_rdtsc();

    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(w_end - w_start);
    double seconds = (double)elapsed.count() / 1000000.;

    return (uint64_t)((double)(r_end - r_start) / seconds);
}

uint64_t arm_rdtsc_ticks_per_sec(){
    static uint64_t cached = arm_measure_rdtsc_ticks_per_sec();
    return cached;
}

std::string get_processor_name(){
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    std::string processor_name = "";
    bool found = false;
    while (std::getline(cpuinfo, line)) {
        if (line.find("model name") != std::string::npos) {
            std::size_t pos = line.find(": ");
            if (pos != std::string::npos) {
                processor_name = line.substr(pos + 2);
                found = true;
                break;
            }
        }
    }

    if (!found)
    {
        perror("sysctl");
    }
    return processor_name;
}

ProcessorSpecs get_processor_specs(){
    ProcessorSpecs specs;
    specs.name = get_processor_name();
    specs.base_frequency = arm_rdtsc_ticks_per_sec();
    specs.threads = std::thread::hardware_concurrency();

#ifdef __linux
    //  Cores + Sockets
    {
//        std::set<int> cores;
        std::set<int> sockets;

        int current_socket = 0;
        std::map<int, std::set<int>> cores_per_socket;

        std::ifstream file("/proc/cpuinfo");
        std::string line;
        while (std::getline(file, line)){
            if (line.find("physical id") == 0){
                size_t pos = line.find(": ");
                if (pos == std::string::npos){
                    throw InternalSystemError(nullptr, PA_CURRENT_FUNCTION, "Unable to parse: /proc/cpuinfo");
                }
                current_socket = atoi(&line[pos + 2]);
                sockets.insert(current_socket);
            }
            if (line.find("core id") == 0){
                size_t pos = line.find(": ");
                if (pos == std::string::npos){
                    throw InternalSystemError(nullptr, PA_CURRENT_FUNCTION, "Unable to parse: /proc/cpuinfo");
                }
//                specs.threads++;
                int core = atoi(&line[pos + 2]);
                cores_per_socket[current_socket].insert(core);
//                cores.insert(core);
            }
        }
        specs.sockets = sockets.size();
        specs.cores = 0;
        for (const auto& socket : cores_per_socket){
            specs.cores += socket.second.size();
        }
    }

    //  NUMA Nodes
    {
        std::set<int> nodes;

        std::ifstream file("/proc/zoneinfo");
        std::string line;
        while (std::getline(file, line)){
            if (line.find("Node ") == 0){
                nodes.insert(atoi(&line[5]));
            }
        }
        specs.numa_nodes = nodes.size();
    }
#endif

    return specs;
}

#endif


}
