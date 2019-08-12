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

#ifndef EMBED_COMMON_H
#define EMBED_COMMON_H


#include <algorithm>
#include <lemon/smart_graph.h>
#include <numeric>
#include <vector>

#include "cpu.h"
#include "flow.h"
#include "module.h"


struct EmbeddingResult
{
  // stores embedding result
  long sol_value = 0;  // objective function's solution
  std::map<std::size_t, std::size_t> mapping;  // node_id: cpu_num
};


long get_flow_crossings(const lemon::SmartDigraph& g,
			const std::map<size_t, size_t>& mapping,
			const std::vector<Flow>& flows,
			bool max_flow_crossings)
{
  // calculates flow crossings
  std::vector<long> flow_sums;
    for (const auto& f : flows)
      {
	size_t cross_sum = 0;
	for (size_t i = 0; i < f.modules().size()-1; i++)
	  {
	    size_t u_cpu = mapping.at(g.id(f.modules()[i].node()));
	    size_t v_cpu = mapping.at(g.id(f.modules()[i+1].node()));
	    if (u_cpu != v_cpu)
	      cross_sum += 1;
	  }
	flow_sums.push_back(cross_sum);
      }

  if (max_flow_crossings == true)
    return *std::max_element(flow_sums.begin(), flow_sums.end());
  else
    return std::accumulate(flow_sums.begin(), flow_sums.end(), 0);
}


std::set<int> get_conflict_ids(const Module& module,
			       const lemon::SmartDigraph& dfg,
			       const lemon::SmartGraph& cg)
{
  // get a set of conflicting modules' IDs
  std::set<int> conflict_ids;
  lemon::SmartGraph::Node cg_module = cg.nodeFromId(dfg.id(module.node()));
  for (lemon::SmartGraph::IncEdgeIt e(cg, cg_module); e != lemon::INVALID; ++e)
    {
      conflict_ids.insert(cg.id(cg.u(e)));
      conflict_ids.insert(cg.id(cg.v(e)));
    }
  conflict_ids.erase(cg.id(cg_module));
  return conflict_ids;
}


class FlowStat
{
 public:
  FlowStat(const Flow& flow, const EmbeddingResult& res, const lemon::SmartDigraph& dfg)
    {
      std::set<size_t> cpus_used;
      flow_name = flow.name();
      for (size_t i = 0; i < flow.modules().size() - 1; ++i)
	{
	  size_t cur_cpu = res.mapping.at(dfg.id(flow.modules().at(i).node()));
	  size_t next_cpu = res.mapping.at(dfg.id(flow.modules().at(i+1).node()));
	  cpus_used.insert(cur_cpu);
	  if (cur_cpu != next_cpu)
	    ++crossings;
	}
      cpus = cpus_used.size();
    }

  friend std::ostream& operator<<(std::ostream& out, const FlowStat& f);

  std::string flow_name;
  size_t crossings = 0;
  size_t cpus = 0;

};

std::ostream& operator<<(std::ostream& out, const FlowStat& fs)
{
  return out << fs.flow_name << ": crossings: " <<  fs.crossings
	     << ", cpus: " << fs.cpus;
}


#endif  // EMBED_COMMON_H
