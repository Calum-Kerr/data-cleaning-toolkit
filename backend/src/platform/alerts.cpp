#include "alerts.h"
#include <fstream>

void writeStartupAlert() {
  std::ofstream alert("/tmp/toolkit_alerts.log", std::ios::app);
  alert << "APP STARTED\n";
}

void writeDailySummary(int requests, int errors) {
  std::ofstream alert("/tmp/toolkit_alerts.log", std::ios::app);
  alert << "DAILY: " << requests << " requests, " << errors << " errors\n";
}
