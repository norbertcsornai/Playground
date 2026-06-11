#include "capture/DesktopFrameCapture.h"  // Imports project declarations from capture/DesktopFrameCapture.h.

#include <algorithm>  // Imports the algorithm standard library declarations used in this file.
#include <chrono>  // Imports the chrono standard library declarations used in this file.
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
    : captureRateLimitFps_(captureRateLimitFps > 0 ? captureRateLimitFps : 30) {}  // Initializes constructor members with captureRateLimitFps_(captureRateLimitFps > 0 ? captureRateLimitFps : 30).

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
  lastCapture_ = now;  // Sets lastCapture_ to now.

  if (auto frame = captureFrameWithDxgi(now)) {  // Guards the following work behind the condition auto frame = captureFrameWithDxgi(now).
    return frame;  // Returns frame to the caller.
  }  // Ends the current code block.

  return captureFrameWithGdi(now);  // Returns captureFrameWithGdi(now) to the caller.
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

  D3D11_TEXTURE2D_DESC stagingDesc = desc;  // Sets D3D11_TEXTURE2D_DESC stagingDesc to desc.
  stagingDesc.BindFlags = 0;  // Sets stagingDesc.BindFlags to 0.
  stagingDesc.MiscFlags = 0;  // Sets stagingDesc.MiscFlags to 0.
  stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;  // Sets stagingDesc.CPUAccessFlags to D3D11_CPU_ACCESS_READ.
  stagingDesc.Usage = D3D11_USAGE_STAGING;  // Sets stagingDesc.Usage to D3D11_USAGE_STAGING.

  if (!dxgiState_->stagingTexture || dxgiState_->stagingWidth != desc.Width ||  // Guards the following work behind the condition !dxgiState_->stagingTexture || dxgiState_->stagingWidth != desc.Width ||.
      dxgiState_->stagingHeight != desc.Height) {  // Continues the staging texture size check.
    hr = dxgiState_->device->CreateTexture2D(&stagingDesc, nullptr, &dxgiState_->stagingTexture);  // Sets hr to dxgiState_->device->CreateTexture2D(&stagingDesc, nullptr, &dxgiState_->stagingTexture).
    if (FAILED(hr)) {  // Guards the following work behind the condition FAILED(hr).
      return std::nullopt;  // Returns std::nullopt to the caller.
    }  // Ends the current code block.
    dxgiState_->stagingWidth = desc.Width;  // Sets dxgiState_->stagingWidth to desc.Width.
    dxgiState_->stagingHeight = desc.Height;  // Sets dxgiState_->stagingHeight to desc.Height.
  }  // Ends the current code block.

  auto* stagingTexture = dxgiState_->stagingTexture.Get();  // Sets auto* stagingTexture to dxgiState_->stagingTexture.Get().
  dxgiState_->context->CopyResource(stagingTexture, acquiredTexture.Get());  // Executes dxgiState_->context->CopyResource(stagingTexture, acquiredTexture.Get()).

  D3D11_MAPPED_SUBRESOURCE mapped{};  // Declares mapped with value initialization.
  hr = dxgiState_->context->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &mapped);  // Sets hr to dxgiState_->context->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &mapped).
  if (FAILED(hr)) {  // Guards the following work behind the condition FAILED(hr).
    return std::nullopt;  // Returns std::nullopt to the caller.
  }  // Ends the current code block.

  std::vector<Color> pixels;  // Declares pixels for use in this scope.
  pixels.reserve(static_cast<std::size_t>(desc.Width * desc.Height));  // Calls reserve on pixels.

  const auto* rows = static_cast<const std::uint8_t*>(mapped.pData);  // Sets const auto* rows to static_cast<const std::uint8_t*>(mapped.pData).
  for (UINT y = 0; y < desc.Height; ++y) {  // Iterates with loop control UINT y = 0; y < desc.Height; ++y.
    const auto* row = rows + static_cast<std::size_t>(y) * mapped.RowPitch;  // Sets const auto* row to rows + static_cast<std::size_t>(y) * mapped.RowPitch.
    for (UINT x = 0; x < desc.Width; ++x) {  // Iterates with loop control UINT x = 0; x < desc.Width; ++x.
      const auto* bgra = row + static_cast<std::size_t>(x) * 4;  // Sets const auto* bgra to row + static_cast<std::size_t>(x) * 4.
      pixels.push_back(Color{bgra[2], bgra[1], bgra[0], bgra[3]});  // Calls push_back on pixels.
    }  // Ends the current code block.
  }  // Ends the current code block.

  dxgiState_->context->Unmap(stagingTexture, 0);  // Executes dxgiState_->context->Unmap(stagingTexture, 0).
  return Frame(static_cast<int>(desc.Width), static_cast<int>(desc.Height), std::move(pixels),  // Supplies return Frame(static_cast<int>(desc.Width), static_cast<int>(desc.Height), std::move(pix... to the surrounding call or initializer.
               timestamp);  // Executes timestamp).
#endif  // Ends the compile-time selection block.
}  // Ends the current code block.

std::optional<Frame> DesktopFrameCapture::captureFrameWithGdi(TimePoint timestamp) const {  // Implements DesktopFrameCapture::captureFrameWithGdi.
#ifndef _WIN32  // Keeps the following code only when _WIN32 is not defined.
  (void)timestamp;  // Marks timestamp as intentionally unused in this build path.
  return std::nullopt;  // Returns std::nullopt to the caller.
#else  // Selects this compile-time branch when earlier branches were not selected.
  const int width = display_.bounds.width;  // Sets const int width to display_.bounds.width.
  const int height = display_.bounds.height;  // Sets const int height to display_.bounds.height.
  if (width <= 0 || height <= 0) {  // Guards the following work behind the condition width <= 0 || height <= 0.
    return std::nullopt;  // Returns std::nullopt to the caller.
  }  // Ends the current code block.

  HDC screenDc = GetDC(nullptr);  // Sets HDC screenDc to GetDC(nullptr).
  HDC memoryDc = CreateCompatibleDC(screenDc);  // Sets HDC memoryDc to CreateCompatibleDC(screenDc).
  HBITMAP bitmap = CreateCompatibleBitmap(screenDc, width, height);  // Sets HBITMAP bitmap to CreateCompatibleBitmap(screenDc, width, height).
  HGDIOBJ oldBitmap = SelectObject(memoryDc, bitmap);  // Sets HGDIOBJ oldBitmap to SelectObject(memoryDc, bitmap).

  const BOOL copied = BitBlt(memoryDc, 0, 0, width, height, screenDc, display_.bounds.x,  // Supplies const BOOL copied = BitBlt(memoryDc, 0, 0, width, height, screenDc, display_.bounds.x to the surrounding call or initializer.
                            display_.bounds.y, SRCCOPY);  // Executes display_.bounds.y, SRCCOPY).

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

  std::vector<Color> pixels;  // Declares pixels for use in this scope.
  pixels.reserve(static_cast<std::size_t>(width * height));  // Calls reserve on pixels.
  for (std::size_t i = 0; i < raw.size(); i += 4) {  // Iterates with loop control std::size_t i = 0; i < raw.size(); i += 4.
    pixels.push_back(Color{raw[i + 2], raw[i + 1], raw[i], raw[i + 3]});  // Calls push_back on pixels.
  }  // Ends the current code block.

  return Frame(width, height, std::move(pixels), timestamp);  // Returns Frame(width, height, std::move(pixels), timestamp) to the caller.
#endif  // Ends the compile-time selection block.
}  // Ends the current code block.

}  // Ends the current code block.
