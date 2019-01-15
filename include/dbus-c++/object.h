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


#ifndef __DBUSXX_OBJECT_H
#define __DBUSXX_OBJECT_H

#include <string>
#include <list>
#include <memory>

#include "api.h"
#include "interface.h"
#include "connection.h"
#include "message.h"
#include "types.h"

namespace DBus
{

class DXXAPI Object
{
private:
  Connection	_conn;
  DBus::Path	_path;
  std::string	_service;
  int		_default_timeout;

protected:

  Object(Connection &conn, const Path &path, const char *service);

public:

  virtual ~Object() = default;

  inline const DBus::Path &path() const noexcept
  {
    return _path;
  }

  inline const std::string &service() const noexcept
  {
    return _service;
  }

  inline Connection &conn() noexcept
  {
    return _conn;
  }

  void set_timeout(int new_timeout = -1);

  inline int get_timeout() const noexcept
  {
    return _default_timeout;
  }

private:

  DXXAPILOCAL virtual bool handle_message(const Message &) = 0;
  DXXAPILOCAL virtual void register_obj() = 0;
  DXXAPILOCAL virtual void unregister_obj(bool throw_on_error = true) = 0;
};

/*
*/

class DXXAPI Tag
{
public:
  virtual ~Tag() = default;
};

/*
*/

class ObjectAdaptor;

using ObjectAdaptorPList = std::list<ObjectAdaptor *>;
using ObjectPathList = std::list<std::string>;

class DXXAPI ObjectAdaptor : public Object, public virtual AdaptorBase
{
public:

  static ObjectAdaptor *from_path(const Path &path);

  static ObjectAdaptorPList from_path_prefix(const std::string &prefix);

  static ObjectPathList child_nodes_from_prefix(const std::string &prefix);

  struct Private;

  ObjectAdaptor(Connection &conn, const Path &path);

  ~ObjectAdaptor();

  inline const ObjectAdaptor *object() const noexcept
  {
    return this;
  }

protected:

  class DXXAPI Continuation
  {
  private:
    Connection _conn;
    CallMessage _call;
    MessageIter _writer;
    ReturnMessage _return;
    const Tag *_tag;

    friend class ObjectAdaptor;

    Continuation(Connection &conn, const CallMessage &call, const Tag *tag);

  public:
    inline MessageIter &writer() noexcept
    {
      return _writer;
    }
    inline Tag *tag() noexcept
    {
      return const_cast<Tag *>(_tag);
    }
  };

  void return_later(const Tag *tag);

  void return_now(Continuation *ret);

  void return_error(Continuation *ret, const Error error);

  Continuation *find_continuation(const Tag *tag);

private:

  void _emit_signal(SignalMessage &);

  bool handle_message(const Message &);

  void register_obj();
  void unregister_obj(bool throw_on_error = true);

  std::map<const Tag *, std::unique_ptr<Continuation>> _continuations;

  friend struct Private;
};

/*
*/

class ObjectProxy;

using ObjectProxyPList = std::list<ObjectProxy *>;

class DXXAPI ObjectProxy : public Object, public virtual ProxyBase
{
public:

  ObjectProxy(Connection &conn, const Path &path, const char *service = "");

  ~ObjectProxy();

  inline const ObjectProxy *object() const noexcept
  {
    return this;
  }

private:

  Message _invoke_method(CallMessage &);

  bool _invoke_method_noreply(CallMessage &call);

  bool handle_message(const Message &);

  void register_obj();
  void unregister_obj(bool throw_on_error = true);

private:

  MessageSlot _filtered;
};

} /* namespace DBus */

#endif//__DBUSXX_OBJECT_H
