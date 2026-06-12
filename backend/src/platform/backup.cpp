#include "backup.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <system_error>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <fcntl.h>
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

// Remove backup archives older than the 30-day retention window promised by
// the site's privacy policy. Caller must hold the backup lock so pruning is
// serialised with backup creation. Uses std::error_code overloads throughout;
// a file that cannot be inspected or removed is skipped, never fatal.
static void pruneOldBackups(const std::string& backupDir) {
  std::error_code iterEc;
  fs::directory_iterator it(backupDir, iterEc);
  if (iterEc) {
    return;
  }

  const std::string prefix = "backup_";
  const std::string suffix = ".tar.gz";
  const auto cutoff = fs::file_time_type::clock::now() - std::chrono::hours(24 * 30);
  std::ofstream manifest(backupDir + "/manifest.txt", std::ios::app);

  for (fs::directory_iterator end; it != end; it.increment(iterEc)) {
    if (iterEc) {
      break;
    }
    std::string name = it->path().filename().string();
    if (name.size() < prefix.size() + suffix.size() ||
        name.compare(0, prefix.size(), prefix) != 0 ||
        name.compare(name.size() - suffix.size(), suffix.size(), suffix) != 0) {
      continue;
    }

    std::error_code fileEc;
    if (!it->is_regular_file(fileEc) || fileEc) {
      continue;
    }
    auto lastWrite = it->last_write_time(fileEc);
    if (fileEc || lastWrite >= cutoff) {
      continue;
    }
    if (fs::remove(it->path(), fileEc) && !fileEc) {
      manifest << "Backup pruned (older than 30 days): " << name << "\n";
    }
  }
}

void createBackup() {
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  std::tm tm = localtime_safe(time);
  ss << std::put_time(&tm, "%Y%m%d_%H%M%S");

  std::string backupDir = getBackupDirectory();
  std::string backupName = backupDir + "/backup_" + ss.str() + ".tar.gz";

  // Take an advisory lock to prevent concurrent backup runs (TOCTOU defence).
  // The lock file lives alongside the backups; it is NOT a security boundary,
  // merely a coordination primitive — processes that ignore it can still race.
  std::string lockPath = backupDir + "/.backup.lock";
  int lockFd = open(lockPath.c_str(), O_CREAT | O_RDWR, 0600);
  if (lockFd == -1 || flock(lockFd, LOCK_EX | LOCK_NB) == -1) {
    std::cerr << "toolkit backup: could not acquire lock, skipping" << std::endl;
    if (lockFd != -1) close(lockFd);
    return;
  }

  // expand the glob for log files and build argv for execv
  glob_t glob_result;
  std::vector<std::string> tarArgs;
  tarArgs.push_back("tar");
  tarArgs.push_back("-czf");
  tarArgs.push_back(backupName);
  // --no-dereference prevents tar from following symlinks, mitigating
  // symlink-based TOCTOU attacks between glob() and execv().
  tarArgs.push_back("--no-dereference");

  int gl = glob("/tmp/toolkit*.log", GLOB_NOSORT, nullptr, &glob_result);
  if (gl == 0) {
    for (size_t i = 0; i < glob_result.gl_pathc; i++) {
      tarArgs.push_back(glob_result.gl_pathv[i]);
    }
  }
  globfree(&glob_result);

  bool tarOk = false;
  if (tarArgs.size() > 4) {
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
  } else if (tarArgs.size() == 4) {
    manifest << "Backup skipped (no log files found): " << ss.str() << "\n";
  } else {
    manifest << "Backup FAILED: " << ss.str() << " at " << backupName << "\n";
    std::cerr << "toolkit backup failed at " << ss.str() << "\n";
  }

  // Release lock
  close(lockFd);
}
