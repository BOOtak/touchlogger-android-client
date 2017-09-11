//
// Created by k.leyfer on 11.09.2017.
//

#include "RawPointerAxes.h"

RawPointerAxes::RawPointerAxes()
{
  clear();
}

void RawPointerAxes::clear()
{
  x.clear();
  y.clear();
  pressure.clear();
  touchMajor.clear();
  touchMinor.clear();
  toolMajor.clear();
  toolMinor.clear();
  orientation.clear();
  distance.clear();
  tiltX.clear();
  tiltY.clear();
  trackingId.clear();
  slot.clear();
}
