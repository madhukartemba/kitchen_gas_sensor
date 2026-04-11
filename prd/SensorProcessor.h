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
      if (state != COOLDOWN) {
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

  void update(double reading, unsigned long timestamp) {
    if (state == WARMUP) {
      if (timestamp > warmupTime) {
        setState(NORMAL, timestamp);
      }
    } else {
      // Do the normal processing everytime
      readings.push_back(reading);
      if (readings.size() > numberOfReadings) {
        readings.erase(readings.begin());
      }

      double readingDiff = reading - readings.front();

      if (readingDiff > relativeThreshold) {
        setState(ALARM, timestamp);
      } else if (reading > absoluteThreshold) {
        setState(ALARM, timestamp);
      } else {
        setState(NORMAL, timestamp);
      }
    }
  }

  SensorState getState() { return state; }

  void resetAlarm(unsigned long timestamp) { setState(COOLDOWN, timestamp); }
};

// Creates a single instance globally accessible from anywhere in ESPHome
inline SensorProcessor *get_processor() {
  static SensorProcessor *instance =
      new SensorProcessor(60000, 30000, 10, 1.5, 0.3);
  return instance;
}