#pragma once
#include <string>
#include <vector>

enum SensorState {
  WARMUP,
  NORMAL,
  ALARM,
  COOLDOWN,
  ERROR,
};

inline const char *sensorStateStr(SensorState s) {
  switch (s) {
  case WARMUP:
    return "warmup";
  case NORMAL:
    return "normal";
  case ALARM:
    return "alarm";
  case COOLDOWN:
    return "cooldown";
  case ERROR:
    return "error";
  default:
    return "?";
  }
}

class SensorProcessor {
private:
  double readingSum;
  std::vector<double> readings;
  int numberOfReadings;
  unsigned long warmupTime;
  unsigned long cooldownTime;
  SensorState state;

  // Thresholds
  double absoluteThreshold;
  double relativeThreshold;

  // Last reading minus oldest sample in window (0 during warmup)
  double relativeReading{0};

  // Timers
  unsigned long cooldownTimer;

  void setState(SensorState newState, unsigned long timestamp) {
    switch (newState) {
    case WARMUP:
      state = newState;
      break;
    case NORMAL:
      if (state != ALARM &&
          (state != COOLDOWN || timestamp - cooldownTimer > cooldownTime)) {
        state = newState;
      }
      break;
    case ALARM:
      if (state != COOLDOWN) {
        state = newState;
      } else if (timestamp - cooldownTimer > cooldownTime) {
        state = newState;
      }
      break;
    case COOLDOWN:
      if (state == ALARM) {
        state = newState;
        cooldownTimer = timestamp;
      }
      break;
    case ERROR:
      state = newState;
      break;
    };
  }

public:
  SensorProcessor(unsigned long warmupTime, unsigned long cooldownTime,
                  int numberOfReadings, double absoluteThreshold,
                  double relativeThreshold)
      : readingSum(0), numberOfReadings(numberOfReadings),
        warmupTime(warmupTime), cooldownTime(cooldownTime), state(WARMUP),
        absoluteThreshold(absoluteThreshold),
        relativeThreshold(relativeThreshold), cooldownTimer(0) {}

  void setAbsoluteThreshold(double value) { absoluteThreshold = value; }

  void setRelativeThreshold(double value) { relativeThreshold = value; }

  int getNumberOfReadings() const { return numberOfReadings; }

  void setNumberOfReadings(int value) {
    if (value < 1) {
      value = 1;
    }
    numberOfReadings = value;
    while (readings.size() > static_cast<size_t>(numberOfReadings)) {
      readings.erase(readings.begin());
    }
  }

  // Milliseconds from boot until warmup ends (compared to millis() in update()).
  unsigned long getWarmupTimeMs() const { return warmupTime; }

  void setWarmupTimeMs(unsigned long ms) { warmupTime = ms; }

  unsigned long getCooldownTimeMs() const { return cooldownTime; }

  void setCooldownTimeMs(unsigned long ms) { cooldownTime = ms; }

  void update(double reading, unsigned long timestamp) {
    if (state == WARMUP) {
      relativeReading = 0;
      if (timestamp > warmupTime) {
        setState(NORMAL, timestamp);
      }
    } else {
      // Do the normal processing everytime
      readings.push_back(reading);
      if (readings.size() > numberOfReadings) {
        readings.erase(readings.begin());
      }

      relativeReading = reading - readings.front();

      if (relativeReading > relativeThreshold) {
        setState(ALARM, timestamp);
      } else if (reading > absoluteThreshold) {
        setState(ALARM, timestamp);
      } else {
        setState(NORMAL, timestamp);
      }
    }
  }

  SensorState getState() const { return state; }

  double getRelativeReading() const { return relativeReading; }

  void resetAlarm(unsigned long timestamp) { setState(COOLDOWN, timestamp); }
};

// Creates a single instance globally accessible from anywhere in ESPHome
inline SensorProcessor *get_processor() {
  static SensorProcessor *instance =
      new SensorProcessor(60000, 30000, 300, 1.5, 0.3);
  return instance;
}