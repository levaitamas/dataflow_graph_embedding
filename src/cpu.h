/*
 * Copyright (C) 2019-     Tamás Lévai    <levait@tmit.bme.hu>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CPU_H
#define CPU_H

#include <vector>
#include <sstream>

#include "module.h"
#include "utils.h"


class Cpu
{
 public:
  Cpu() {}
  Cpu(size_t id, float capacity)
    {
      id_ = id;
      capacity_ = capacity;
    }

  friend std::ostream& operator<<(std::ostream& out, const Cpu& c);

  const size_t& id() const { return id_; }
  const float& capacity() const { return capacity_; }
  const float& load() const { return load_; }
  const std::vector<Module>& modules() const { return modules_; }

  void add_module(const Module& mod)
  {
    modules_.push_back(mod);
    load_ += mod.weight();
  }

 private:
  size_t id_ = 0;
  float capacity_ = 0;
  float load_ = 0;
  std::vector<Module> modules_;
};

std::ostream& operator<<(std::ostream& out, const Cpu& c)
{
  return out << "CPU" << c.id()
	     << " (" << c.load() << "/" << c.capacity() << "):  "
	     << print_modules(c.modules());
}

#endif  // CPU_H
