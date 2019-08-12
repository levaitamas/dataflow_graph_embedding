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

#ifndef MODULE_H
#define MODULE_H

#include <lemon/smart_graph.h>


class Module
{
 public:
  Module() {}
  Module(lemon::SmartDigraph::Node& node, std::string name = "", float weight = 0)
    {
      node_ = node;
      name_ = name;
      weight_ = weight;
    }

  friend std::ostream& operator<<(std::ostream& out, const Module& m);

  const std::string& name() const { return name_; }
  const float& weight() const { return weight_; }
  const lemon::SmartDigraph::Node& node() const { return node_; }

 private:
  std::string name_;
  float weight_;
  lemon::SmartDigraph::Node node_;
};

std::ostream& operator<<(std::ostream& out, const Module& m)
{
  return out << m.name() << ":  (" << m.weight() << ")";
}

#endif  // MODULE_H
