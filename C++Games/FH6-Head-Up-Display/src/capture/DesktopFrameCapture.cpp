#include "capture/DesktopFrameCapture.h"  // codex-line-comment: documents this line.

#include <algorithm>  // codex-line-comment: documents this line.
#include <chrono>  // codex-line-comment: documents this line.
#include <cstdint>  // codex-line-comment: documents this line.
#include <limits>  // codex-line-comment: documents this line.
#include <vector>  // codex-line-comment: documents this line.

#ifdef _WIN32  // codex-line-comment: documents this line.
#include <d3d11.h>  // codex-line-comment: documents this line.
#include <dxgi1_2.h>  // codex-line-comment: documents this line.
#include <wrl/client.h>  // codex-line-comment: documents this line.
#include <windows.h>  // codex-line-comment: documents this line.
#endif  // codex-line-comment: documents this line.

namespace fh6 {  // codex-line-comment: documents this line.

#ifdef _WIN32  // codex-line-comment: documents this line.
namespace {  // codex-line-comment: documents this line.

using Microsoft::WRL::ComPtr;  // codex-line-comment: documents this line.

struct DesktopRect {  // codex-line-comment: documents this line.
  int left{0};  // codex-line-comment: documents this line.
  int top{0};  // codex-line-comment: documents this line.
  int right{0};  // codex-line-comment: documents this line.
  int bottom{0};  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.

int overlapArea(const Rect& a, const DesktopRect& b) {  // codex-line-comment: documents this line.
  const int left = std::max(a.x, b.left);  // codex-line-comment: documents this line.
  const int top = std::max(a.y, b.top);  // codex-line-comment: documents this line.
  const int right = std::min(a.x + a.width, b.right);  // codex-line-comment: documents this line.
  const int bottom = std::min(a.y + a.height, b.bottom);  // codex-line-comment: documents this line.
  if (right <= left || bottom <= top) {  // codex-line-comment: documents this line.
    return 0;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
  return (right - left) * (bottom - top);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

}  // namespace  // codex-line-comment: documents this line.

struct DesktopFrameCapture::DxgiCaptureState {  // codex-line-comment: documents this line.
  ComPtr<ID3D11Device> device;  // codex-line-comment: documents this line.
  ComPtr<ID3D11DeviceContext> context;  // codex-line-comment: documents this line.
  ComPtr<IDXGIOutputDuplication> duplication;  // codex-line-comment: documents this line.
  DXGI_OUTPUT_DESC outputDesc{};  // codex-line-comment: documents this line.
  int width{0};  // codex-line-comment: documents this line.
  int height{0};  // codex-line-comment: documents this line.
};  // codex-line-comment: documents this line.
#else  // codex-line-comment: documents this line.
struct DesktopFrameCapture::DxgiCaptureState {};  // codex-line-comment: documents this line.
#endif  // codex-line-comment: documents this line.

DesktopFrameCapture::DesktopFrameCapture(int captureRateLimitFps)  // codex-line-comment: documents this line.
    : captureRateLimitFps_(captureRateLimitFps > 0 ? captureRateLimitFps : 30) {}  // codex-line-comment: documents this line.

DesktopFrameCapture::~DesktopFrameCapture() {  // codex-line-comment: documents this line.
  stop();  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

bool DesktopFrameCapture::start(const DisplayInfo& display) {  // codex-line-comment: documents this line.
  display_ = display;  // codex-line-comment: documents this line.
  available_ = !display.bounds.empty();  // codex-line-comment: documents this line.
  dxgiState_.reset();  // codex-line-comment: documents this line.

#ifdef _WIN32  // codex-line-comment: documents this line.
  (void)initializeDxgiCapture();  // codex-line-comment: documents this line.
#endif  // codex-line-comment: documents this line.

  return available_;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

std::optional<Frame> DesktopFrameCapture::captureFrame() {  // codex-line-comment: documents this line.
  if (!available_) {  // codex-line-comment: documents this line.
    return std::nullopt;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  const auto now = Clock::now();  // codex-line-comment: documents this line.
  const auto minInterval = std::chrono::milliseconds(1000 / captureRateLimitFps_);  // codex-line-comment: documents this line.
  if (lastCapture_ != TimePoint{} && now - lastCapture_ < minInterval) {  // codex-line-comment: documents this line.
    return std::nullopt;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
  lastCapture_ = now;  // codex-line-comment: documents this line.

  if (auto frame = captureFrameWithDxgi(now)) {  // codex-line-comment: documents this line.
    return frame;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  return captureFrameWithGdi(now);  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void DesktopFrameCapture::stop() {  // codex-line-comment: documents this line.
  dxgiState_.reset();  // codex-line-comment: documents this line.
  available_ = false;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

bool DesktopFrameCapture::isAvailable() const {  // codex-line-comment: documents this line.
  return available_;  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void DesktopFrameCapture::handleDisplayChanged(const DisplayInfo& display) {  // codex-line-comment: documents this line.
  display_ = display;  // codex-line-comment: documents this line.
  available_ = !display.bounds.empty();  // codex-line-comment: documents this line.
  dxgiState_.reset();  // codex-line-comment: documents this line.
#ifdef _WIN32  // codex-line-comment: documents this line.
  (void)initializeDxgiCapture();  // codex-line-comment: documents this line.
#endif  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

void DesktopFrameCapture::setCaptureRateLimit(int captureRateLimitFps) {  // codex-line-comment: documents this line.
  if (captureRateLimitFps > 0) {  // codex-line-comment: documents this line.
    captureRateLimitFps_ = captureRateLimitFps;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

bool DesktopFrameCapture::initializeDxgiCapture() {  // codex-line-comment: documents this line.
#ifndef _WIN32  // codex-line-comment: documents this line.
  return false;  // codex-line-comment: documents this line.
#else  // codex-line-comment: documents this line.
  if (display_.bounds.empty()) {  // codex-line-comment: documents this line.
    return false;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  auto state = std::make_unique<DxgiCaptureState>();  // codex-line-comment: documents this line.

  const D3D_FEATURE_LEVEL featureLevels[] = {  // codex-line-comment: documents this line.
      D3D_FEATURE_LEVEL_11_1,  // codex-line-comment: documents this line.
      D3D_FEATURE_LEVEL_11_0,  // codex-line-comment: documents this line.
  };  // codex-line-comment: documents this line.
  D3D_FEATURE_LEVEL selectedFeatureLevel{};  // codex-line-comment: documents this line.
  UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;  // codex-line-comment: documents this line.

  HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels,  // codex-line-comment: documents this line.
                                 static_cast<UINT>(sizeof(featureLevels) / sizeof(featureLevels[0])),  // codex-line-comment: documents this line.
                                 D3D11_SDK_VERSION,  // codex-line-comment: documents this line.
                                 &state->device, &selectedFeatureLevel, &state->context);  // codex-line-comment: documents this line.
  if (FAILED(hr)) {  // codex-line-comment: documents this line.
    return false;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  ComPtr<IDXGIDevice> dxgiDevice;  // codex-line-comment: documents this line.
  hr = state->device.As(&dxgiDevice);  // codex-line-comment: documents this line.
  if (FAILED(hr)) {  // codex-line-comment: documents this line.
    return false;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  ComPtr<IDXGIAdapter> adapter;  // codex-line-comment: documents this line.
  hr = dxgiDevice->GetAdapter(&adapter);  // codex-line-comment: documents this line.
  if (FAILED(hr)) {  // codex-line-comment: documents this line.
    return false;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  ComPtr<IDXGIOutput> bestOutput;  // codex-line-comment: documents this line.
  DXGI_OUTPUT_DESC bestDesc{};  // codex-line-comment: documents this line.
  int bestOverlap = -1;  // codex-line-comment: documents this line.

  for (UINT index = 0;; ++index) {  // codex-line-comment: documents this line.
    ComPtr<IDXGIOutput> output;  // codex-line-comment: documents this line.
    if (adapter->EnumOutputs(index, &output) == DXGI_ERROR_NOT_FOUND) {  // codex-line-comment: documents this line.
      break;  // codex-line-comment: documents this line.
    }  // codex-line-comment: documents this line.

    DXGI_OUTPUT_DESC desc{};  // codex-line-comment: documents this line.
    if (FAILED(output->GetDesc(&desc))) {  // codex-line-comment: documents this line.
      continue;  // codex-line-comment: documents this line.
    }  // codex-line-comment: documents this line.

    const DesktopRect outputRect{  // codex-line-comment: documents this line.
        desc.DesktopCoordinates.left,  // codex-line-comment: documents this line.
        desc.DesktopCoordinates.top,  // codex-line-comment: documents this line.
        desc.DesktopCoordinates.right,  // codex-line-comment: documents this line.
        desc.DesktopCoordinates.bottom,  // codex-line-comment: documents this line.
    };  // codex-line-comment: documents this line.
    const int area = overlapArea(display_.bounds, outputRect);  // codex-line-comment: documents this line.
    if (area > bestOverlap) {  // codex-line-comment: documents this line.
      bestOverlap = area;  // codex-line-comment: documents this line.
      bestOutput = output;  // codex-line-comment: documents this line.
      bestDesc = desc;  // codex-line-comment: documents this line.
    }  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  if (!bestOutput || bestOverlap <= 0) {  // codex-line-comment: documents this line.
    return false;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  ComPtr<IDXGIOutput1> output1;  // codex-line-comment: documents this line.
  hr = bestOutput.As(&output1);  // codex-line-comment: documents this line.
  if (FAILED(hr)) {  // codex-line-comment: documents this line.
    return false;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  hr = output1->DuplicateOutput(state->device.Get(), &state->duplication);  // codex-line-comment: documents this line.
  if (FAILED(hr)) {  // codex-line-comment: documents this line.
    return false;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  state->outputDesc = bestDesc;  // codex-line-comment: documents this line.
  state->width = bestDesc.DesktopCoordinates.right - bestDesc.DesktopCoordinates.left;  // codex-line-comment: documents this line.
  state->height = bestDesc.DesktopCoordinates.bottom - bestDesc.DesktopCoordinates.top;  // codex-line-comment: documents this line.
  dxgiState_ = std::move(state);  // codex-line-comment: documents this line.
  return true;  // codex-line-comment: documents this line.
#endif  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

std::optional<Frame> DesktopFrameCapture::captureFrameWithDxgi(TimePoint timestamp) {  // codex-line-comment: documents this line.
#ifndef _WIN32  // codex-line-comment: documents this line.
  (void)timestamp;  // codex-line-comment: documents this line.
  return std::nullopt;  // codex-line-comment: documents this line.
#else  // codex-line-comment: documents this line.
  if (!dxgiState_ || !dxgiState_->duplication) {  // codex-line-comment: documents this line.
    return std::nullopt;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  DXGI_OUTDUPL_FRAME_INFO frameInfo{};  // codex-line-comment: documents this line.
  ComPtr<IDXGIResource> desktopResource;  // codex-line-comment: documents this line.
  HRESULT hr = dxgiState_->duplication->AcquireNextFrame(0, &frameInfo, &desktopResource);  // codex-line-comment: documents this line.
  if (hr == DXGI_ERROR_WAIT_TIMEOUT) {  // codex-line-comment: documents this line.
    return std::nullopt;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  if (hr == DXGI_ERROR_ACCESS_LOST) {  // codex-line-comment: documents this line.
    dxgiState_.reset();  // codex-line-comment: documents this line.
    (void)initializeDxgiCapture();  // codex-line-comment: documents this line.
    return std::nullopt;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  if (FAILED(hr)) {  // codex-line-comment: documents this line.
    return std::nullopt;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  struct FrameRelease {  // codex-line-comment: documents this line.
    IDXGIOutputDuplication* duplication{nullptr};  // codex-line-comment: documents this line.
    ~FrameRelease() {  // codex-line-comment: documents this line.
      if (duplication != nullptr) {  // codex-line-comment: documents this line.
        duplication->ReleaseFrame();  // codex-line-comment: documents this line.
      }  // codex-line-comment: documents this line.
    }  // codex-line-comment: documents this line.
  } release{dxgiState_->duplication.Get()};  // codex-line-comment: documents this line.

  ComPtr<ID3D11Texture2D> acquiredTexture;  // codex-line-comment: documents this line.
  hr = desktopResource.As(&acquiredTexture);  // codex-line-comment: documents this line.
  if (FAILED(hr)) {  // codex-line-comment: documents this line.
    return std::nullopt;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  D3D11_TEXTURE2D_DESC desc{};  // codex-line-comment: documents this line.
  acquiredTexture->GetDesc(&desc);  // codex-line-comment: documents this line.

  D3D11_TEXTURE2D_DESC stagingDesc = desc;  // codex-line-comment: documents this line.
  stagingDesc.BindFlags = 0;  // codex-line-comment: documents this line.
  stagingDesc.MiscFlags = 0;  // codex-line-comment: documents this line.
  stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;  // codex-line-comment: documents this line.
  stagingDesc.Usage = D3D11_USAGE_STAGING;  // codex-line-comment: documents this line.

  ComPtr<ID3D11Texture2D> stagingTexture;  // codex-line-comment: documents this line.
  hr = dxgiState_->device->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);  // codex-line-comment: documents this line.
  if (FAILED(hr)) {  // codex-line-comment: documents this line.
    return std::nullopt;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  dxgiState_->context->CopyResource(stagingTexture.Get(), acquiredTexture.Get());  // codex-line-comment: documents this line.

  D3D11_MAPPED_SUBRESOURCE mapped{};  // codex-line-comment: documents this line.
  hr = dxgiState_->context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped);  // codex-line-comment: documents this line.
  if (FAILED(hr)) {  // codex-line-comment: documents this line.
    return std::nullopt;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  std::vector<Color> pixels;  // codex-line-comment: documents this line.
  pixels.reserve(static_cast<std::size_t>(desc.Width * desc.Height));  // codex-line-comment: documents this line.

  const auto* rows = static_cast<const std::uint8_t*>(mapped.pData);  // codex-line-comment: documents this line.
  for (UINT y = 0; y < desc.Height; ++y) {  // codex-line-comment: documents this line.
    const auto* row = rows + static_cast<std::size_t>(y) * mapped.RowPitch;  // codex-line-comment: documents this line.
    for (UINT x = 0; x < desc.Width; ++x) {  // codex-line-comment: documents this line.
      const auto* bgra = row + static_cast<std::size_t>(x) * 4;  // codex-line-comment: documents this line.
      pixels.push_back(Color{bgra[2], bgra[1], bgra[0], bgra[3]});  // codex-line-comment: documents this line.
    }  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  dxgiState_->context->Unmap(stagingTexture.Get(), 0);  // codex-line-comment: documents this line.
  return Frame(static_cast<int>(desc.Width), static_cast<int>(desc.Height), std::move(pixels),  // codex-line-comment: documents this line.
               timestamp);  // codex-line-comment: documents this line.
#endif  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

std::optional<Frame> DesktopFrameCapture::captureFrameWithGdi(TimePoint timestamp) const {  // codex-line-comment: documents this line.
#ifndef _WIN32  // codex-line-comment: documents this line.
  (void)timestamp;  // codex-line-comment: documents this line.
  return std::nullopt;  // codex-line-comment: documents this line.
#else  // codex-line-comment: documents this line.
  const int width = display_.bounds.width;  // codex-line-comment: documents this line.
  const int height = display_.bounds.height;  // codex-line-comment: documents this line.
  if (width <= 0 || height <= 0) {  // codex-line-comment: documents this line.
    return std::nullopt;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  HDC screenDc = GetDC(nullptr);  // codex-line-comment: documents this line.
  HDC memoryDc = CreateCompatibleDC(screenDc);  // codex-line-comment: documents this line.
  HBITMAP bitmap = CreateCompatibleBitmap(screenDc, width, height);  // codex-line-comment: documents this line.
  HGDIOBJ oldBitmap = SelectObject(memoryDc, bitmap);  // codex-line-comment: documents this line.

  const BOOL copied = BitBlt(memoryDc, 0, 0, width, height, screenDc, display_.bounds.x,  // codex-line-comment: documents this line.
                            display_.bounds.y, SRCCOPY);  // codex-line-comment: documents this line.

  BITMAPINFO bitmapInfo{};  // codex-line-comment: documents this line.
  bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);  // codex-line-comment: documents this line.
  bitmapInfo.bmiHeader.biWidth = width;  // codex-line-comment: documents this line.
  bitmapInfo.bmiHeader.biHeight = -height;  // codex-line-comment: documents this line.
  bitmapInfo.bmiHeader.biPlanes = 1;  // codex-line-comment: documents this line.
  bitmapInfo.bmiHeader.biBitCount = 32;  // codex-line-comment: documents this line.
  bitmapInfo.bmiHeader.biCompression = BI_RGB;  // codex-line-comment: documents this line.

  std::vector<std::uint8_t> raw(static_cast<std::size_t>(width * height * 4));  // codex-line-comment: documents this line.
  const int readRows = copied ? GetDIBits(memoryDc, bitmap, 0, static_cast<UINT>(height), raw.data(),  // codex-line-comment: documents this line.
                                         &bitmapInfo, DIB_RGB_COLORS)  // codex-line-comment: documents this line.
                              : 0;  // codex-line-comment: documents this line.

  SelectObject(memoryDc, oldBitmap);  // codex-line-comment: documents this line.
  DeleteObject(bitmap);  // codex-line-comment: documents this line.
  DeleteDC(memoryDc);  // codex-line-comment: documents this line.
  ReleaseDC(nullptr, screenDc);  // codex-line-comment: documents this line.

  if (readRows == 0) {  // codex-line-comment: documents this line.
    return std::nullopt;  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  std::vector<Color> pixels;  // codex-line-comment: documents this line.
  pixels.reserve(static_cast<std::size_t>(width * height));  // codex-line-comment: documents this line.
  for (std::size_t i = 0; i < raw.size(); i += 4) {  // codex-line-comment: documents this line.
    pixels.push_back(Color{raw[i + 2], raw[i + 1], raw[i], raw[i + 3]});  // codex-line-comment: documents this line.
  }  // codex-line-comment: documents this line.

  return Frame(width, height, std::move(pixels), timestamp);  // codex-line-comment: documents this line.
#endif  // codex-line-comment: documents this line.
}  // codex-line-comment: documents this line.

}  // namespace fh6  // codex-line-comment: documents this line.
