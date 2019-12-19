#include "Link.hpp"

Link::Link() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  this->link = new ableton::Link(120.0);
  this->link->enable(true);
  this->bufsizeDivider.setDivision(256);
  double sampleRate = 44100.0;
  double bufSize = 256.0;
  this->sampleTime = 1.0 / sampleRate;
  this->blockTime = this->sampleTime * bufSize;
  this->timeStamps.reserve(2000);
}

Link::~Link() {
  if (this->link) {
    this->link->enable(false);
    delete this->link;
  }
}

void Link::process(const ProcessArgs &args) {
  if (this->link) {
    // std::chrono::microseconds now = this->link->clock().micros();

    if (this->warmUp > 0) {
      // We wait some samples before starting to do any estimations
      // just to hope to appear somewhere near a start of a block
      this->warmUp--;
      return;
    }

    if (!this->synced && this->bufsizeDivider.process()) {
      lights[READY_LED].setBrightness(0.f);
      std::chrono::microseconds now = this->link->clock().micros();
      this->timeStamps.push_back(now);
      if (this->timeStamps.size() == this->timeStamps.capacity()) {
        // Alternative to linear regression used in ableton::link::HostTimeFilter
        // We doing it only once to avoid jitter
        std::size_t stampsSize = this->timeStamps.size();
        unsigned long long microsTotal = 0;
        for (size_t i = 0; i < stampsSize; i++) {
          size_t distanceFromNow = stampsSize - i - 1;
          std::chrono::microseconds extrapolatedNow = this->timeStamps.at(i) + std::chrono::microseconds(
            std::llround(this->blockTime * distanceFromNow * 1000000)
          );
          microsTotal += extrapolatedNow.count();
        }
        long long nowAverage = microsTotal / stampsSize;
        this->timeBase = std::chrono::microseconds(nowAverage);

        this->timeAcc = 0.0;
        this->synced = true;
        lights[READY_LED].setBrightness(1.f);
      }
    }

    this->timeAcc += this->sampleTime;

    ableton::Link::SessionState sessionState = this->link->captureAudioSessionState();
    std::chrono::microseconds extrapolatedTime = this->timeBase + std::chrono::microseconds(std::llround(this->timeAcc * 1000000));
    double barPhase = sessionState.phaseAtTime(extrapolatedTime, 4);
    outputs[BEAT_PHASE_OUTPUT].setVoltage(eucMod(barPhase, 1.f) * 10.f);
    outputs[BAR_PHASE_OUTPUT].setVoltage(barPhase * 2.5f);
  }
}

LinkWidget::LinkWidget(Link *module) {
  setModule(module);
  setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Link.svg")));

  addChild(createLight<SmallLight<ZZC_YellowLight>>(Vec(34.2f, 258.0f), module, Link::READY_LED));

  addOutput(createOutput<ZZC_PJ_Port>(Vec(8, 320), module, Link::BEAT_PHASE_OUTPUT));
  addOutput(createOutput<ZZC_PJ_Port>(Vec(42.5, 320), module, Link::BAR_PHASE_OUTPUT));

  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model *modelLink = createModel<Link, LinkWidget>("Link");
