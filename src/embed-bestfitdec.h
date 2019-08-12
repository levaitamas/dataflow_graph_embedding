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

#ifndef EMBED_BESTFITDEC_H
#define EMBED_BESTFITDEC_H

#include <vector>
#include <stdexcept>
#include <lemon/smart_graph.h>

#include "embed-common.h"


using namespace lemon;



struct CpuBin
{
  size_t cpu_id;
  float free_cap;
  CpuBin(const size_t& id, float free_capacity)
  {
    cpu_id = id;
    free_cap = free_capacity;
  }
};

bool compare_cpubin_cap_decr(const CpuBin a, const CpuBin b)
{
  return (b.free_cap > a.free_cap);
}

bool compare_module_weight_decr(const Module* m, const Module* n)
{
  return (m->weight() > n->weight());
}


EmbeddingResult _embed_bfd_conflictfree(const SmartDigraph& g,
					const SmartGraph& cg,
					const std::vector<Cpu>& cpus,
					const std::vector<Flow>& flows,
					const std::vector<Module>& modules,
					bool max_obj_func = false)
{
  EmbeddingResult retval;

  // init bins (with size of free cpu capacities)
  std::vector<CpuBin> bins;
  for (const auto& cpu : cpus)
    bins.push_back(CpuBin(cpu.id(), cpu.capacity() - cpu.load()));

  // sort modules
  std::vector<const Module*> modules_sorted;
  for (const auto& module : modules)
    modules_sorted.push_back(&module);
  std::sort(modules_sorted.begin(), modules_sorted.end(), compare_module_weight_decr);

  for (const auto& module_ptr : modules_sorted)
    {
      const Module& module = *module_ptr;
      size_t idx = -1;

      std::sort(bins.begin(), bins.end(), compare_cpubin_cap_decr);
      for (auto& bin : bins)
	{
	  float val = bin.free_cap - module.weight();
	  if (val >= 0)
	    {
	      idx = bin.cpu_id;
	      bin.free_cap = val;
	      break;
	    }
	}
      if (idx > cpus.size())
	throw std::runtime_error("Embedding not possible: out of available CPUs");

      retval.mapping[g.id(module.node())] = idx;
    }

  retval.sol_value = get_flow_crossings(g, retval.mapping, flows, max_obj_func);
  return retval;
}


EmbeddingResult _embed_bfd_conflicts(const SmartDigraph& g,
				     const SmartGraph& cg,
				     const std::vector<Cpu>& cpus,
				     const std::vector<Flow>& flows,
				     const std::vector<Module>& modules,
				     bool max_obj_func = false)
{
  EmbeddingResult retval;

  // init bins (with size of free cpu capacities)
  std::vector<CpuBin> bins;
  for (const auto& cpu : cpus)
    bins.push_back(CpuBin(cpu.id(), cpu.capacity() - cpu.load()));

  // sort modules
  std::vector<const Module*> modules_sorted;
  for (const auto& module : modules)
    modules_sorted.push_back(&module);
  std::sort(modules_sorted.begin(), modules_sorted.end(), compare_module_weight_decr);

  for (const auto& module_ptr : modules_sorted)
    {
      const Module& module = *module_ptr;
      std::set<int> conflict_ids = get_conflict_ids(module, g, cg);
      size_t idx = -1;

      std::sort(bins.begin(), bins.end(), compare_cpubin_cap_decr);
      for (auto& bin : bins)
	{
	  // check cpu eligability
	  bool no_go = false;
	  for (const auto& c : conflict_ids)
	    if (retval.mapping[c] == cpus[bin.cpu_id].id())
	      {
		no_go = true;
		break;
	      }
	  if (no_go == false)
	    for (const auto& cpu_module : cpus[bin.cpu_id].modules())
	      if (conflict_ids.find(g.id(cpu_module.node())) != conflict_ids.end())
		{
		  no_go = true;
		  break;
		}

	  if (no_go == true)
	    // if cpu is  not eligable, try next one
	    continue;

	  float val = bin.free_cap - module.weight();
	  if (val >= 0)
	    {
	      idx = bin.cpu_id;
	      bin.free_cap = val;
	      break;
	    }
	}
      if (idx > cpus.size())
	throw std::runtime_error("Embedding not possible: out of available CPUs");

      retval.mapping[g.id(module.node())] = idx;
    }

  retval.sol_value = get_flow_crossings(g, retval.mapping, flows, max_obj_func);
  return retval;
}


EmbeddingResult embed_bestfitdecreasing(const SmartDigraph& g,
					const SmartGraph& cg,
					const std::vector<Cpu>& cpus,
					const std::vector<Flow>& flows,
					const std::vector<Module>& modules,
					bool max_obj_func = false)
{
  if (countEdges(cg) == 0)
    return _embed_bfd_conflictfree(g, cg, cpus, flows, modules, max_obj_func);
  else
    return _embed_bfd_conflicts(g, cg, cpus, flows, modules, max_obj_func);
}


# endif  // EMBED_BESTFITDEC_H
