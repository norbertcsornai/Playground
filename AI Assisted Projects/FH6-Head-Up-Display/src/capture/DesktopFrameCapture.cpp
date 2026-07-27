#include "capture/DesktopFrameCapture.h"  // Imports project declarations from capture/DesktopFrameCapture.h.

#include <algorithm>  // Imports the algorithm standard library declarations used in this file.
#include <chrono>  // Imports the chrono standard library declarations used in this file.
#include <cmath>  // Imports the cmath standard library declarations used in this file.
#include <cstdint>  // Imports the cstdint standard library declarations used in this file.
#include <limits>  // Imports the limits standard library declarations used in this file.
#include <vector>  // Imports the vector standard library declarations used in this file.

#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
#include <d3d11.h>  // Imports the d3d11.h standard library declarations used in this file.
#include <dxgi1_2.h>  // Imports the dxgi1_2.h standard library declarations used in this file.
#include <wrl/client.h>  // Imports the wrl/client.h standard library declarations used in this file.
#include <windows.h>  // Imports the windows.h standard library declarations used in this file.
#endif  // Ends the compile-time selection block.

namespace fh6 {  // Places the following declarations inside namespace fh6.

#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
namespace {  // Starts a file-local helper namespace.

using Microsoft::WRL::ComPtr;  // Allows COM smart pointers to be written as ComPtr in this file.

struct DesktopRect {  // Declares the DesktopRect value type and fields.
  int left{0};  // Declares left and initializes it with 0.
  int top{0};  // Declares top and initializes it with 0.
  int right{0};  // Declares right and initializes it with 0.
  int bottom{0};  // Declares bottom and initializes it with 0.
};  // Ends the current type, struct, or initializer declaration.

int overlapArea(const Rect& a, const DesktopRect& b) {  // Begins function overlapArea.
  const int left = std::max(a.x, b.left);  // Sets const int left to std::max(a.x, b.left).
  const int top = std::max(a.y, b.top);  // Sets const int top to std::max(a.y, b.top).
  const int right = std::min(a.x + a.width, b.right);  // Sets const int right to std::min(a.x + a.width, b.right).
  const int bottom = std::min(a.y + a.height, b.bottom);  // Sets const int bottom to std::min(a.y + a.height, b.bottom).
  if (right <= left || bottom <= top) {  // Guards the following work behind the condition right <= left || bottom <= top.
    return 0;  // Returns 0 to the caller.
  }  // Ends the current code block.
  return (right - left) * (bottom - top);  // Returns (right - left) * (bottom - top) to the caller.
}  // Ends the current code block.

}  // Ends the current code block.

struct DesktopFrameCapture::DxgiCaptureState {  // Declares the DesktopFrameCapture value type and fields.
  ComPtr<ID3D11Device> device;  // Declares device for use in this scope.
  ComPtr<ID3D11DeviceContext> context;  // Declares context for use in this scope.
  ComPtr<IDXGIOutputDuplication> duplication;  // Declares duplication for use in this scope.
  ComPtr<ID3D11Texture2D> stagingTexture;  // Declares stagingTexture for use in this scope.
  DXGI_OUTPUT_DESC outputDesc{};  // Declares outputDesc with value initialization.
  UINT stagingWidth{0};  // Declares stagingWidth and initializes it with 0.
  UINT stagingHeight{0};  // Declares stagingHeight and initializes it with 0.
  int width{0};  // Declares width and initializes it with 0.
  int height{0};  // Declares height and initializes it with 0.
};  // Ends the current type, struct, or initializer declaration.
#else  // Selects this compile-time branch when earlier branches were not selected.
struct DesktopFrameCapture::DxgiCaptureState {};  // Declares the DesktopFrameCapture value type and fields.
#endif  // Ends the compile-time selection block.

DesktopFrameCapture::DesktopFrameCapture(int captureRateLimitFps)  // Begins the multi-line constructor definition for DesktopFrameCapture.
    : captureRateLimitFps_(captureRateLimitFps > 0 ? captureRateLimitFps : 120) {}  // Initializes constructor members with captureRateLimitFps_(captureRateLimitFps > 0 ? captureRateLimitFps : 120).

DesktopFrameCapture::~DesktopFrameCapture() {  // Starts a multi-line initializer or scope for DesktopFrameCapture::~DesktopFrameCapture().
  stop();  // Invokes stop with the supplied arguments.
}  // Ends the current code block.

bool DesktopFrameCapture::start(const DisplayInfo& display) {  // Implements DesktopFrameCapture::start.
  display_ = display;  // Sets display_ to display.
  available_ = !display.bounds.empty();  // Sets available_ to !display.bounds.empty().
  dxgiState_.reset();  // Calls reset on dxgiState_.

#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
  (void)initializeDxgiCapture();  // Executes (void)initializeDxgiCapture().
#endif  // Ends the compile-time selection block.

  return available_;  // Returns available_ to the caller.
}  // Ends the current code block.

std::optional<Frame> DesktopFrameCapture::captureFrame() {  // Implements DesktopFrameCapture::captureFrame.
  if (!available_) {  // Guards the following work behind the condition !available_.
    return std::nullopt;  // Returns std::nullopt to the caller.
  }  // Ends the current code block.

  const auto now = Clock::now();  // Sets const auto now to Clock::now().
  const auto minInterval = std::chrono::milliseconds(1000 / captureRateLimitFps_);  // Sets const auto minInterval to std::chrono::milliseconds(1000 / captureRateLimitFps_).
  if (lastCapture_ != TimePoint{} && now - lastCapture_ < minInterval) {  // Guards the following work behind the condition lastCapture_ != TimePoint{} && now - lastCapture_ < minInterval.
    return std::nullopt;  // Returns std::nullopt to the caller.
  }  // Ends the current code block.

  // Only stamp lastCapture_ once a frame is actually obtained. Stamping it unconditionally here
  // meant a single DXGI WAIT_TIMEOUT (no new compositor frame yet) cost a full extra capture
  // interval before the next retry, doubling worst-case latency for no reason -- the caller's
  // poll loop already retries every few milliseconds, faster than the compositor produces frames.
  if (auto frame = captureFrameWithDxgi(now)) {  // Guards the following work behind the condition auto frame = captureFrameWithDxgi(now).
    lastCapture_ = now;  // Sets lastCapture_ to now.
    return frame;  // Returns frame to the caller.
  }  // Ends the current code block.

  if (auto frame = captureFrameWithGdi(now)) {  // Guards the following work behind the condition auto frame = captureFrameWithGdi(now).
    lastCapture_ = now;  // Sets lastCapture_ to now.
    return frame;  // Returns frame to the caller.
  }  // Ends the current code block.

  return std::nullopt;  // Returns std::nullopt to the caller.
}  // Ends the current code block.

void DesktopFrameCapture::stop() {  // Implements DesktopFrameCapture::stop.
  dxgiState_.reset();  // Calls reset on dxgiState_.
  available_ = false;  // Sets available_ to false.
}  // Ends the current code block.

bool DesktopFrameCapture::isAvailable() const {  // Implements DesktopFrameCapture::isAvailable.
  return available_;  // Returns available_ to the caller.
}  // Ends the current code block.

void DesktopFrameCapture::handleDisplayChanged(const DisplayInfo& display) {  // Implements DesktopFrameCapture::handleDisplayChanged.
  display_ = display;  // Sets display_ to display.
  available_ = !display.bounds.empty();  // Sets available_ to !display.bounds.empty().
  dxgiState_.reset();  // Calls reset on dxgiState_.
#ifdef _WIN32  // Keeps the following code only when _WIN32 is defined.
  (void)initializeDxgiCapture();  // Executes (void)initializeDxgiCapture().
#endif  // Ends the compile-time selection block.
}  // Ends the current code block.

void DesktopFrameCapture::setCaptureRateLimit(int captureRateLimitFps) {  // Implements DesktopFrameCapture::setCaptureRateLimit.
  if (captureRateLimitFps > 0) {  // Guards the following work behind the condition captureRateLimitFps > 0.
    captureRateLimitFps_ = captureRateLimitFps;  // Sets captureRateLimitFps_ to captureRateLimitFps.
  }  // Ends the current code block.
}  // Ends the current code block.

void DesktopFrameCapture::setRegionOfInterest(const Rect& region) {  // Implements DesktopFrameCapture::setRegionOfInterest.
  regionOfInterest_ = region;  // Sets regionOfInterest_ to region.
}  // Ends the current code block.

// Returns the rect to capture in display-local coordinates, where {0,0} is the display's top-left
// corner. This matches the coordinate space CalibrationService produces gear regions in, and the
// space Frame::origin() is expressed in, so no desktop-origin offset is involved on either side.
Rect DesktopFrameCapture::resolveCaptureRect() const {  // Implements DesktopFrameCapture::resolveCaptureRect.
  const Rect fullDisplay{0, 0, display_.bounds.width, display_.bounds.height};  // Declares fullDisplay covering the whole display in display-local coordinates.
  if (regionOfInterest_.empty()) {  // Guards the following work behind the condition regionOfInterest_.empty().
    return fullDisplay;  // Returns the whole display when no region of interest was requested.
  }  // Ends the current code block.

  // Grab a little more than asked for. The consumer recomputes its region from the returned frame's
  // source dimensions, and any rounding between coordinate spaces could otherwise leave that region
  // reaching a pixel or two past what was captured. The padding is negligible next to a full screen.
  constexpr int kPadding = 16;  // Defines compile-time constant kPadding as 16.

  // Intersect the padded region with the display so we never read outside the captured surface.
  const int left = std::max(regionOfInterest_.x - kPadding, 0);  // Sets const int left to the intersected left edge.
  const int top = std::max(regionOfInterest_.y - kPadding, 0);  // Sets const int top to the intersected top edge.
  const int right = std::min(regionOfInterest_.x + regionOfInterest_.width + kPadding,  // Sets const int right to the intersected right edge.
                             fullDisplay.width);  // Executes fullDisplay.width).
  const int bottom = std::min(regionOfInterest_.y + regionOfInterest_.height + kPadding,  // Sets const int bottom to the intersected bottom edge.
                              fullDisplay.height);  // Executes fullDisplay.height).

  if (right <= left || bottom <= top) {  // Guards the following work behind an empty intersection.
    return fullDisplay;  // Falls back to the whole display when the region does not overlap it.
  }  // Ends the current code block.

  return Rect{left, top, right - left, bottom - top};  // Returns the intersected capture rect to the caller.
}  // Ends the current code block.

bool DesktopFrameCapture::initializeDxgiCapture() {  // Implements DesktopFrameCapture::initializeDxgiCapture.
#ifndef _WIN32  // Keeps the following code only when _WIN32 is not defined.
  return false;  // Returns false to the caller.
#else  // Selects this compile-time branch when earlier branches were not selected.
  if (display_.bounds.empty()) {  // Guards the following work behind the condition display_.bounds.empty().
    return false;  // Returns false to the caller.
  }  // Ends the current code block.

  auto state = std::make_unique<DxgiCaptureState>();  // Sets auto state to std::make_unique<DxgiCaptureState>().

  const D3D_FEATURE_LEVEL featureLevels[] = {  // Starts a multi-line initializer or scope for const D3D_FEATURE_LEVEL featureLevels[] =.
      D3D_FEATURE_LEVEL_11_1,  // Supplies D3D_FEATURE_LEVEL_11_1 to the surrounding call or initializer.
      D3D_FEATURE_LEVEL_11_0,  // Supplies D3D_FEATURE_LEVEL_11_0 to the surrounding call or initializer.
  };  // Ends the current type, struct, or initializer declaration.
  D3D_FEATURE_LEVEL selectedFeatureLevel{};  // Declares selectedFeatureLevel with value initialization.
  UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;  // Sets UINT flags to D3D11_CREATE_DEVICE_BGRA_SUPPORT.

  HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels,  // Supplies HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featu... to the surrounding call or initializer.
                                 static_cast<UINT>(sizeof(featureLevels) / sizeof(featureLevels[0])),  // Supplies static_cast<UINT>(sizeof(featureLevels) / sizeof(featureLevels[0])) to the surrounding call or initializer.
                                 D3D11_SDK_VERSION,  // Supplies D3D11_SDK_VERSION to the surrounding call or initializer.
                                 &state->device, &selectedFeatureLevel, &state->context);  // Executes &state->device, &selectedFeatureLevel, &state->context).
  if (FAILED(hr)) {  // Guards the following work behind the condition FAILED(hr).
    return false;  // Returns false to the caller.
  }  // Ends the current code block.

