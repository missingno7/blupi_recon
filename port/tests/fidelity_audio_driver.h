// Test-only access to the production SDL callback; no audio device is opened.
#pragma once
#include <vector>
namespace audio_test { std::vector<float> render(int frames); }
