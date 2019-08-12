# Dataflow Graph Embedding for Programmable Software Switches

[Overview](#overview) | [Installation](#installation) | [Usage](#usage) | [License](#license)

## Overview

Accompanying material for Tamas Levai and Gabor Retvari: [Resilient Dataflow Graph Embedding for Programmable Software Switches](papers/rndm19/RNDM19.pdf), in RNDM'19, 2019.

## Installation

### Dependencies

Recommended:

* [LEMON](https://lemon.cs.elte.hu/trac/lemon/wiki/Downloads)
* gcc, make, etc.
* milp solver supported by LEMON

Suggested:

* [Python](https://www.python.org/downloads) (>=3.6)
* [Gurobi](https://www.gurobi.com/downloads) (==8.1)
* a Gurobi interface for LEMON

### Compilation
1. Edit the [Makefile](src/Makefile) according to your taste

2. Run `make` in `src`

## Usage

### Running the Dataflow Graph Embedding Software
To start the software: run `dfg-embed` in `src`.

The main command-line parameters cover the following functions:

* `-infile <str>`: a custom LEMON Graph Format file as pipeline description.

* `-method <str>`: embedding method

* `-maxflow`: the metric to use for embedding


### The Input LEMON Graph Format File
The basics of LEMON Graph Format can be read [here](http://lemon.cs.elte.hu/pub/doc/1.3.1/a00004.html).

Modules are encoded in the @nodes section by `label` (LEMON specific), `name` (string), and `weight` (float).

Module interconnections are represented as arcs in section @arcs. An arc is defined by a the source module label and the destination label. Each arc must have a unique `label`.

CPU information is encoded in section @attributes by name-value pairs. The two required attributes are `cpu_number` and `cpu_capacity`. Each CPU can run modules with no overload if the sum of module weights is less or equal to the capacity.

Traffic flows are defined in section @flows. A name and a comma-separated list of traversing module names define a flow.

Module conflicts are defined by a pair of conflicting modules label in section @conflicts. Note: if the file contains @conflicts, it means embedding with conflicts automatically.

### Utilities
Helper scripts are located in [utils](utils/).

Use `gen_mgw_lgf.py` to generate mobile gateway configs.

To generate input LGF file from a running BESS pipeline, use `bess2lgf.py`.


### Usage Example
This simple example presents the workflow of (re)producing results using the embedding software in two steps:

1. Generate an input MGW config with 2 bearers, 10 users, 5 bearer0 users for 32 CPUs each having capacity of 120 units. The configuration is resilience against a single CPU failure:
```sh
python3 utils/gen_mgw_lgf.py -b 2 -u 10 -B 5 -c 120 -C 32 -l 1 -q -o src/config/out.lgf
```

2. Embed the generated configuration using the best fit decreasing-based heuristic:
```sh
src/dfg-embed -i src/config/out.lgf -m bfd
```

The results are presented on the standard output (note the org-compatible format of results).

## License

The presented software are free software and licensed under [GPLv3+](./LICENSE).
