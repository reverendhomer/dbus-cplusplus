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


#ifndef __DBUSXX_UTIL_H
#define __DBUSXX_UTIL_H

#include <sstream>
#include <iostream>
#include <iomanip>
#include <memory>
#include <cassert>

#include "api.h"
#include "debug.h"

namespace DBus
{

/*
 *   Typed callback template
 */

template <class R, class P>
class Callback_Base
{
public:
  virtual R call(P param) const = 0;
  virtual ~Callback_Base() = default;
};

template <class R, class P>
class Slot
{
public:

  Slot &operator = (Callback_Base<R, P>* s)
  {
    _cb.reset(s);
    return *this;
  }

  R operator()(P param) const
  {
    if (!empty())
      return _cb->call(param);

    // TODO: think about return type in this case
    // this assert should help me to find the use case where it's needed...
    //assert (false);
  }

  R call(P param) const
  {
    if (!empty())
      return _cb->call(param);

    // TODO: think about return type in this case
    // this assert should help me to find the use case where it's needed...
    //assert (false);
  }

  inline bool empty() const noexcept
  {
    return (bool)_cb;
  }

private:

  std::shared_ptr< Callback_Base<R, P> > _cb;
};

template <class C, class R, class P>
class Callback : public Callback_Base<R, P>
{
public:

  using M = R(C::*)(P);

  Callback(C *c, M m)
    : _c(c), _m(m)
  {}

  R call(P param) const
  {
    /*if (_c)*/ return (_c->*_m)(param);
  }

private:

  C *_c;
  M _m;
};

} /* namespace DBus */

#endif//__DBUSXX_UTIL_H
