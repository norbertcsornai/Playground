#pragma once

namespace fh6 {

enum class FrameStatus {
  Idle,
  Capturing,
  Unavailable,
  Failed,
};

enum class OverlayStatus {
  Hidden,
  Visible,
  Failed,
};

}  // namespace fh6
