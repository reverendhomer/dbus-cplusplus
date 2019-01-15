/*
 *
 *  D-Bus++ - C++ bindings for D-Bus
 *
 *  Copyright (C) 2005-2007  Paolo Durante <shackan@gmail.com>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <dbus-c++/eventloop.h>
#include <dbus-c++/debug.h>

#include <sys/poll.h>
#include <sys/time.h>

#include <dbus/dbus.h>

using namespace DBus;
using namespace std;

static double millis(timeval tv)
{
  return (tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0);
}

DefaultTimeout::DefaultTimeout(int interval, bool repeat, DefaultMainLoop *ed)
  : _enabled(true), _interval(interval), _repeat(repeat), _expiration(0), _data(0), _disp(ed)
{
  timeval now;
  gettimeofday(&now, NULL);

  _expiration = millis(now) + interval;

  std::lock_guard<std::mutex> lck(_disp->_mutex_t);
  _disp->_timeouts.push_back(this);
}

DefaultTimeout::~DefaultTimeout()
{
  std::lock_guard<std::mutex> lck(_disp->_mutex_t);
  _disp->_timeouts.remove(this);
}

DefaultWatch::DefaultWatch(int fd, int flags, DefaultMainLoop *ed)
  : _enabled(true), _fd(fd), _flags(flags), _state(0), _data(0), _disp(ed)
{
  std::lock_guard<std::recursive_mutex> lck(_disp->_mutex_w);
  _disp->_watches.push_back(this);
}

DefaultWatch::~DefaultWatch()
{
  std::lock_guard<std::recursive_mutex> lck(_disp->_mutex_w);
  _disp->_watches.remove(this);
}

DefaultMainLoop::DefaultMainLoop() :
  _mutex_w()
{
}

DefaultMainLoop::~DefaultMainLoop()
{
  _mutex_w.lock();

  DefaultWatches::iterator wi = _watches.begin();
  while (wi != _watches.end()) {
    DefaultWatches::iterator wmp = wi;
    ++wmp;
    _mutex_w.unlock();
    delete(*wi);
    _mutex_w.lock();
    wi = wmp;
  }
  _mutex_w.unlock();

  _mutex_t.lock();
  DefaultTimeouts::iterator ti = _timeouts.begin();
  while (ti != _timeouts.end())
  {
    DefaultTimeouts::iterator tmp = ti;
    ++tmp;
    _mutex_t.unlock();
    delete(*ti);
    _mutex_t.lock();
    ti = tmp;
  }
  _mutex_t.unlock();
}

void DefaultMainLoop::dispatch()
{
  _mutex_w.lock();
  auto nfd = _watches.size();

  if (_fdunlock)
    nfd += 2;

  pollfd fds[nfd];

  auto wi = _watches.begin();
  for (nfd = 0; wi != _watches.end(); ++wi) {
    if ((*wi)->enabled()) {
      fds[nfd].fd = (*wi)->descriptor();
      fds[nfd].events = (*wi)->flags();
      fds[nfd].revents = 0;

      ++nfd;
    }
  }

  if (_fdunlock) {
    fds[nfd].fd = _fdunlock[0];
    fds[nfd].events = POLLIN | POLLOUT | POLLPRI ;
    fds[nfd].revents = 0;

    nfd++;
    fds[nfd].fd = _fdunlock[1];
    fds[nfd].events = POLLIN | POLLOUT | POLLPRI ;
    fds[nfd].revents = 0;
  }
  _mutex_w.unlock();

  int wait_min = 10000;

  {
    std::lock_guard<std::mutex> lck(_mutex_t);
    for (const auto& timeout : _timeouts)
      if (timeout->enabled() && timeout->interval() < wait_min)
        wait_min = timeout->interval();
  }

  poll(fds, nfd, wait_min);

  timeval now;
  gettimeofday(&now, NULL);

  double now_millis = millis(now);

  {
    std::lock_guard<std::mutex> lck(_mutex_t);
    auto ti = _timeouts.begin();

    while (ti != _timeouts.end()) {
      auto tmp = ti;
      ++tmp;

      if ((*ti)->enabled() && now_millis >= (*ti)->_expiration) {
        (*ti)->expired(*(*ti));

        if ((*ti)->_repeat)
          (*ti)->_expiration = now_millis + (*ti)->_interval;
      }

      ti = tmp;
    }
  }

  {
    std::lock_guard<std::recursive_mutex> lck(_mutex_w);
    for (size_t j = 0; j < nfd; ++j) {
      for (auto wi = _watches.begin(); wi != _watches.end();) {
        auto tmp = wi;
        ++tmp;

        if ((*wi)->enabled() && (*wi)->_fd == fds[j].fd) {
          if (fds[j].revents) {
            (*wi)->_state = fds[j].revents;
            (*wi)->ready(*(*wi));
            fds[j].revents = 0;
          }
        }
        wi = tmp;
      }
    }
  }
}

