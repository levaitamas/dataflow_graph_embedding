#!/usr/bin/env python3
#
# Copyright (C) 2019-     Tamás Lévai    <levait@tmit.bme.hu>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
"""
Helper script to convert a running BESS pipeline into an LGF
(LemonGraphFormat) file.

"""
import argparse
import os
import sys


def get_topo(bess):
    """Get topology from running BESS pipeline

    Parameters:
    bess (pybess.BESS): Connected BESS handler

    Returns:
    topo (dict): BESS module topology

    """
    topo = {}
    for module in bess.list_modules().modules:
        info = bess.get_module_info(module.name)
        topo[info.name] = {'successors': [n.name for n in info.ogates]}
    return topo


def convert_topo(topo):
    """Convert topology for further processing

    Parameters:
    topo (dict): BESS module topology

    Returns:
    nodes (dict): Modules, key: module name, value: assigned id
    arcs (list): Each arc is represented as a tuple of ids of source
    and target nodes

    """
    nodes = {n: i for i, n in enumerate(topo.keys())}
    arcs = []
    for source, details in topo.items():
        for target in details['successors']:
            arcs.append((nodes[source], nodes[target]))
    return nodes, arcs


def print_lgf(nodes, arcs):
    """Print LGF file content to stdout

    Parameters:
    nodes (dict): Modules, key: module name, value: assigned id
    arcs (list): Each arc is represented as a tuple of ids of source
    and target nodes

    """
    print('@nodes')
    print('label\tname')
    for name in nodes:
        print(f'{nodes[name]}\t{name}\t')

    print()
    print('@arcs')
    print('\t\tlabel')
    for label, arc in enumerate(arcs):
        print(f'{arc[0]}\t{arc[1]}\t{label}')


def write_lgf(nodes, arcs, out_file):
    """Write LGF file content to file

    Parameters:
    nodes (dict): Modules, key: module name, value: assigned id
    arcs (list): Each arc is represented as a tuple of ids of source
    and target nodes
    out_file (str): Path to write LGF

    """
    with open(out_file, 'w') as outfile:
        nodes_header = '@nodes\nlabel\tname\n'
        nodes_content = '\n'.join([f'{nodes[name]}\t{name}' for name in nodes])
        arcs_header = '@arcs\n\t\tlabel\n'
        arcs_content = '\n'.join([f'{arc[0]}\t{arc[1]}\t{label}'
                                  for label, arc in enumerate(arcs)])
        outfile.write(f'{nodes_header}{nodes_content}\n\n'
                      f'{arcs_header}{arcs_content}\n')


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--bessdir', '-b', type=str,
                        default='/opt/bess',
                        help='BESS root directory')
    parser.add_argument('--outfile', '-o', type=str,
                        help='Outfile (LGF)')
    args = parser.parse_args()

    bessdir = os.path.expanduser(args.bessdir)
    try:
        sys.path.insert(1, bessdir)
        from pybess.bess import BESS
    except ImportError as exc:
        text = f'{exc}.\nCannot import (pybess) API module from {args.bessdir}'
        raise Exception(text) from None

    bess = BESS()

    try:
        bess.connect()
    except (BESS.APIError, BESS.RPCError) as exc:
        text = f'{exc}.\nPerhaps BESS daemon is not running locally?'
        raise Exception(text) from None

    nodes, arcs = convert_topo(get_topo(bess))

    print_lgf(nodes, arcs)

    if args.outfile:
        write_lgf(nodes, arcs, args.outfile)
