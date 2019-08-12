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

#include<chrono>
#include <iostream>
#include <string>
#include <vector>
#include <lemon/arg_parser.h>
#include <lemon/lgf_reader.h>
#include <lemon/lgf_writer.h>
#include <lemon/smart_graph.h>

#include "cpu.h"
#include "embed-bestfitdec.h"
#include "embed-common.h"
#include "embed-greedy.h"
#include "embed-ilp.h"
#include "embed-random.h"
#include "embed-roundrobin.h"
#include "flow.h"
#include "module.h"
#include "utils.h"

using namespace lemon;


int main(int argc, char **argv)
{
  ArgParser ap(argc, argv);

  std::string in_file = "config/pipeline.lgf";
  std::string method = "ilp";
  bool max_obj_func = false;
  bool show_solver_log = false;

  ap.refOption("infile",
	       "Input pipeline desrciption LGF",
	       in_file,
	       false);
  ap.refOption("showlog",
	       "Show log messages of ILP solver",
	       show_solver_log,
	       false);
  ap.refOption("maxflow",
	       "Use the max per-flow crossing metric instead of sum crossings",
	       max_obj_func,
	       false);
  ap.refOption("method",
	       "Embedding method to use. [ilp, bestfitdec, random, roundrobin]",
	       method,
	       false);
  ap.synonym("i", "infile");
  ap.synonym("s", "showlog");
  ap.synonym("M", "maxflow");
  ap.synonym("m", "method");
  ap.parse();

  SmartDigraph dfg;
  SmartDigraph::NodeMap<std::string> module_name(dfg);
  SmartDigraph::NodeMap<float> module_weight(dfg);
  std::map<std::string, Module> module_lookup_map;
  std::vector<Module> modules;

  size_t cpu_number;
  float cpu_capacity;
  std::vector<Cpu> cpus;

  std::map<std::string, std::string> flow_sections;
  std::vector<Flow> flows;

  bool has_conflicts = true;
  std::vector<std::pair<std::string, std::string>> conflict_sections;
  SmartGraph cg;

  // read LGF file
  try {
    digraphReader(dfg, in_file).
      nodeMap("weight", module_weight).
      nodeMap("name", module_name).
      attribute("cpu_number", cpu_number).
      attribute("cpu_capacity", cpu_capacity).
      run();

    sectionReader(in_file).
      sectionLines("flows", FlowSection(flow_sections)).
      run();

  } catch (Exception& error) {
    std::cerr << "Error: " << error.what() << std::endl;
    return -1;
  }

  try {
      sectionReader(in_file).
	sectionLines("conflicts", ConflictSection(conflict_sections)).
	run();
  } catch (Exception& error) {
    has_conflicts = false;
  }

  // init modules
  // construct module lookup map to get module by name
  for (SmartDigraph::NodeIt n(dfg); n != INVALID; ++n)
    {
      Module mod = Module(n, module_name[n], module_weight[n]);
      module_lookup_map[module_name[n]] = mod;
      modules.push_back(mod);
    }

  if (has_conflicts == true)
    {
      // init conflict graph
      for (SmartDigraph::NodeIt n(dfg); n != INVALID; ++n)
	  cg.addNode();
      for (const auto& it : conflict_sections)
          cg.addEdge(cg.nodeFromId(std::stoi(it.first)),
		     cg.nodeFromId(std::stoi(it.second)));
    }

  // init flows
  for (const auto& it : flow_sections)
    {
    std::vector<Module> path;
    for (const auto& mname : split_string_to_vec(it.second))
	path.push_back(module_lookup_map[mname]);
    flows.push_back(Flow(it.first, path));
    }

  // init CPUs
  for (size_t i = 0; i < cpu_number; i++)
    {
      cpus.push_back(Cpu(i, cpu_capacity));
    }

  // embed
  EmbeddingResult res;

  // convert method name to lowercase
  for (auto& c : method)
    c = std::tolower(c);

  std::chrono::high_resolution_clock::time_point t_before = std::chrono::high_resolution_clock::now();

  if (method == "ilp")
    {
      res = embed_ilp(dfg, cg, cpus, flows, modules, max_obj_func, show_solver_log);
    }
  else if (method == "greedy" || method == "g")
    {
      res = embed_greedy(dfg, cg, cpus, flows, modules, max_obj_func);
    }
  else if (method == "bestfitdec" || method == "bfd")
    {
      res = embed_bestfitdecreasing(dfg, cg, cpus, flows, modules, max_obj_func);
    }
  else if (method == "random" || method == "rnd")
    {
      res = embed_random(dfg, cg, cpus, flows, modules, max_obj_func);
    }
  else if (method == "roundrobin" || method == "rr")
    {
      res = embed_roundrobin(dfg, cg, cpus, flows, modules, max_obj_func);
    }
  else
    throw runtime_error("Invalid method");

  std::chrono::high_resolution_clock::time_point t_after = std::chrono::high_resolution_clock::now();
  auto embed_duration = std::chrono::duration_cast<std::chrono::microseconds>(t_after-t_before).count();

  for (const auto& it : res.mapping)
    {
      Module& mod = module_lookup_map[module_name[dfg.nodeFromId(it.first)]];
      cpus[it.second].add_module(mod);
    }

  // verificate results
  for (const auto& it : conflict_sections)
    {
      int u = std::stoi(it.first);
      int v = std::stoi(it.second);
      if (res.mapping.at(u) == res.mapping.at(v))
  	throw runtime_error("Invalid mapping!");
    }

  // print results
  std::cout << std::endl << "* Modules" << std::endl;
  for (const auto& module : modules)
    std::cout << module << " [" << dfg.id(module.node()) << "]" << std::endl;

  std::cout  << std::endl << "* Flows" << std::endl;
  for (const auto& f : flows)
    std::cout << f << std::endl;

  std::cout  << std::endl << "* CPUs" << std::endl;
  for (const auto& c : cpus)
    std::cout << c << std::endl;

  std::cout  << std::endl << "* Objective function"
	     << std::endl << "value: " << res.sol_value << std::endl;

  // print stats
  std::vector<FlowStat> flow_stats;
  for (const auto& flow : flows)
      flow_stats.push_back(FlowStat(flow, res, dfg));

  std::cout  << std::endl << "* Flow stats" << std::endl;

  std::vector<size_t> flow_crossings;
  std::vector<size_t> flow_cpus;
  for (const auto& fs : flow_stats)
    {
      flow_crossings.push_back(fs.crossings);
      flow_cpus.push_back(fs.cpus);
      std::cout << fs << std::endl;
    }
  float sum_flow_crossings = std::accumulate(flow_crossings.begin(), flow_crossings.end(), 0.0);
  float sum_flow_cpus = std::accumulate(flow_cpus.begin(), flow_cpus.end(), 0.0);

  std::cout << std::endl << "** cpus" << std::endl
	    << "sum: " <<  sum_flow_cpus << std::endl
    	    << "min: " << *std::min_element(flow_cpus.begin(), flow_cpus.end()) << std::endl
	    << "max: " << *std::max_element(flow_cpus.begin(), flow_cpus.end()) << std::endl
	    << "avg: " << sum_flow_cpus / flow_stats.size() << std::endl
	    << "std_dev: " << calc_stdev<size_t>(flow_cpus, sum_flow_cpus) << std::endl
	    << std::endl;

  std::cout << "** crossings" << std::endl
	    << "sum: " <<  sum_flow_crossings << std::endl
    	    << "min: " << *std::min_element(flow_crossings.begin(), flow_crossings.end()) << std::endl
	    << "max: " << *std::max_element(flow_crossings.begin(), flow_crossings.end()) << std::endl
	    << "avg: " <<  sum_flow_crossings / flow_stats.size() << std::endl
	    << "std_dev: " << calc_stdev<size_t>(flow_crossings, sum_flow_crossings) << std::endl
	    << std::endl;


  std::cout  << std::endl << "* CPU stats" << std::endl;

  std::vector<float> cpu_loads;
  for (const auto& cpu : cpus)
      cpu_loads.push_back(cpu.load());
  float sum_cpu_loads = std::accumulate(cpu_loads.begin(), cpu_loads.end(), 0.0);

  std::cout << std::endl << "** load" << std::endl
    	    << "available: " <<  cpus[0].capacity() * cpus.size() << std::endl
	    << "sum: " <<  sum_cpu_loads << std::endl
    	    << "min: " << *std::min_element(cpu_loads.begin(), cpu_loads.end()) << std::endl
	    << "max: " << *std::max_element(cpu_loads.begin(), cpu_loads.end()) << std::endl
	    << "avg: " <<  sum_cpu_loads / cpus.size() << std::endl
    	    << "std_dev: " <<  calc_stdev<float>(cpu_loads, sum_cpu_loads) << std::endl
	    << std::endl;

  std::cout << "* Execution time" << std::endl
	    << embed_duration << " us" << std::endl << std::endl;

  return 0;
}
