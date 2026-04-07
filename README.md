# kitchen_gas_sensor

This is a kitchen gas sensor using ESPHome. It detects gas leaks using an MQ-6 sensor and triggers an active buzzer alarm.

---

## 🔧 Hardware
- ESP32-C3
- MQ-6 Gas Sensor (analog output)
- Active buzzer
- 1x NeoPixel (status LED)
- Push button (reset)

---

## 🚀 Setup & Build

### ✅ Run
```bash
python -m esphome -s device_id 0 run gas_sensor.yaml
```



### ✅ Repair (Windows)
```bash
Remove-Item -Recurse -Force $env:USERPROFILE\.platformio -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force .\.esphome -ErrorAction SilentlyContinue
```





