//
// Created by kirill on 08.04.18.
//

#ifndef TOUCHLOGGER_DIRTY_PAUSABLE_H
#define TOUCHLOGGER_DIRTY_PAUSABLE_H


class Pausable
{
public:
  virtual void pause()= 0;

  virtual void resume() = 0;
};


#endif //TOUCHLOGGER_DIRTY_PAUSABLE_H