  ComPtr<IDXGIDevice> dxgiDevice;  // Declares dxgiDevice for use in this scope.
  hr = state->device.As(&dxgiDevice);  // Sets hr to state->device.As(&dxgiDevice).
  if (FAILED(hr)) {  // Guards the following work behind the condition FAILED(hr).
    return false;  // Returns false to the caller.
  }  // Ends the current code block.

  ComPtr<IDXGIAdapter> adapter;  // Declares adapter for use in this scope.
  hr = dxgiDevice->GetAdapter(&adapter);  // Sets hr to dxgiDevice->GetAdapter(&adapter).
  if (FAILED(hr)) {  // Guards the following work behind the condition FAILED(hr).
    return false;  // Returns false to the caller.
  }  // Ends the current code block.

  ComPtr<IDXGIOutput> bestOutput;  // Declares bestOutput for use in this scope.
  DXGI_OUTPUT_DESC bestDesc{};  // Declares bestDesc with value initialization.
  int bestOverlap = -1;  // Sets int bestOverlap to -1.

  for (UINT index = 0;; ++index) {  // Iterates with loop control UINT index = 0;; ++index.
    ComPtr<IDXGIOutput> output;  // Declares output for use in this scope.
    if (adapter->EnumOutputs(index, &output) == DXGI_ERROR_NOT_FOUND) {  // Guards the following work behind the condition adapter->EnumOutputs(index, &output) == DXGI_ERROR_NOT_FOUND.
      break;  // Stops executing this switch branch or loop.
    }  // Ends the current code block.

    DXGI_OUTPUT_DESC desc{};  // Declares desc with value initialization.
    if (FAILED(output->GetDesc(&desc))) {  // Guards the following work behind the condition FAILED(output->GetDesc(&desc)).
      continue;  // Skips the rest of this loop iteration.
    }  // Ends the current code block.

    const DesktopRect outputRect{  // Starts a multi-line initializer or scope for const DesktopRect outputRect.
        desc.DesktopCoordinates.left,  // Supplies desc.DesktopCoordinates.left to the surrounding call or initializer.
        desc.DesktopCoordinates.top,  // Supplies desc.DesktopCoordinates.top to the surrounding call or initializer.
        desc.DesktopCoordinates.right,  // Supplies desc.DesktopCoordinates.right to the surrounding call or initializer.
        desc.DesktopCoordinates.bottom,  // Supplies desc.DesktopCoordinates.bottom to the surrounding call or initializer.
    };  // Ends the current type, struct, or initializer declaration.
    const int area = overlapArea(display_.bounds, outputRect);  // Sets const int area to overlapArea(display_.bounds, outputRect).
    if (area > bestOverlap) {  // Guards the following work behind the condition area > bestOverlap.
      bestOverlap = area;  // Sets bestOverlap to area.
      bestOutput = output;  // Sets bestOutput to output.
      bestDesc = desc;  // Sets bestDesc to desc.
    }  // Ends the current code block.
  }  // Ends the current code block.

