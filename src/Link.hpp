#include "ZZC.hpp"
#include "../ZZC/src/widgets.hpp"

// The following Rack macros are in conflict with macros from ASIO
#undef defer
#undef debug
#undef info

#include "ableton/Link.hpp"
#include "HostTimeFilter.hpp"

struct Link : Module {
  enum ParamIds {
    NUM_PARAMS
  };
  enum InputIds {
    NUM_INPUTS
  };
  enum OutputIds {
    BEAT_PHASE_OUTPUT,
    BAR_PHASE_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    READY_LED,
    NUM_LIGHTS
  };

  ableton::Link *link;
  ableton::link::HostTimeFilter<ableton::link::platform::Clock> hostTimeFilter;
  double timeAcc = 0.0;
  int warmUp = 16384 + 10;
  dsp::ClockDivider bufsizeDivider;
  std::vector<std::chrono::microseconds> timeStamps;
  double blockTime = 0;
  double sampleTime = 0;
  bool synced = false;

  std::chrono::microseconds timeBase = std::chrono::microseconds(0);

  Link();
  ~Link();
  void process(const ProcessArgs &args) override;
};

struct LinkWidget : ModuleWidget {
  LinkWidget(Link *module);
};
