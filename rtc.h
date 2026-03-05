#include <RtcDS3231.h>
#include <Wire.h>
RtcDS3231<TwoWire> Rtc(Wire);

void set_rtc(int year, int month, int day, int hour, int minute, int second);

struct rtc_struct {
  int year_;
  int month_;
  int day_;
  int hour_;
  int minute_;
  int second_;  
} rtc_data;

bool initRTC() {
  Rtc.Begin();

  // Проверяем, работает ли RTC и валидно ли время
  if (!Rtc.GetIsRunning() || Rtc.GetDateTime().Year() < 2025) {
    // Устанавливаем время только если RTC не работает или время сброшено
    set_rtc(2025, 3, 20, 5, 51, 0); // Ваше начальное время
    Rtc.SetIsRunning(true); // Запускаем RTC, если он был остановлен
    Serial.println("RTC initialized with new time");
  } else {
    Serial.println("RTC already running, keeping current time");
  }

  return Rtc.GetIsRunning();
}

void set_rtc(int year, int month, int day, int hour, int minute, int second) {
  RtcDateTime dt(year, month, day, hour, minute, second);
  Rtc.SetDateTime(dt);
}

rtc_struct * get_rtc() {
  RtcDateTime dt = Rtc.GetDateTime();
  rtc_data.year_ = dt.Year();
  rtc_data.month_ = dt.Month();
  rtc_data.day_ = dt.Day();
  rtc_data.hour_ = dt.Hour();
  rtc_data.minute_ = dt.Minute();
  rtc_data.second_ = dt.Second();
  return &rtc_data;
}

void print_data(rtc_struct * data_) {
  Serial.print("Time: ");
  Serial.print(data_->year_);
  Serial.print(".");
  if (data_->month_ < 10) {
    Serial.print(0);
    Serial.print(data_->month_);
  } else {
    Serial.print(data_->month_);  
  }
  Serial.print(".");
  if (data_->day_ < 10) {
    Serial.print(0);
    Serial.print(data_->day_);
  } else {
    Serial.print(data_->day_);  
  }
  Serial.print("  ");
  if (data_->hour_ < 10) {
    Serial.print(0);
    Serial.print(data_->hour_);
  } else {
    Serial.print(data_->hour_);  
  }
  Serial.print(":");
  if (data_->minute_ < 10) {
    Serial.print(0);
    Serial.print(data_->minute_);
  } else {
    Serial.print(data_->minute_);  
  }
  Serial.print(":");
  if (data_->second_ < 10) {
    Serial.print(0);
    Serial.println(data_->second_);
  } else {
    Serial.println(data_->second_);  
  }
}
