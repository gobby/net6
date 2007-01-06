/* net6 - library providing ipv4/ipv6 network access
 * Copyright (C) 2005 Armin Burgmeier / 0x539 dev group
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _NET6_PACKET_HPP_
#define _NET6_PACKET_HPP_

#include <vector>
#include "export.hpp"

namespace net6
{

class NET6_EXPORT packet
{
public:
  class NET6_EXPORT param
  {
  public:
    enum type_type {
      INT,
      FLOAT,
      STRING
    };
      
    param();
    param(int val);
    param(float val);
    param(const std::string& val);
    param(const param& other);
    ~param();

    param& operator=(const param& other);

    int as_int() const;
    float as_float() const;
    const std::string& as_string() const;

    type_type get_type() const;
  protected:
    void clear_memory();

    union data_type
    {
      int i;
      float f;
      std::string* s;
    };
      
    type_type type;
    data_type data;
  };
    
  packet();
  packet(const std::string& command, unsigned int size = 0);
  packet(const packet& other);
  ~packet();

  packet& operator=(const packet& other);

  template<typename T> packet& operator<<(const T& val)
  {
    params.push_back(param(val) );
    return *this;
  }

  const std::string& get_command() const;

  const param& get_param(unsigned int index) const;
  unsigned int get_param_count() const;

  std::string get_raw_string() const;
  void set_raw_string(const std::string& raw_string);

protected:
  static std::string escape(const std::string& string);
  static std::string unescape(const std::string& string);

  void set_raw_param(const std::string& param_string);

  std::string command;
  std::vector<param> params;
};

}

#endif

