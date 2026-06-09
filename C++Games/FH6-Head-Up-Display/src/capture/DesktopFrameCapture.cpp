#include "capture/DesktopFrameCapture.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <limits>
#include <vector>

#ifdef _WIN32
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <windows.h>
#endif

namespace fh6 {

#ifdef _WIN32
namespace {

using Microsoft::WRL::ComPtr;

struct DesktopRect {
  int left{0};
  int top{0};
  int right{0};
  int bottom{0};
};

int overlapArea(const Rect& a, const DesktopRect& b) {
  const int left = std::max(a.x, b.left);
  const int top = std::max(a.y, b.top);
  const int right = std::min(a.x + a.width, b.right);
  const int bottom = std::min(a.y + a.height, b.bottom);
  if (right <= left || bottom <= top) {
    return 0;
  }
  return (right - left) * (bottom - top);
}

}  // namespace

struct DesktopFrameCapture::DxgiCaptureState {
  ComPtr<ID3D11Device> device;
  ComPtr<ID3D11DeviceContext> context;
  ComPtr<IDXGIOutputDuplication> duplication;
  DXGI_OUTPUT_DESC outputDesc{};
  int width{0};
  int height{0};
};
#else
struct DesktopFrameCapture::DxgiCaptureState {};
#endif

DesktopFrameCapture::DesktopFrameCapture(int captureRateLimitFps)
    : captureRateLimitFps_(captureRateLimitFps > 0 ? captureRateLimitFps : 30) {}

DesktopFrameCapture::~DesktopFrameCapture() {
  stop();
}

bool DesktopFrameCapture::start(const DisplayInfo& display) {
  display_ = display;
  available_ = !display.bounds.empty();
  dxgiState_.reset();

#ifdef _WIN32
  (void)initializeDxgiCapture();
#endif

  return available_;
}

std::optional<Frame> DesktopFrameCapture::captureFrame() {
  if (!available_) {
    return std::nullopt;
  }

  const auto now = Clock::now();
  const auto minInterval = std::chrono::milliseconds(1000 / captureRateLimitFps_);
  if (lastCapture_ != TimePoint{} && now - lastCapture_ < minInterval) {
    return std::nullopt;
  }
  lastCapture_ = now;

  if (auto frame = captureFrameWithDxgi(now)) {
    return frame;
  }

  return captureFrameWithGdi(now);
}

void DesktopFrameCapture::stop() {
  dxgiState_.reset();
  available_ = false;
}

bool DesktopFrameCapture::isAvailable() const {
  return available_;
}

void DesktopFrameCapture::handleDisplayChanged(const DisplayInfo& display) {
  display_ = display;
  available_ = !display.bounds.empty();
  dxgiState_.reset();
#ifdef _WIN32
  (void)initializeDxgiCapture();
#endif
}

void DesktopFrameCapture::setCaptureRateLimit(int captureRateLimitFps) {
  if (captureRateLimitFps > 0) {
    captureRateLimitFps_ = captureRateLimitFps;
  }
}

bool DesktopFrameCapture::initializeDxgiCapture() {
#ifndef _WIN32
  return false;
#else
  if (display_.bounds.empty()) {
    return false;
  }

  auto state = std::make_unique<DxgiCaptureState>();

  const D3D_FEATURE_LEVEL featureLevels[] = {
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0,
  };
  D3D_FEATURE_LEVEL selectedFeatureLevel{};
  UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

  HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels,
                                 static_cast<UINT>(sizeof(featureLevels) / sizeof(featureLevels[0])),
                                 D3D11_SDK_VERSION,
                                 &state->device, &selectedFeatureLevel, &state->context);
  if (FAILED(hr)) {
    return false;
  }

  ComPtr<IDXGIDevice> dxgiDevice;
  hr = state->device.As(&dxgiDevice);
  if (FAILED(hr)) {
    return false;
  }

  ComPtr<IDXGIAdapter> adapter;
  hr = dxgiDevice->GetAdapter(&adapter);
  if (FAILED(hr)) {
    return false;
  }

  ComPtr<IDXGIOutput> bestOutput;
  DXGI_OUTPUT_DESC bestDesc{};
  int bestOverlap = -1;

  for (UINT index = 0;; ++index) {
    ComPtr<IDXGIOutput> output;
    if (adapter->EnumOutputs(index, &output) == DXGI_ERROR_NOT_FOUND) {
      break;
    }

    DXGI_OUTPUT_DESC desc{};
    if (FAILED(output->GetDesc(&desc))) {
      continue;
    }

    const DesktopRect outputRect{
        desc.DesktopCoordinates.left,
        desc.DesktopCoordinates.top,
        desc.DesktopCoordinates.right,
        desc.DesktopCoordinates.bottom,
    };
    const int area = overlapArea(display_.bounds, outputRect);
    if (area > bestOverlap) {
      bestOverlap = area;
      bestOutput = output;
      bestDesc = desc;
    }
  }

  if (!bestOutput || bestOverlap <= 0) {
    return false;
  }

  ComPtr<IDXGIOutput1> output1;
  hr = bestOutput.As(&output1);
  if (FAILED(hr)) {
    return false;
  }

  hr = output1->DuplicateOutput(state->device.Get(), &state->duplication);
  if (FAILED(hr)) {
    return false;
  }

