//
// Created by kirill on 08.04.18.
//

#include <linux/time.h>
#include <time.h>
#include <cstdio>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <cerrno>
#include "Reanimator.h"
#include "../dirty/common/logging.h"

#define MS_IN_SEC   1000 * 1000L
#define MS_IN_NS    1000L

Reanimator::Reanimator(useconds_t maxHeartBeatIntervalMs, reanimate_callback_t reanimateFunc)
    : maxHeartBeatInterval(maxHeartBeatIntervalMs), reanimateCallback(reanimateFunc),
      checkInterval(maxHeartBeatIntervalMs / 2), lastHeartbeatTimeStamp(0), shouldStop(false),
      reanimatorThread(NULL)
{}

int Reanimator::onHeartBeat()
{
  lastHeartbeatTimeStamp = getTimeStampMs();
  return 0;
}

long Reanimator::getTimeStampMs()
{
  timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1)
  {
    LOGV("Unable to get current time: %s!", strerror(errno));
    return 0;
  }

  return ts.tv_sec * MS_IN_SEC + ts.tv_nsec / MS_IN_NS;
}

bool Reanimator::isTooLate(long stamp)
{
  return lastHeartbeatTimeStamp > 0 && stamp > lastHeartbeatTimeStamp + maxHeartBeatInterval * 2;
}

int Reanimator::start()
{
  __sync_val_compare_and_swap(&shouldStop, 1, 0);
  int createRetval;
  if ((createRetval = pthread_create(&reanimatorThread, NULL, &reanimatorLoop, this)) != 0)
  {
    LOGV("Unable to create thread: %s!", strerror(createRetval));
    return createRetval;
  }

  return 0;
}

int Reanimator::stop()
{
  __sync_val_compare_and_swap(&shouldStop, 0, 1);
  if (reanimatorThread)
  {
    void* threadRetval;
    int joinRetval;
    if ((joinRetval = pthread_join(reanimatorThread, &threadRetval)) != 0)
    {
      LOGV("Unable to join thread: %s", strerror(joinRetval));
      return joinRetval;
    }
  }

  return 0;
}

Reanimator::~Reanimator()
{
  this->stop();
}

void* Reanimator::reanimatorLoop(void* param)
{
  Reanimator* cls = reinterpret_cast<Reanimator*>(param);
  while (__sync_bool_compare_and_swap(&(cls->shouldStop), 0, 0))
  {
    usleep(cls->checkInterval);
    long currentTimeStamp = getTimeStampMs();
    if (cls->isTooLate(currentTimeStamp))
    {
      LOGV("There is no info from service for more than %li us, reanimating it!",
           currentTimeStamp - cls->lastHeartbeatTimeStamp);
      if (cls->reanimateCallback() == -1)
      {
        LOGV("Unable to reanimate service!");
      }
      else
      {
        LOGV("Service reanimated.");
      }
    }
  }

  return NULL;
}
