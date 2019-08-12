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

#ifndef UTILS_H
#define UTILS_H

#include <cmath>
#include <iostream>
#include <numeric>
#include <sstream>


struct FlowSection
{
  // helper struct for parsing LGF @flows section
  std::map<std::string, std::string>& _data;
  FlowSection(std::map<std::string, std::string>& data) : _data(data) {}
  void operator()(const std::string& line)
  {
    std::istringstream ls(line);
    std::string key, value;
    while (ls >> key && ls >> value) _data[key] = value;
  }
};


struct ConflictSection
{
  typedef std::pair<std::string,std::string> string_pair;
  // helper struct for parsing LGF @conflicts section
  std::vector<string_pair>& _data;
  ConflictSection(std::vector<string_pair>& data) : _data(data) {}
  void operator()(const std::string& line)
  {
    std::istringstream ls(line);
    std::string u, v;
    while (ls >> u && ls >> v)
      _data.push_back(string_pair(u, v));
  }
};


std::vector<std::string> split_string_to_vec(const std::string &input) {
  // based on https://stackoverflow.com/a/11719617
  std::istringstream ss(input);
  std::string token;
  std::vector<std::string> retval;
  while(std::getline(ss, token, ','))
    retval.push_back(token);
  return retval;
}


std::string print_modules(const std::vector<Module>& modules,
			  const std::string separator = ", ",
			  bool name_only = true)
{
  std::stringstream ss;
  for (size_t i = 0; i < modules.size(); ++i)
    {
      if (i != 0)
	ss << separator;

      if (name_only == true)
	ss << modules[i].name();
      else
	ss << modules[i];
    }
  return ss.str();
}

template<typename T>
float calc_stdev(const std::vector<T>& values, float sum_values = 0.0)
{
  // based on https://stackoverflow.com/a/7616783
  if (sum_values == 0.0)
    sum_values = std::accumulate(values.begin(), values.end(), 0.0);
  float mean = sum_values / values.size();
  std::vector<float> diff(values.size());
  std::transform(values.begin(), values.end(),
		 diff.begin(), [mean](float x) { return x - mean; });
  float sqsum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
  float stdev = std::sqrt(sqsum / (values.size() - 1));
  return stdev;
}


#endif  // UTILS_H