  state->outputDesc = bestDesc;
  state->width = bestDesc.DesktopCoordinates.right - bestDesc.DesktopCoordinates.left;
  state->height = bestDesc.DesktopCoordinates.bottom - bestDesc.DesktopCoordinates.top;
  dxgiState_ = std::move(state);
  return true;
#endif
}

std::optional<Frame> DesktopFrameCapture::captureFrameWithDxgi(TimePoint timestamp) {
#ifndef _WIN32
  (void)timestamp;
  return std::nullopt;
#else
  if (!dxgiState_ || !dxgiState_->duplication) {
    return std::nullopt;
  }

  DXGI_OUTDUPL_FRAME_INFO frameInfo{};
  ComPtr<IDXGIResource> desktopResource;
  HRESULT hr = dxgiState_->duplication->AcquireNextFrame(0, &frameInfo, &desktopResource);
  if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
    return std::nullopt;
  }

  if (hr == DXGI_ERROR_ACCESS_LOST) {
    dxgiState_.reset();
    (void)initializeDxgiCapture();
    return std::nullopt;
  }

  if (FAILED(hr)) {
    return std::nullopt;
  }

  struct FrameRelease {
    IDXGIOutputDuplication* duplication{nullptr};
    ~FrameRelease() {
      if (duplication != nullptr) {
        duplication->ReleaseFrame();
      }
    }
  } release{dxgiState_->duplication.Get()};

  ComPtr<ID3D11Texture2D> acquiredTexture;
  hr = desktopResource.As(&acquiredTexture);
  if (FAILED(hr)) {
    return std::nullopt;
  }

  D3D11_TEXTURE2D_DESC desc{};
  acquiredTexture->GetDesc(&desc);

  D3D11_TEXTURE2D_DESC stagingDesc = desc;
  stagingDesc.BindFlags = 0;
  stagingDesc.MiscFlags = 0;
  stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  stagingDesc.Usage = D3D11_USAGE_STAGING;

  ComPtr<ID3D11Texture2D> stagingTexture;
  hr = dxgiState_->device->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);
  if (FAILED(hr)) {
    return std::nullopt;
  }

  dxgiState_->context->CopyResource(stagingTexture.Get(), acquiredTexture.Get());

  D3D11_MAPPED_SUBRESOURCE mapped{};
  hr = dxgiState_->context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped);
  if (FAILED(hr)) {
    return std::nullopt;
  }

  std::vector<Color> pixels;
  pixels.reserve(static_cast<std::size_t>(desc.Width * desc.Height));

  const auto* rows = static_cast<const std::uint8_t*>(mapped.pData);
  for (UINT y = 0; y < desc.Height; ++y) {
    const auto* row = rows + static_cast<std::size_t>(y) * mapped.RowPitch;
    for (UINT x = 0; x < desc.Width; ++x) {
      const auto* bgra = row + static_cast<std::size_t>(x) * 4;
      pixels.push_back(Color{bgra[2], bgra[1], bgra[0], bgra[3]});
    }
  }

  dxgiState_->context->Unmap(stagingTexture.Get(), 0);
  return Frame(static_cast<int>(desc.Width), static_cast<int>(desc.Height), std::move(pixels),
               timestamp);
#endif
}

std::optional<Frame> DesktopFrameCapture::captureFrameWithGdi(TimePoint timestamp) const {
#ifndef _WIN32
  (void)timestamp;
  return std::nullopt;
#else
  const int width = display_.bounds.width;
  const int height = display_.bounds.height;
  if (width <= 0 || height <= 0) {
    return std::nullopt;
  }

  HDC screenDc = GetDC(nullptr);
  HDC memoryDc = CreateCompatibleDC(screenDc);
  HBITMAP bitmap = CreateCompatibleBitmap(screenDc, width, height);
  HGDIOBJ oldBitmap = SelectObject(memoryDc, bitmap);

  const BOOL copied = BitBlt(memoryDc, 0, 0, width, height, screenDc, display_.bounds.x,
                            display_.bounds.y, SRCCOPY);

  BITMAPINFO bitmapInfo{};
  bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bitmapInfo.bmiHeader.biWidth = width;
  bitmapInfo.bmiHeader.biHeight = -height;
  bitmapInfo.bmiHeader.biPlanes = 1;
  bitmapInfo.bmiHeader.biBitCount = 32;
  bitmapInfo.bmiHeader.biCompression = BI_RGB;

  std::vector<std::uint8_t> raw(static_cast<std::size_t>(width * height * 4));
  const int readRows = copied ? GetDIBits(memoryDc, bitmap, 0, static_cast<UINT>(height), raw.data(),
                                         &bitmapInfo, DIB_RGB_COLORS)
                              : 0;

  SelectObject(memoryDc, oldBitmap);
  DeleteObject(bitmap);
  DeleteDC(memoryDc);
  ReleaseDC(nullptr, screenDc);

  if (readRows == 0) {
    return std::nullopt;
  }

  std::vector<Color> pixels;
  pixels.reserve(static_cast<std::size_t>(width * height));
  for (std::size_t i = 0; i < raw.size(); i += 4) {
    pixels.push_back(Color{raw[i + 2], raw[i + 1], raw[i], raw[i + 3]});
  }

  return Frame(width, height, std::move(pixels), timestamp);
#endif
}

}  // namespace fh6
