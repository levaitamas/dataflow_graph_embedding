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

#ifndef EMBED_ROUNDROBIN_H
#define EMBED_ROUNDROBIN_H

#include <random>
#include <lemon/smart_graph.h>

#include "cpu.h"
#include "embed-common.h"
#include "flow.h"
#include "utils.h"

using namespace lemon;


EmbeddingResult _embed_rr_conflictfree(const SmartDigraph& g,
				       const SmartGraph& cg,
				       const std::vector<Cpu>& cpus,
				       const std::vector<Flow>& flows,
				       const std::vector<Module>& modules,
				       bool max_obj_func = false)
{
  EmbeddingResult retval;
  size_t idx = 0;

  for (const auto& module : modules)
    {
      retval.mapping[g.id(module.node())] = idx;
      idx = (idx + 1) % cpus.size();
    }

  retval.sol_value = get_flow_crossings(g, retval.mapping, flows, max_obj_func);

  return retval;
}


EmbeddingResult _embed_rr_conflicts(const SmartDigraph& g,
				    const SmartGraph& cg,
				    const std::vector<Cpu>& cpus,
				    const std::vector<Flow>& flows,
				    const std::vector<Module>& modules,
				    bool max_obj_func = false)
{
  EmbeddingResult retval;
  size_t idx = 0;
  for (const auto& module : modules)
    // init retval.mapping to a probably invalid value
    retval.mapping[g.id(module.node())] = -1;

  for (const auto& module : modules)
    {
      size_t start_idx = idx;
      bool done = false;
      std::set<int> conflict_ids = get_conflict_ids(module, g, cg);
      while(done == false)
	{
	  bool no_go = false;
	  for (const auto& c : conflict_ids)
	    if (retval.mapping[c] == cpus[idx].id())
	      {
		no_go = true;
		break;
	      }

	  if (cpus[idx].load() + module.weight() > cpus[idx].capacity())
	    // not enough resource,
	    // cpu is not free to use
	    no_go = true;

	  if (no_go == false)
	    for (const auto& cpu_module : cpus[idx].modules())
	      if (conflict_ids.find(g.id(cpu_module.node())) != conflict_ids.end())
		// conflicting module on cpu,
		// cpu is not free to use
		no_go = true;

	  if (no_go == false)
	    {
	      retval.mapping[g.id(module.node())] = cpus[idx].id();
	      done = true;
	    }

	  idx = (idx + 1) % cpus.size();

	  if (idx == start_idx)
	    throw runtime_error("Embedding not possible: out of available CPUs");
	}
    }

  retval.sol_value = get_flow_crossings(g, retval.mapping, flows, max_obj_func);

  return retval;
}


EmbeddingResult embed_roundrobin(const SmartDigraph& g,
				 const SmartGraph& cg,
				 const std::vector<Cpu>& cpus,
				 const std::vector<Flow>& flows,
				 const std::vector<Module>& modules,
				 bool max_obj_func = false)
{
 if (countEdges(cg) == 0)
    return _embed_rr_conflictfree(g, cg, cpus, flows, modules, max_obj_func);
  else
    return _embed_rr_conflicts(g, cg, cpus, flows, modules, max_obj_func);
}


# endif  // EMBED_ROUNDROBIN_H
