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

#include <dbus-c++/dispatcher.h>

#include <dbus/dbus.h>

#include "dispatcher_p.h"
#include "server_p.h"
#include "connection_p.h"

DBus::Dispatcher *DBus::default_dispatcher = NULL;

using namespace DBus;

Timeout::Timeout(Timeout::Internal *i)
  : _int(i)
{
  dbus_timeout_set_data((DBusTimeout *)i, this, NULL);
}

int Timeout::interval() const
{
  return dbus_timeout_get_interval((DBusTimeout *)_int);
}

bool Timeout::enabled() const
{
  return dbus_timeout_get_enabled((DBusTimeout *)_int);
}

bool Timeout::handle()
{
  return dbus_timeout_handle((DBusTimeout *)_int);
}

/*
*/

Watch::Watch(Watch::Internal *i)
  : _int(i)
{
  dbus_watch_set_data((DBusWatch *)i, this, NULL);
}

int Watch::descriptor() const noexcept
{
#if HAVE_WIN32
  return dbus_watch_get_socket((DBusWatch *)_int);
#else
  return dbus_watch_get_unix_fd((DBusWatch *)_int);
#endif
}

int Watch::flags() const noexcept
{
  return dbus_watch_get_flags((DBusWatch *)_int);
}

bool Watch::enabled() const noexcept
{
  return dbus_watch_get_enabled((DBusWatch *)_int);
}

bool Watch::handle(int flags) noexcept
{
  return dbus_watch_handle((DBusWatch *)_int, flags);
}

/*
*/

dbus_bool_t Dispatcher::Private::on_add_watch(DBusWatch *watch, void *data)
{
  auto d = static_cast<Dispatcher *>(data);
  d->add_watch(reinterpret_cast<Watch::Internal *>(watch));
  return true;
}

void Dispatcher::Private::on_rem_watch(DBusWatch *watch, void *data)
{
  auto d = static_cast<Dispatcher *>(data);
  d->rem_watch(static_cast<Watch *>(dbus_watch_get_data(watch)));
}

void Dispatcher::Private::on_toggle_watch(DBusWatch *watch, void *data)
{
  static_cast<Watch *>(dbus_watch_get_data(watch))->toggle();
}

dbus_bool_t Dispatcher::Private::on_add_timeout(DBusTimeout *timeout, void *data)
{
  auto d = static_cast<Dispatcher *>(data);
  d->add_timeout(reinterpret_cast<Timeout::Internal *>(timeout));
  return true;
}

void Dispatcher::Private::on_rem_timeout(DBusTimeout *timeout, void *data)
{
  auto d = static_cast<Dispatcher *>(data);
  d->rem_timeout(static_cast<Timeout *>(dbus_timeout_get_data(timeout)));
}

void Dispatcher::Private::on_toggle_timeout(DBusTimeout *timeout, void *data)
{
  static_cast<Timeout *>(dbus_timeout_get_data(timeout))->toggle();
}

void Dispatcher::queue_connection(Connection::Private *cp)
{
  std::lock_guard<std::mutex> lck(_mutex_p);
  _pending_queue.push_back(cp);
}


bool Dispatcher::has_something_to_dispatch()
{
  std::lock_guard<std::mutex> lck(_mutex_p);
  for (const auto& pq : _pending_queue)
    if (pq->has_something_to_dispatch())
      return true;
  return false;
}


void Dispatcher::dispatch_pending()
{
  while (true) {
    _mutex_p.lock();
    if (_pending_queue.empty()) {
      _mutex_p.unlock();
      break;
    }

    Connection::PrivatePList pending_queue_copy(_pending_queue);
    _mutex_p.unlock();

    const auto copy_elem_num = pending_queue_copy.size();
    dispatch_pending(pending_queue_copy);

    //only push_back on list is mandatory!
    _mutex_p.lock();

    auto iter = _pending_queue.begin();
    size_t counter = 0;
    while (counter < copy_elem_num && iter != _pending_queue.end()) {
      auto tmp_iter = iter;
      ++tmp_iter;
      _pending_queue.erase(iter);
      iter = tmp_iter;
      ++counter;
    }

    _mutex_p.unlock();
  }
}

void Dispatcher::dispatch_pending(Connection::PrivatePList &pending_queue)
{
  // SEEME: dbus-glib is dispatching only one message at a time to not starve the loop/other things...

  std::lock_guard<std::mutex> lck(_mutex_p_copy);
  while (pending_queue.size() > 0) {
    auto iter = pending_queue.begin();
    while (iter != pending_queue.end()) {
      auto tmp_iter = iter;

      ++tmp_iter;

      if ((*iter)->do_dispatch())
        pending_queue.erase(iter);
      else
        debug_log("dispatch_pending_private: do_dispatch error");

      iter = tmp_iter;
    }
  }
}

void DBus::_init_threading()
{
#ifdef DBUS_HAS_THREADS_INIT_DEFAULT
  dbus_threads_init_default();
#else
  debug_log("Thread support is not enabled! Your D-Bus version is too old!");
#endif//DBUS_HAS_THREADS_INIT_DEFAULT
}

