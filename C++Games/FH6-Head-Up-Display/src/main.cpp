#include <atomic>
#include <csignal>
#include <iostream>

#include "app/HudApplication.h"

namespace {

std::atomic_bool g_stopRequested{false};
fh6::HudApplication* g_app{nullptr};

void handleSignal(int) {
  g_stopRequested = true;
  if (g_app != nullptr) {
    g_app->requestStop();
  }
}

}  // namespace

int main() {
  std::signal(SIGINT, handleSignal);
  std::signal(SIGTERM, handleSignal);

  fh6::HudApplication app;
  g_app = &app;
  if (!app.initialize()) {
    std::cerr << "Failed to initialize FH6 Head-Up Display.\n";
    return 1;
  }

  std::cout << "FH6 Head-Up Display is running. Press Ctrl+C to exit.\n";

  app.run();

  app.shutdown();
  g_app = nullptr;
  return 0;
}