  if (!bestOutput || bestOverlap <= 0) {  // Guards the following work behind the condition !bestOutput || bestOverlap <= 0.
    return false;  // Returns false to the caller.
  }  // Ends the current code block.

  ComPtr<IDXGIOutput1> output1;  // Declares output1 for use in this scope.
  hr = bestOutput.As(&output1);  // Sets hr to bestOutput.As(&output1).
  if (FAILED(hr)) {  // Guards the following work behind the condition FAILED(hr).
    return false;  // Returns false to the caller.
  }  // Ends the current code block.

  hr = output1->DuplicateOutput(state->device.Get(), &state->duplication);  // Sets hr to output1->DuplicateOutput(state->device.Get(), &state->duplication).
  if (FAILED(hr)) {  // Guards the following work behind the condition FAILED(hr).
    return false;  // Returns false to the caller.
  }  // Ends the current code block.

  state->outputDesc = bestDesc;  // Sets state->outputDesc to bestDesc.
  state->width = bestDesc.DesktopCoordinates.right - bestDesc.DesktopCoordinates.left;  // Sets state->width to bestDesc.DesktopCoordinates.right - bestDesc.DesktopCoordinates.left.
  state->height = bestDesc.DesktopCoordinates.bottom - bestDesc.DesktopCoordinates.top;  // Sets state->height to bestDesc.DesktopCoordinates.bottom - bestDesc.DesktopCoordinates.top.
  dxgiState_ = std::move(state);  // Sets dxgiState_ to std::move(state).
  return true;  // Returns true to the caller.
#endif  // Ends the compile-time selection block.
}  // Ends the current code block.

