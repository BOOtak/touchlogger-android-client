//
// Created by kirill on 08.04.18.
//

#ifndef TOUCHLOGGER_DIRTY_REANIMATOR_H
#define TOUCHLOGGER_DIRTY_REANIMATOR_H


#include <sys/stat.h>

typedef int (* reanimate_callback_t)();

class Reanimator
{
public:
  Reanimator(useconds_t maxHeartBeatInterval, reanimate_callback_t reanimateFunc);

  ~Reanimator();

  int onHeartBeat();

  int start();

  int stop();
private:
  const useconds_t maxHeartBeatInterval;

  const reanimate_callback_t reanimateCallback;

  bool shouldStop;

  pthread_t reanimatorThread;

  volatile long lastHeartbeatTimeStamp;

  const useconds_t checkInterval;

  /**
   * Get current monotonic clock timestamp in microseconds.
   * @return Current monotonic clock timestamp in microseconds.
   */
  static long getTimeStampUs();

  bool isTooLate(long stamp);

  static void* reanimatorLoop(void* param);
};


#endif //TOUCHLOGGER_DIRTY_REANIMATOR_H
