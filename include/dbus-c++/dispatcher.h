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


#ifndef __DBUSXX_DISPATCHER_H
#define __DBUSXX_DISPATCHER_H

#include "api.h"
#include "connection.h"
#include "eventloop.h"

#include <mutex>

namespace DBus
{

class DXXAPI Timeout
{
public:

  class Internal;

  Timeout(Internal *i);

  virtual ~Timeout() {}

  /*!
   * \brief Gets the timeout interval.
   *
   * The handle() should be called each time this interval elapses,
   * starting after it elapses once.
   *
   * The interval may change during the life of the timeout; if so, the timeout
   * will be disabled and re-enabled (calling the "timeout toggled function") to
   * notify you of the change.
   *
   * return The interval in miliseconds.
   */
  int interval() const;

  bool enabled() const;

  /*!
   * \brief Calls the timeout handler for this timeout.
   *
   * This function should be called when the timeout occurs.
   *
   * If this function returns FALSE, then there wasn't enough memory to handle
   * the timeout. Typically just letting the timeout fire again next time it
   * naturally times out is an adequate response to that problem, but you could
   * try to do more if you wanted.
   *
   * return false If there wasn't enough memory.
   */
  bool handle();

  virtual void toggle() = 0;

private:

  DXXAPILOCAL Timeout(const Timeout &);

private:

  Internal *_int;
};

class DXXAPI Watch
{
public:

  class Internal;

  Watch(Internal *i);

  virtual ~Watch() {}

  /*!
   * \brief A main loop could poll this descriptor to integrate dbus-c++.
   *
   * This function calls dbus_watch_get_socket() on win32 and
   * dbus_watch_get_unix_fd() on all other systems. (see dbus documentation)
   *
   * @return The file descriptor.
   */
  int descriptor() const noexcept;

  /*!
   * \brief Gets flags from DBusWatchFlags indicating what conditions should be
   *        monitored on the file descriptor.
   *
   * The flags returned will only contain DBUS_WATCH_READABLE and DBUS_WATCH_WRITABLE,
   * never DBUS_WATCH_HANGUP or DBUS_WATCH_ERROR; all watches implicitly include
   * a watch for hangups, errors, and other exceptional conditions.
   *
   * @return The conditions to watch.
   */
  int flags() const noexcept;

  bool enabled() const noexcept;

  /*!
   * \brief Called to notify the D-Bus library when a previously-added watch
   *        is ready for reading or writing, or has an exception such as a hangup.
   *
   * If this function returns FALSE, then the file descriptor may still be
   * ready for reading or writing, but more memory is needed in order to do the
   * reading or writing. If you ignore the FALSE return, your application may
   * spin in a busy loop on the file descriptor until memory becomes available,
   * but nothing more catastrophic should happen.
   *
   * dbus_watch_handle() cannot be called during the DBusAddWatchFunction, as the
   * connection will not be ready to handle that watch yet.
   *
   * It is not allowed to reference a DBusWatch after it has been passed to remove_function.
   *
   * @param flags The poll condition using DBusWatchFlags values.
   * @return false If there wasn't enough memory.
   */
  bool handle(int flags) noexcept;

  virtual void toggle() = 0;

private:

  DXXAPILOCAL Watch(const Watch &);

private:

  Internal *_int;
};

class DXXAPI Dispatcher
{
public:

  virtual ~Dispatcher()
  {}

  void queue_connection(Connection::Private *);

  void dispatch_pending();
  bool has_something_to_dispatch();

  virtual void enter() = 0;

  virtual void leave() = 0;

  virtual Timeout *add_timeout(Timeout::Internal *) = 0;

  virtual void rem_timeout(Timeout *) = 0;

  virtual Watch *add_watch(Watch::Internal *) = 0;

  virtual void rem_watch(Watch *) = 0;

  struct Private;

private:
  void dispatch_pending(Connection::PrivatePList &pending_queue);

  std::mutex _mutex_p;
  std::mutex _mutex_p_copy;

  Connection::PrivatePList _pending_queue;
};

extern DXXAPI Dispatcher *default_dispatcher;

void DXXAPI _init_threading();

} /* namespace DBus */

#endif//__DBUSXX_DISPATCHER_H
