// /proc/stat gives cumulative CPU ticks
// You read it twice with a 1-second gap and compute the delta
// Format: cpu  user nice system idle iowait irq softirq

#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <map>
#include <cstdio>
#include <iostream>

struct CpuTimes { long user, nice, system, idle, total; };

CpuTimes readCpu() {
    std::ifstream f("/proc/stat");
    std::string label;
    CpuTimes t{};
    f >> label >> t.user >> t.nice >> t.system >> t.idle;
    t.total = t.user + t.nice + t.system + t.idle;
    return t;
}

float getCpuPercent() {
    auto t1 = readCpu();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    auto t2 = readCpu();
    long totalDiff = t2.total - t1.total;
    long idleDiff  = t2.idle  - t1.idle;
    return 100.0f * (1.0f - (float)idleDiff / totalDiff);
}

float getMemPercent() {
    std::ifstream f("/proc/meminfo");
    std::map<std::string, long> m;
    std::string key; long val; std::string unit;
    while (f >> key >> val >> unit)
        m[key] = val;
    long total = m["MemTotal:"];
    long avail = m["MemAvailable:"];
    return 100.0f * (1.0f - (float)avail / total);
}

// Simplest approach: popen() to run systemctl

bool isServiceActive(const std::string& serviceName) {
    std::string cmd = "systemctl is-active --quiet " + serviceName;
    return (system(cmd.c_str()) == 0);  // returns 0 if active
}

int main() {
    std::cout << "System Monitor Starting...\n";
    std::cout << "--------------------------\n";

    // 1. Check CPU Usage
    std::cout << "CPU Usage:    " << getCpuPercent() << "%\n";

    // 2. Check Memory Usage
    std::cout << "Memory Usage: " << getMemPercent() << "%\n";

    // 3. Check a Service (e.g., cron or sshd)
    std::string service = "cron"; 
    bool isActive = isServiceActive(service);
    std::cout << "Service '" << service << "': " 
              << (isActive ? "Active (Running)" : "Inactive (Stopped/Missing)") << "\n";

    return 0; // Tell the OS the program exited successfully
}