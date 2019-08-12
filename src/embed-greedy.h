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

#ifndef EMBED_GREEDY_H
#define EMBED_GREEDY_H

#include <vector>

#include "embed-common.h"

using namespace lemon;


EmbeddingResult _embed_greedy_conflictfree(const SmartDigraph& g,
					   const SmartGraph& cg,
					   const std::vector<Cpu>& cpus,
					   const std::vector<Flow>& flows,
					   const std::vector<Module>& modules,
					   bool max_obj_func = false)
{
  // TODO

  EmbeddingResult retval;

  return retval;
}


EmbeddingResult _embed_greedy_conflicts(const SmartDigraph& g,
					const SmartGraph& cg,
					const std::vector<Cpu>& cpus,
					const std::vector<Flow>& flows,
					const std::vector<Module>& modules,
					bool max_obj_func = false)
{
  // TODO

  EmbeddingResult retval;

  return retval;
}


EmbeddingResult embed_greedy(const SmartDigraph& g,
			     const SmartGraph& cg,
			     const std::vector<Cpu>& cpus,
			     const std::vector<Flow>& flows,
			     const std::vector<Module>& modules,
			     bool max_obj_func = false)
{
 if (countEdges(cg) == 0)
    return _embed_greedy_conflictfree(g, cg, cpus, flows, modules, max_obj_func);
  else
    return _embed_greedy_conflicts(g, cg, cpus, flows, modules, max_obj_func);
}


# endif  // EMBED_GREEDY_H
