#include "backup.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <unistd.h>
#include <sys/wait.h>
#include <glob.h>
#include <vector>
#include <iostream>
#include "time_safe.h"

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
  std::tm tm = localtime_safe(time);
  ss << std::put_time(&tm, "%Y%m%d_%H%M%S");

  std::string backupDir = getBackupDirectory();
  std::string backupName = backupDir + "/backup_" + ss.str() + ".tar.gz";

  // expand the glob for log files and build argv for execv
  glob_t glob_result;
  std::vector<std::string> tarArgs;
  tarArgs.push_back("tar");
  tarArgs.push_back("-czf");
  tarArgs.push_back(backupName);

  int gl = glob("/tmp/toolkit*.log", GLOB_NOSORT, nullptr, &glob_result);
  if (gl == 0) {
    for (size_t i = 0; i < glob_result.gl_pathc; i++) {
      tarArgs.push_back(glob_result.gl_pathv[i]);
    }
  }
  globfree(&glob_result);

  bool tarOk = false;
  if (tarArgs.size() > 3) {
    // build char* argv array for execv
    std::vector<char*> argv;
    for (auto& a : tarArgs) {
      argv.push_back(&a[0]);
    }
    argv.push_back(nullptr);

    pid_t pid = fork();
    if (pid == 0) {
      // child: execute tar directly, no shell
      execv("/bin/tar", argv.data());
      _exit(1);
    } else if (pid > 0) {
      // parent: wait for tar to finish
      int status;
      waitpid(pid, &status, 0);
      tarOk = WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }
  }

  std::ofstream manifest(backupDir + "/manifest.txt", std::ios::app);
  if (tarOk) {
    manifest << "Backup created: " << ss.str() << " at " << backupName << "\n";
  } else if (tarArgs.size() == 3) {
    manifest << "Backup skipped (no log files found): " << ss.str() << "\n";
  } else {
    manifest << "Backup FAILED: " << ss.str() << " at " << backupName << "\n";
    std::cerr << "toolkit backup failed at " << ss.str() << "\n";
  }
}
