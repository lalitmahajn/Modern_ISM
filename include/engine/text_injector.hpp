#pragma once
#include "engine/differential_injector.hpp"
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace minglish {

class ITextInjector {
public:
    virtual ~ITextInjector() = default;
    virtual bool Inject(const InjectionDiffU16& diff) = 0;
};

// Builds a contiguous Win32 INPUT array for atomic SendInput batching
std::vector<INPUT> BuildSendInputEvents(const InjectionDiffU16& diff);

class Win32SendInputInjector : public ITextInjector {
public:
    Win32SendInputInjector() = default;
    bool Inject(const InjectionDiffU16& diff) override;
};

class ClipboardFallbackInjector : public ITextInjector {
public:
    ClipboardFallbackInjector() = default;
    bool Inject(const InjectionDiffU16& diff) override;
};

} // namespace minglish
