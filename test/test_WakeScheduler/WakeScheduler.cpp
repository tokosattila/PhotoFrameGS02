#include <unity.h>
#include <App/Global.h>
#include <App/WakeScheduler.h>

using namespace App;

void test_single_alarm_due() {
  SRTCDateTime now {0, 30, 7, 1, 15, 5, 2026};
  SAlarmSpec alarm;
  alarm.DateTime = now;
  alarm.Enabled = true;
  SWakeSchedule sched;
  sched.Alarms.push_back(alarm);
  sched.Enabled = true;
  auto status = WakeScheduler_::Instance().Compute(sched, now);
  TEST_ASSERT_TRUE(status.WakeDue);
}

void test_alarm_not_due() {
  SRTCDateTime now {0, 30, 7, 1, 15, 5, 2026};
  SAlarmSpec alarm;
  alarm.DateTime = now;
  alarm.Enabled = true;
  SWakeSchedule sched;
  sched.Alarms.push_back(alarm);
  sched.Enabled = true;
  SRTCDateTime later = now;
  later.Minute = 31;
  auto status = WakeScheduler_::Instance().Compute(sched, later);
  TEST_ASSERT_FALSE(status.WakeDue);
}

void test_alarm_disabled() {
  SRTCDateTime now {0, 30, 7, 1, 15, 5, 2026};
  SAlarmSpec alarm;
  alarm.DateTime = now;
  alarm.Enabled = false;
  SWakeSchedule sched;
  sched.Alarms.push_back(alarm);
  sched.Enabled = true;
  auto status = WakeScheduler_::Instance().Compute(sched, now);
  TEST_ASSERT_FALSE(status.WakeDue);
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_single_alarm_due);
  RUN_TEST(test_alarm_not_due);
  RUN_TEST(test_alarm_disabled);
  UNITY_END();
  return 0;
}