std::optional<Frame> DesktopFrameCapture::captureFrameWithDxgi(TimePoint timestamp) {  // Implements DesktopFrameCapture::captureFrameWithDxgi.
#ifndef _WIN32  // Keeps the following code only when _WIN32 is not defined.
  (void)timestamp;  // Marks timestamp as intentionally unused in this build path.
  return std::nullopt;  // Returns std::nullopt to the caller.
#else  // Selects this compile-time branch when earlier branches were not selected.
  if (!dxgiState_ || !dxgiState_->duplication) {  // Guards the following work behind the condition !dxgiState_ || !dxgiState_->duplication.
    return std::nullopt;  // Returns std::nullopt to the caller.
  }  // Ends the current code block.

  DXGI_OUTDUPL_FRAME_INFO frameInfo{};  // Declares frameInfo with value initialization.
  ComPtr<IDXGIResource> desktopResource;  // Declares desktopResource for use in this scope.
  HRESULT hr = dxgiState_->duplication->AcquireNextFrame(0, &frameInfo, &desktopResource);  // Sets HRESULT hr to dxgiState_->duplication->AcquireNextFrame(0, &frameInfo, &desktopResource).
  if (hr == DXGI_ERROR_WAIT_TIMEOUT) {  // Guards the following work behind the condition hr == DXGI_ERROR_WAIT_TIMEOUT.
    return std::nullopt;  // Returns std::nullopt to the caller.
  }  // Ends the current code block.

  if (hr == DXGI_ERROR_ACCESS_LOST) {  // Guards the following work behind the condition hr == DXGI_ERROR_ACCESS_LOST.
    dxgiState_.reset();  // Calls reset on dxgiState_.
    (void)initializeDxgiCapture();  // Executes (void)initializeDxgiCapture().
    return std::nullopt;  // Returns std::nullopt to the caller.
  }  // Ends the current code block.

  if (FAILED(hr)) {  // Guards the following work behind the condition FAILED(hr).
    return std::nullopt;  // Returns std::nullopt to the caller.
  }  // Ends the current code block.

  struct FrameRelease {  // Declares the FrameRelease value type and fields.
    IDXGIOutputDuplication* duplication{nullptr};  // Declares duplication and initializes it with nullptr.
    ~FrameRelease() {  // Begins the FrameRelease destructor body.
      if (duplication != nullptr) {  // Guards the following work behind the condition duplication != nullptr.
        duplication->ReleaseFrame();  // Calls ReleaseFrame through duplication.
      }  // Ends the current code block.
    }  // Ends the current code block.
  } release{dxgiState_->duplication.Get()};  // Declares release and initializes it with dxgiState_->duplication.Get().

  ComPtr<ID3D11Texture2D> acquiredTexture;  // Declares acquiredTexture for use in this scope.
  hr = desktopResource.As(&acquiredTexture);  // Sets hr to desktopResource.As(&acquiredTexture).
  if (FAILED(hr)) {  // Guards the following work behind the condition FAILED(hr).
    return std::nullopt;  // Returns std::nullopt to the caller.
  }  // Ends the current code block.

  D3D11_TEXTURE2D_DESC desc{};  // Declares desc with value initialization.
  acquiredTexture->GetDesc(&desc);  // Calls GetDesc through acquiredTexture.

  // resolveCaptureRect() works in the coordinate space of the reported display bounds, which can
  // differ from the duplicated texture's own pixel grid when DPI virtualization is in play. Scale
  // into texture space so the copy box addresses the right pixels either way; when the two spaces
  // already agree (the normal DPI-aware case) both scale factors are exactly 1.
  const Rect captureRect = resolveCaptureRect();  // Sets const Rect captureRect to resolveCaptureRect().
  const double scaleX = display_.bounds.width > 0  // Sets const double scaleX to the display-to-texture horizontal ratio.
                            ? static_cast<double>(desc.Width) / display_.bounds.width  // Supplies the computed ratio when the display width is known.
                            : 1.0;  // Falls back to an identity ratio when the display width is unknown.
  const double scaleY = display_.bounds.height > 0  // Sets const double scaleY to the display-to-texture vertical ratio.
                            ? static_cast<double>(desc.Height) / display_.bounds.height  // Supplies the computed ratio when the display height is known.
                            : 1.0;  // Falls back to an identity ratio when the display height is unknown.

  const int texWidth = static_cast<int>(desc.Width);  // Sets const int texWidth to the texture width.
  const int texHeight = static_cast<int>(desc.Height);  // Sets const int texHeight to the texture height.
  const UINT boxLeft = static_cast<UINT>(  // Sets const UINT boxLeft to the clamped copy-box left edge.
      std::clamp(static_cast<int>(std::floor(captureRect.x * scaleX)), 0, texWidth));  // Executes std::clamp(static_cast<int>(std::floor(captureRect.x * scaleX)), 0, texWidth)).
  const UINT boxTop = static_cast<UINT>(  // Sets const UINT boxTop to the clamped copy-box top edge.
      std::clamp(static_cast<int>(std::floor(captureRect.y * scaleY)), 0, texHeight));  // Executes std::clamp(static_cast<int>(std::floor(captureRect.y * scaleY)), 0, texHeight)).
  const UINT boxRight = static_cast<UINT>(std::clamp(  // Sets const UINT boxRight to the clamped copy-box right edge.
      static_cast<int>(std::ceil((captureRect.x + captureRect.width) * scaleX)), 0, texWidth));  // Executes static_cast<int>(std::ceil((captureRect.x + captureRect.width) * scaleX)), 0, texWidth)).
  const UINT boxBottom = static_cast<UINT>(std::clamp(  // Sets const UINT boxBottom to the clamped copy-box bottom edge.
      static_cast<int>(std::ceil((captureRect.y + captureRect.height) * scaleY)), 0, texHeight));  // Executes static_cast<int>(std::ceil((captureRect.y + captureRect.height) * scaleY)), 0, texHeight)).
  if (boxRight <= boxLeft || boxBottom <= boxTop) {  // Guards the following work behind an empty copy box.
    return std::nullopt;  // Returns std::nullopt to the caller.
  }  // Ends the current code block.

  const UINT regionWidth = boxRight - boxLeft;  // Sets const UINT regionWidth to the copy box width.
  const UINT regionHeight = boxBottom - boxTop;  // Sets const UINT regionHeight to the copy box height.

  D3D11_TEXTURE2D_DESC stagingDesc = desc;  // Sets D3D11_TEXTURE2D_DESC stagingDesc to desc.
  stagingDesc.Width = regionWidth;  // Sizes the staging texture to the region instead of the whole output.
  stagingDesc.Height = regionHeight;  // Sizes the staging texture to the region instead of the whole output.
  stagingDesc.BindFlags = 0;  // Sets stagingDesc.BindFlags to 0.
  stagingDesc.MiscFlags = 0;  // Sets stagingDesc.MiscFlags to 0.
  stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;  // Sets stagingDesc.CPUAccessFlags to D3D11_CPU_ACCESS_READ.
  stagingDesc.Usage = D3D11_USAGE_STAGING;  // Sets stagingDesc.Usage to D3D11_USAGE_STAGING.

  if (!dxgiState_->stagingTexture || dxgiState_->stagingWidth != regionWidth ||  // Guards the following work behind a staging texture that does not match the region.
      dxgiState_->stagingHeight != regionHeight) {  // Continues the staging texture size check.
    dxgiState_->stagingTexture.Reset();  // Releases the previous staging texture before resizing.
    hr = dxgiState_->device->CreateTexture2D(&stagingDesc, nullptr, &dxgiState_->stagingTexture);  // Sets hr to dxgiState_->device->CreateTexture2D(&stagingDesc, nullptr, &dxgiState_->stagingTexture).
    if (FAILED(hr)) {  // Guards the following work behind the condition FAILED(hr).
      return std::nullopt;  // Returns std::nullopt to the caller.
    }  // Ends the current code block.
    dxgiState_->stagingWidth = regionWidth;  // Sets dxgiState_->stagingWidth to regionWidth.
    dxgiState_->stagingHeight = regionHeight;  // Sets dxgiState_->stagingHeight to regionHeight.
  }  // Ends the current code block.

  // Copying only the region keeps GPU-to-CPU traffic proportional to the HUD area rather than to
  // the whole screen, which dominates per-frame cost at high resolutions.
  const D3D11_BOX sourceBox{boxLeft, boxTop, 0, boxRight, boxBottom, 1};  // Declares sourceBox describing the region to copy.
  auto* stagingTexture = dxgiState_->stagingTexture.Get();  // Sets auto* stagingTexture to dxgiState_->stagingTexture.Get().
  dxgiState_->context->CopySubresourceRegion(stagingTexture, 0, 0, 0, 0, acquiredTexture.Get(), 0,  // Supplies the destination and source arguments for the region copy.
                                             &sourceBox);  // Executes &sourceBox).

  D3D11_MAPPED_SUBRESOURCE mapped{};  // Declares mapped with value initialization.
  hr = dxgiState_->context->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &mapped);  // Sets hr to dxgiState_->context->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &mapped).
  if (FAILED(hr)) {  // Guards the following work behind the condition FAILED(hr).
    return std::nullopt;  // Returns std::nullopt to the caller.
  }  // Ends the current code block.

  std::vector<Color> pixels(static_cast<std::size_t>(regionWidth) * regionHeight);  // Sizes the pixel buffer up front so the loop can write by index.

  const auto* rows = static_cast<const std::uint8_t*>(mapped.pData);  // Sets const auto* rows to static_cast<const std::uint8_t*>(mapped.pData).
  for (UINT y = 0; y < regionHeight; ++y) {  // Iterates with loop control UINT y = 0; y < regionHeight; ++y.
    const auto* row = rows + static_cast<std::size_t>(y) * mapped.RowPitch;  // Sets const auto* row to rows + static_cast<std::size_t>(y) * mapped.RowPitch.
    auto* destination = pixels.data() + static_cast<std::size_t>(y) * regionWidth;  // Sets auto* destination to this row's first output pixel.
    for (UINT x = 0; x < regionWidth; ++x) {  // Iterates with loop control UINT x = 0; x < regionWidth; ++x.
      const auto* bgra = row + static_cast<std::size_t>(x) * 4;  // Sets const auto* bgra to row + static_cast<std::size_t>(x) * 4.
      destination[x] = Color{bgra[2], bgra[1], bgra[0], bgra[3]};  // Sets destination[x] to the converted RGBA pixel.
    }  // Ends the current code block.
  }  // Ends the current code block.

  dxgiState_->context->Unmap(stagingTexture, 0);  // Executes dxgiState_->context->Unmap(stagingTexture, 0).

  const Rect origin{static_cast<int>(boxLeft), static_cast<int>(boxTop),  // Declares origin describing where these pixels sit in display-local coordinates.
                    static_cast<int>(regionWidth), static_cast<int>(regionHeight)};  // Finishes this initializer entry for the surrounding aggregate.
  return Frame(origin, static_cast<int>(desc.Width), static_cast<int>(desc.Height),  // Supplies the sub-rectangle and full display size to the Frame constructor.
               std::move(pixels), timestamp);  // Returns the captured sub-rectangle frame to the caller.
