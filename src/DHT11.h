#ifndef DHT11_H
#define DHT11_H

class DHT11
{
private:
  const int pin;

  uint8_t humidityReading;
  uint8_t temperatureReading;

  unsigned long lastUpdateMilliseconds;

public:
  enum class UpdateResult
  {
    OK,
    FailedToStartReading,
    FailedToReadByte,
    NoEndSignal,
    ChecksumError
  };

  DHT11(int pin);
  DHT11(const DHT11&) = delete;
  DHT11& operator=(const DHT11&) = delete;

  bool AbleToTakeReading() const;
  UpdateResult UpdateReadings();

  uint8_t Humidity() const { return humidityReading; }
  uint8_t Temperature() const { return temperatureReading; }
};

#endif // #ifndef DHT11_H
