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

#ifndef EMBED_RANDOM_H
#define EMBED_RANDOM_H

#include <random>
#include <lemon/smart_graph.h>

#include "cpu.h"
#include "embed-common.h"
#include "flow.h"
#include "utils.h"

using namespace lemon;


EmbeddingResult _embed_random_conflictfree(const SmartDigraph& g,
					   const SmartGraph& cg,
					   const std::vector<Cpu>& cpus,
					   const std::vector<Flow>& flows,
					   const std::vector<Module>& modules,
					   bool max_obj_func = false)
{
  std::random_device rd;
  std::default_random_engine generator(rd());
  std::uniform_int_distribution<size_t> distribution(0, cpus.size()-1);

  EmbeddingResult retval;

  for (const auto& module : modules)
      retval.mapping[g.id(module.node())] = distribution(generator);

  retval.sol_value = get_flow_crossings(g, retval.mapping, flows, max_obj_func);

  return retval;
}


EmbeddingResult _embed_random_conflicts(const SmartDigraph& g,
					const SmartGraph& cg,
					const std::vector<Cpu>& cpus,
					const std::vector<Flow>& flows,
					const std::vector<Module>& modules,
					bool max_obj_func = false)
{
  std::random_device rd;
  std::default_random_engine generator(rd());

  EmbeddingResult retval;

  for (const auto& module : modules)
    // init retval.mapping to a probably invalid value
    retval.mapping[g.id(module.node())] = -1;

  for (const auto& module : modules)
    {
      std::vector<const Cpu*> available_cpus;
      std::set<int> conflict_ids = get_conflict_ids(module, g, cg);

      for (const auto& cpu : cpus)
	{
	  bool no_go = false;
	  for (const auto& c : conflict_ids)
	    if (retval.mapping[c] == cpu.id())
	      {
		no_go = true;
		break;
	      }

	  if (no_go == false)
	    {
	      if (cpu.modules().size() == 0)
		available_cpus.push_back(&cpu);
	      else
		for (const auto& cpu_module : cpu.modules())
		  if (conflict_ids.find(g.id(cpu_module.node())) == conflict_ids.end())
		    // no conflicting module on cpu,
		    if (cpu.load() + module.weight() <= cpu.capacity())
		      // enough resource is available,
		      // cpu is free to use
		      available_cpus.push_back(&cpu);
	    }
	}

      if (available_cpus.size() == 0 )
	    throw runtime_error("Embedding not possible: out of available CPUs");

      std::uniform_int_distribution<size_t> distribution(0, available_cpus.size()-1);
      retval.mapping[g.id(module.node())] = available_cpus[distribution(generator)]->id();
    }

  retval.sol_value = get_flow_crossings(g, retval.mapping, flows, max_obj_func);

  return retval;
}


EmbeddingResult embed_random(const SmartDigraph& g,
			     const SmartGraph& cg,
			     const std::vector<Cpu>& cpus,
			     const std::vector<Flow>& flows,
			     const std::vector<Module>& modules,
			     bool max_obj_func = false)
{
  if (countEdges(cg) == 0)
    return _embed_random_conflictfree(g, cg, cpus, flows, modules, max_obj_func);
  else
    return _embed_random_conflicts(g, cg, cpus, flows, modules, max_obj_func);
}


# endif  // EMBED_RANDOM_H
