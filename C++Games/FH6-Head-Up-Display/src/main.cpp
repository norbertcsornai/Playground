#include <atomic>  // codex-line-comment: documents this line.
#include <csignal>  // codex-line-comment: documents this line.
#include <iostream>  // codex-line-comment: documents this line.

#include "app/HudApplication.h"  // codex-line-comment: documents this line.

namespace {  // codex-line-comment: documents this line.

std::atomic_bool g_stopRequested{false};  // codex-line-comment: documents this line.
fh6::HudApplication* g_app{nullptr};  // codex-line-comment: documents this line.

void handleSignal(int) {  // codex-line-comment: documents this line.
  g_stopRequested = true;  // codex-line-comment: documents this line.
  if (g_app != nullptr) {  // codex-line-comment: documents this line.
    g_app->requestStop();  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

}  // namespace  // codex-line-comment: documents this line.

int main() {  // codex-line-comment: documents this line.
  std::signal(SIGINT, handleSignal);  // codex-line-comment: documents this line.
  std::signal(SIGTERM, handleSignal);  // codex-line-comment: documents this line.

  fh6::HudApplication app;  // codex-line-comment: documents this line.
  g_app = &app;  // codex-line-comment: documents this line.
  if (!app.initialize()) {  // codex-line-comment: documents this line.
    std::cerr << "Failed to initialize FH6 Head-Up Display.\n";  // codex-line-comment: documents this line.
    return 1;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  std::cout << "FH6 Head-Up Display is running. Press Ctrl+C to exit.\n";  // codex-line-comment: documents this line.

  app.run();  // codex-line-comment: documents this line.

  app.shutdown();  // codex-line-comment: documents this line.
  g_app = nullptr;  // codex-line-comment: documents this line.
  return 0;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.
