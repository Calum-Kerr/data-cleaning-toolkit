#include "backup.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

std::string getBackupDirectory() {
  std::string backupDir = "/tmp/toolkit_backups";
  fs::create_directories(backupDir);
  return backupDir;
}

void createBackup() {
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");

  std::string backupDir = getBackupDirectory();
  std::string backupName = backupDir + "/backup_" + ss.str() + ".tar.gz";

  std::string cmd = "tar -czf " + backupName + " /tmp/toolkit*.log 2>/dev/null";
  system(cmd.c_str());

  std::ofstream manifest(backupDir + "/manifest.txt", std::ios::app);
  manifest << "Backup created: " << ss.str() << " at " << backupName << "\n";
}
