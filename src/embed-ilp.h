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

#ifndef EMBED_ILP_H
#define EMBED_ILP_H

#include <vector>
#include <stdexcept>
#include <lemon/smart_graph.h>
#include <lemon/lp.h>

#include "cpu.h"
#include "embed-common.h"
#include "flow.h"
#include "utils.h"

using namespace lemon;
using namespace std;


EmbeddingResult embed_ilp(const SmartDigraph& g,
			  const SmartGraph& cg,
			  const vector<Cpu>& cpus,
			  const vector<Flow>& flows,
			  const vector<Module>& modules,
			  bool max_obj_func = false,
			  bool show_solver_log = true)
{
  ArcLookUp<SmartDigraph> arclookup(g);

  Mip mapping;
  if (show_solver_log == true)
    mapping.messageLevel(LpBase::MESSAGE_NORMAL);

  Lp::Expr obj_func;
  SmartDigraph::NodeMap<vector<LpBase::Col>> x(g);
  SmartDigraph::ArcMap<LpBase::Col> phi(g);

  mapping.addColSet(phi);

  map<int, float> module_weights;
  for (const auto& module : modules)
    module_weights[g.id(module.node())] = module.weight();

  for (SmartDigraph::NodeIt n(g); n != INVALID; ++n)
    {
      x[n].resize(cpus.size());
      for (size_t i = 0; i < x[n].size(); i++)
       	x[n][i] = mapping.addCol();
    }

  // x_{vi} \in {0,1}, \forall v,i
  for (SmartDigraph::NodeIt n(g); n != INVALID; ++n)
    {
      for (size_t i = 0; i < cpus.size(); i++)
	{
	  mapping.colType(x[n][i], Mip::INTEGER);
	  mapping.colLowerBound(x[n][i], 0);
	  mapping.colUpperBound(x[n][i], 1);
      }
    }

  // \sum\limits_{i \in N} x_{vi} = 1, \forall v \in V
  for (SmartDigraph::NodeIt n(g); n != INVALID; ++n)
    {
      Lp::Expr e;
      for (size_t i = 0; i < cpus.size(); i++)
	e += x[n][i];
      mapping.addRow(e == 1);
    }

  // \sum\limits_{v \in V} w_{v} x_{vi} \leq C
  for (size_t i = 0; i < cpus.size(); i++)
    {
      Lp::Expr e;
      for (SmartDigraph::NodeIt n(g); n != INVALID; ++n)
  	e += module_weights[g.id(n)] * x[n][i];
      mapping.addRow(e <= cpus[0].capacity());
    }

  // \phi(u,v) \ge x_{ui} - x_{vi}  \forall (u,v) \in A, \forall i \in N
  for (SmartDigraph::ArcIt a(g); a != INVALID; ++a)
    {
      SmartDigraph::Node s = g.source(a);
      SmartDigraph::Node t = g.target(a);
      for (size_t i = 0; i < cpus.size(); i++)
	{
  	  mapping.addRow((x[s][i] - x[t][i]) <= phi[a]);
  	}
    }

  // x_{ui} + x_{vi} \le 1 \forall (u,v) \in E
  for (SmartGraph::EdgeIt e(cg); e != INVALID; ++e)
    {
      SmartDigraph::Node n = g.nodeFromId(cg.id(cg.u(e)));
      SmartDigraph::Node m = g.nodeFromId(cg.id(cg.v(e)));
      for (size_t i = 0; i < cpus.size(); i++)
	{
	  mapping.addRow((x[n][i] +  x[m][i]) <= 1);
	}
    }

  // objective func
  if (max_obj_func == true)
    {
      // \min \alpha:
      // \alpha \ge \sum_{(u,v) \in p_f} \phi(u,v)    \forall f \in F
      LpBase::Col alpha = mapping.addCol();
      for (const auto& f : flows)
	{
	  Lp::Expr flow_sum;
	  for (size_t i = 0; i < f.modules().size()-1; i++)
	    {
	      flow_sum += phi[arclookup(f.modules()[i].node(),
					f.modules()[i+1].node())];
	    }
	  mapping.addRow(flow_sum <= alpha);
	}
      obj_func += alpha;
    }
  else
    {
      // \sum_{f\in F} \sum_{(u,v) \in p_f} \phi(u,v)
      for (const auto& f : flows)
	  for (size_t i = 0; i < f.modules().size()-1; i++)
	    // NB: segfault here if a module is missing from a flow
	    // definition
	    obj_func += phi[arclookup(f.modules()[i].node(),
				      f.modules()[i+1].node())];
    }

  mapping.min();
  mapping.obj(obj_func);
  mapping.solve();

  // mapping.write("/tmp/dfg.lp", "lp");

  if (mapping.type() != Mip::OPTIMAL)
    throw runtime_error("Optimal solution not found");

  EmbeddingResult retval;
  for (SmartDigraph::NodeIt n(g); n != INVALID; ++n)
    {
      size_t cpu_id = 0;
      while (cpu_id < cpus.size() - 1 && mapping.sol(x[n][cpu_id]) != 1)
	++cpu_id;
      retval.mapping[g.id(n)] = cpu_id;
    }
  retval.sol_value = mapping.solValue();
  return retval;
}

#endif  // EMBED_ILP_H