#endif  // Ends the compile-time selection block.
}  // Ends the current code block.

std::optional<Frame> DesktopFrameCapture::captureFrameWithGdi(TimePoint timestamp) const {  // Implements DesktopFrameCapture::captureFrameWithGdi.
#ifndef _WIN32  // Keeps the following code only when _WIN32 is not defined.
  (void)timestamp;  // Marks timestamp as intentionally unused in this build path.
  return std::nullopt;  // Returns std::nullopt to the caller.
#else  // Selects this compile-time branch when earlier branches were not selected.
  if (display_.bounds.width <= 0 || display_.bounds.height <= 0) {  // Guards the following work behind an empty display.
    return std::nullopt;  // Returns std::nullopt to the caller.
  }  // Ends the current code block.

  // Blit only the region of interest so this fallback path also scales with HUD area rather than
  // with screen area. captureRect is display-local, so offset by the display's desktop origin.
  const Rect captureRect = resolveCaptureRect();  // Sets const Rect captureRect to resolveCaptureRect().
  const int width = captureRect.width;  // Sets const int width to captureRect.width.
  const int height = captureRect.height;  // Sets const int height to captureRect.height.
  if (width <= 0 || height <= 0) {  // Guards the following work behind the condition width <= 0 || height <= 0.
    return std::nullopt;  // Returns std::nullopt to the caller.
  }  // Ends the current code block.

  HDC screenDc = GetDC(nullptr);  // Sets HDC screenDc to GetDC(nullptr).
  HDC memoryDc = CreateCompatibleDC(screenDc);  // Sets HDC memoryDc to CreateCompatibleDC(screenDc).
  HBITMAP bitmap = CreateCompatibleBitmap(screenDc, width, height);  // Sets HBITMAP bitmap to CreateCompatibleBitmap(screenDc, width, height).
  HGDIOBJ oldBitmap = SelectObject(memoryDc, bitmap);  // Sets HGDIOBJ oldBitmap to SelectObject(memoryDc, bitmap).

  const BOOL copied = BitBlt(memoryDc, 0, 0, width, height, screenDc,  // Supplies the destination and source device contexts for the blit.
                             display_.bounds.x + captureRect.x,  // Supplies the desktop-space source column to the surrounding call.
                             display_.bounds.y + captureRect.y, SRCCOPY);  // Executes display_.bounds.y + captureRect.y, SRCCOPY).

  BITMAPINFO bitmapInfo{};  // Declares bitmapInfo with value initialization.
  bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);  // Sets bitmapInfo.bmiHeader.biSize to sizeof(BITMAPINFOHEADER).
  bitmapInfo.bmiHeader.biWidth = width;  // Sets bitmapInfo.bmiHeader.biWidth to width.
  bitmapInfo.bmiHeader.biHeight = -height;  // Sets bitmapInfo.bmiHeader.biHeight to -height.
  bitmapInfo.bmiHeader.biPlanes = 1;  // Sets bitmapInfo.bmiHeader.biPlanes to 1.
  bitmapInfo.bmiHeader.biBitCount = 32;  // Sets bitmapInfo.bmiHeader.biBitCount to 32.
  bitmapInfo.bmiHeader.biCompression = BI_RGB;  // Sets bitmapInfo.bmiHeader.biCompression to BI_RGB.

  std::vector<std::uint8_t> raw(static_cast<std::size_t>(width * height * 4));  // Executes std::vector<std::uint8_t> raw(static_cast<std::size_t>(width * height * 4)).
  const int readRows = copied ? GetDIBits(memoryDc, bitmap, 0, static_cast<UINT>(height), raw.data(),  // Supplies const int readRows = copied ? GetDIBits(memoryDc, bitmap, 0, static_cast<UINT>(height),... to the surrounding call or initializer.
                                         &bitmapInfo, DIB_RGB_COLORS)  // Continues the surrounding declaration or control-flow expression.
                              : 0;  // Executes : 0.

  SelectObject(memoryDc, oldBitmap);  // Invokes SelectObject with the supplied arguments.
  DeleteObject(bitmap);  // Invokes DeleteObject with the supplied arguments.
  DeleteDC(memoryDc);  // Invokes DeleteDC with the supplied arguments.
  ReleaseDC(nullptr, screenDc);  // Invokes ReleaseDC with the supplied arguments.

  if (readRows == 0) {  // Guards the following work behind the condition readRows == 0.
    return std::nullopt;  // Returns std::nullopt to the caller.
  }  // Ends the current code block.

  std::vector<Color> pixels(static_cast<std::size_t>(width) * height);  // Sizes the pixel buffer up front so the loop can write by index.
  for (std::size_t i = 0; i < pixels.size(); ++i) {  // Iterates over each destination pixel.
    const std::size_t offset = i * 4;  // Sets const std::size_t offset to this pixel's byte offset in the raw buffer.
    pixels[i] = Color{raw[offset + 2], raw[offset + 1], raw[offset], raw[offset + 3]};  // Sets pixels[i] to the converted RGBA pixel.
  }  // Ends the current code block.

  return Frame(Rect{captureRect.x, captureRect.y, width, height}, display_.bounds.width,  // Supplies the sub-rectangle and full display size to the Frame constructor.
               display_.bounds.height, std::move(pixels), timestamp);  // Returns the captured sub-rectangle frame to the caller.
#endif  // Ends the compile-time selection block.
}  // Ends the current code block.

}  // Ends the current code block.
