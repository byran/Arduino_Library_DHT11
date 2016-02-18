#include "Arduino.h"
#include "DHT11.h"

namespace
{
  const unsigned long TIME_BETWEEN_UPDATES = 200;

  void RequestSample(int pin)
  {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    delay(18);
    pinMode(pin, INPUT);
  }

  struct PinWaitResult
  {
    bool valid;
    unsigned long micros;
  };

  struct PinWaitResult WaitForPin(int pin, int state)
  {
    unsigned long start = micros();
    PinWaitResult result;

    result.valid = false;

    do
    {
      result.micros = micros();
      if((result.micros - start) > 100)
      {
        return result;
      }
    } while(digitalRead(pin) != state);

    result.valid = true;

    return result;
  }
} // namespace

DHT11::DHT11(int pin)
  : pin{pin}
  , humidityReading{0}
  , temperatureReading{0}
  , lastUpdateMilliseconds{0}
{
}

bool DHT11::AbleToTakeReading() const
{
  return (millis() - lastUpdateMilliseconds) > TIME_BETWEEN_UPDATES;
}

auto DHT11::UpdateReadings() -> UpdateResult
{
  while(!AbleToTakeReading()) { }

  RequestSample(pin);

  if(!WaitForPin(pin, LOW).valid)
  {
    return UpdateResult::FailedToStartReading;
  }
  if(!WaitForPin(pin, HIGH).valid)
  {
    return UpdateResult::FailedToStartReading;
  }
  if(!WaitForPin(pin, LOW).valid)
  {
    return UpdateResult::FailedToStartReading;
  }

  uint8_t data[5];
  for(int byteIndex = 0; byteIndex < 5; ++byteIndex)
  {
    data[byteIndex] = 0;
    for(int bitIndex = 0; bitIndex < 8; ++bitIndex)
    {
      auto highStartTime = WaitForPin(pin, HIGH);
      if(!highStartTime.valid)
      {
        return UpdateResult::FailedToReadByte;
      }

      auto highEndTime = WaitForPin(pin, LOW);
      if(!highEndTime.valid)
      {
        return UpdateResult::FailedToReadByte;
      }

      if((highEndTime.micros - highStartTime.micros) > 40)
      {
        data[byteIndex] |= (1 << (8 - bitIndex));
      }
    }
  }

  if(!WaitForPin(pin, HIGH).valid)
  {
    return UpdateResult::NoEndSignal;
  }

  if(data[0] + data[1] + data[2] + data[3] != data[4])
  {
    return UpdateResult::ChecksumError;
  }

  humidityReading = data[0];
  temperatureReading = data[2];

  lastUpdateMilliseconds = millis();

  return UpdateResult::OK;
}
