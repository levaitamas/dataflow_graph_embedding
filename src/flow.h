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

#ifndef FLOW_H
#define FLOW_H

#include <vector>

#include "module.h"
#include "utils.h"


class Flow
{
 public:
  Flow() {}
  Flow(std::string name, const std::vector<Module>& modules)
    {
      name_ = name;
      modules_ = modules;
    }

  friend std::ostream& operator<<(std::ostream& out, const Flow& f);

  const std::string& name() const { return name_; }
  const std::vector<Module>& modules() const { return modules_; }

 private:
  std::string name_;
  std::vector<Module> modules_;
};

std::ostream& operator<<(std::ostream& out, const Flow& f)
{
  return out << f.name() << ":  " << print_modules(f.modules());
}

#endif  // FLOW_H
