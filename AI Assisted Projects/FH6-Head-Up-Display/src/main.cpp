#include <atomic>  // Imports the atomic standard library declarations used in this file.
#include <csignal>  // Imports the csignal standard library declarations used in this file.
#include <iostream>  // Imports the iostream standard library declarations used in this file.

#include "app/HudApplication.h"  // Imports project declarations from app/HudApplication.h.

namespace {  // Starts a file-local helper namespace.

std::atomic_bool g_stopRequested{false};  // Declares g_stopRequested and initializes it with false.
fh6::HudApplication* g_app{nullptr};  // Declares g_app and initializes it with nullptr.

void handleSignal(int) {  // Begins function handleSignal.
  g_stopRequested = true;  // Sets g_stopRequested to true.
  if (g_app != nullptr) {  // Guards the following work behind the condition g_app != nullptr.
    g_app->requestStop();  // Calls requestStop through g_app.
  }  // Ends the current code block.
}  // Ends the current code block.

}  // Ends the current code block.

int main() {  // Begins function main.
  std::signal(SIGINT, handleSignal);  // Executes std::signal(SIGINT, handleSignal).
  std::signal(SIGTERM, handleSignal);  // Executes std::signal(SIGTERM, handleSignal).

  fh6::HudApplication app;  // Declares app for use in this scope.
  g_app = &app;  // Sets g_app to &app.
  if (!app.initialize()) {  // Guards the following work behind the condition !app.initialize().
    std::cerr << "Failed to initialize FH6 Head-Up Display.\n";  // Writes "Failed to initialize FH6 Head-Up Display.\n" to standard error.
    return 1;  // Returns 1 to the caller.
  }  // Ends the current code block.

  std::cout << "FH6 Head-Up Display is running. Press Ctrl+C to exit.\n";  // Writes "FH6 Head-Up Display is running. Press Ctrl+C to exit.\n" to standard output.

  app.run();  // Calls run on app.

  app.shutdown();  // Calls shutdown on app.
  g_app = nullptr;  // Sets g_app to nullptr.
  return 0;  // Returns 0 to the caller.
}  // Ends the current code block.
