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
Helper script to generate MGW (mobile gateway) config LGF
(LemonGraphFormat) file.

"""
import argparse


MODULE_WEIGHTS = {
    "mac_table": 1,
    "type_check": 1,
    "dir_selector": 1,
    "dl_br_selector": 20,
    "dl_ue_selector": 20,

    "vxlan_decap": 10,
    "ul_br_selector": 10,
    "ul_ue_selector": 10,
    "update_ttl": 1,
    "L3": 1,
    "update_mac_dl": 1,
    "ip_checksum_dl": 1,
    "update_mac_ul": 1,
    "ip_checksum_ul": 1,

    "dl_user_bp": 20,
    "setmd_dl": 20,
    "vxlan_encap": 20,
    "ip_encap": 20,
    "ether_encap": 20,

    "ul_user_bp": 10,
    "setmd_ul": 10,
}


class Module(object):
    """Represents a packet processing module of pipeline """

    def __init__(self, name, label, weight=-1, successors=None, conflicts=None):
        self.name = name
        self.label = label
        self.weight = weight
        self.successors = successors or set()
        self.conflicts = conflicts or set()


class Flow(object):
    """Represents a flow of pipeline """

    def __init__(self, name, modules=None):
        self.name = name
        self.modules = modules or []

    def list_modules(self):
        """ Generates a list of module names """
        return [module.name for module in self.modules]

    @staticmethod
    def get_flow_module_names(direction, bearer, user, conflict=None):
        """ Generates a string of flow modules

        Parameters:
        direction (str): Flow direction (upstream/downstream). Either 'u' or 'd'.
        bearer (int): Bearer id
        user (int): User id
        conflict (int): Conflict id

        Returns:
        a string of flow modules' name separated by ','

        """
        if direction == 'd':
            mods = ['mac_table', 'type_check', 'dir_selector', 'dl_br_selector',
                    f'dl_ue_selector_{bearer}', f'dl_user_bp_{user}_{bearer}',
                    f'setmd_dl_{user}_{bearer}', f'vxlan_encap_{user}_{bearer}',
                    f'ip_encap_{user}_{bearer}', f'ether_encap_{user}_{bearer}',
                    'update_ttl', 'L3', 'update_mac_dl', 'ip_checksum_dl']
        elif direction == 'u':
            mods = ['mac_table', 'type_check', 'dir_selector',
                    'vxlan_decap', 'ul_br_selector',
                    f'ul_ue_selector_{bearer}', f'ul_user_bp_{user}_{bearer}',
                    f'setmd_ul_{user}_{bearer}',
                    'update_ttl', 'L3', 'update_mac_ul', 'ip_checksum_ul']
        else:
            err_text = "Flow direction (upstream/downstream) must be either 'u' or 'd'"
            raise ValueError(err_text)

        if conflict:
            mods = [f'{name}-c{conflict}' for name in mods]
        return ','.join(mods)


class Config(object):
    """MGW config based on pipeline arguments """

    def __init__(self, nodes=None, arcs=None, attributes=None, flows=None, conflicts=None):
        """
        Parameters:
        nodes (list): List of Module objects
        arcs (int): List of arc tuples
        attributes (dict): LGF attributes
        flows (list): List of Flow objects
        conflicts (list): List of conflict tuples

        """
        self.nodes = nodes or []
        self.arcs = arcs or []
        self.attributes = attributes or {}
        self.flows = flows or []
        self.conflicts = conflicts or []

    def print_lgf(self):
        """Print LGF file content to stdout """
        print('@nodes')
        print('label\tname\tweight')
        for node in self.nodes:
            print(f'{node.label}\t{node.name}\t{node.weight}')

        print()
        print('@arcs')
        print('\t\tlabel')
        for label, arc in enumerate(self.arcs):
            print(f'{arc[0]}\t{arc[1]}\t{label}')

        print()
        print('@attributes')
        for key, value in self.attributes.items():
            print(f'{key}\t{value}')

        print()
        print('@flows')
        for flow in self.flows:
            modules = ','.join(flow.list_modules())
            print(f'{flow.name}\t{modules}\n')

        print()
        print('@conflicts')
        for node in self.nodes:
            for conflict_node in node.conflicts:
                print(f'{node.label}\t{conflict_node.label}')

    def write_lgf(self, out_file):
        """Write to LGF file

        Parameters:
        out_file (str): Path to write LGF

        """
        with open(out_file, 'w') as outfile:
            nodes_header = '@nodes\nlabel\tname\tweight\n'
            nodes_content = '\n'.join([f'{node.label}\t{node.name}\t{node.weight}'
                                       for node in self.nodes])

            arcs_header = '@arcs\n\t\tlabel\n'
            arcs_content = '\n'.join([f'{arc[0]}\t{arc[1]}\t{label}'
                                      for label, arc in enumerate(self.arcs)])

            attrs_header = '@attributes\n'
            attrs_content = '\n'.join([f'{key}\t{value}'
                                       for key, value in self.attributes.items()])
            flows_header = '@flows\n'
            flows_content = ''
            for flow in self.flows:
                modules = ','.join(flow.list_modules())
                flows_content += f'{flow.name}\t{modules}\n'

            confl_header = '@conflicts\n'
            confl_content = ''
            for node in self.nodes:
                for conflict_node in node.conflicts:
                    confl_content += f'{node.label}\t{conflict_node.label}\n'

            outfile.write(f'{nodes_header}{nodes_content}\n\n'
                          f'{arcs_header}{arcs_content}\n\n'
                          f'{attrs_header}{attrs_content}\n\n'
                          f'{flows_header}{flows_content}\n'
                          f'{confl_header}{confl_content}\n')


def create_config(args):
    """Create an MGW config based on pipeline arguments

    Parameters:
    args (Namespace): Pipeline arguments: user_num, bearer_num,
    bearer0_user, cpu_number_ cpu_capacity

    Returns:
    config (Config): a config object with corresponding objects

    """
    user_num = args.usernum
    bearer_num = args.bearernum
    bearer0_user = args.bearer0user

    module_map = {}
    nodes = []

    common_node_names = [(name, name) for name in
                         ("mac_table", "type_check", "dir_selector",
                          "dl_br_selector", "vxlan_decap", "ul_br_selector",
                          "update_ttl", "L3", "update_mac_dl",
                          "ip_checksum_dl", "update_mac_ul", "ip_checksum_ul")]

    dl_node_names = ["dl_user_bp", "setmd_dl", "vxlan_encap", "ip_encap",
                     "ether_encap"]

    ul_node_names = ["ul_user_bp", "setmd_ul"]

    common_node_names.extend([(f'{names[0]}-c{conflict_id}', names[1])
                              for names in common_node_names
                              for conflict_id in range(args.conflictnum)])

    for name, w_name in common_node_names:
        module = Module(name, len(nodes), MODULE_WEIGHTS[w_name])
        module_map[name] = module
        nodes.append(module)

    for bearer in range(bearer_num):
        selector_names = [(f'{name}_{bearer}', name)
                          for name in ('dl_ue_selector', 'ul_ue_selector')]
        if bearer == 0:
            selector_names.extend([(f'{names[0]}-c{conflict_id}', names[1])
                                   for names in selector_names
                                   for conflict_id in range(args.conflictnum)])
        for name, w_name in selector_names:
            module = Module(name, len(nodes), MODULE_WEIGHTS[w_name])
            module_map[name] = module
            nodes.append(module)
        for user in range(user_num):
            if bearer == 0 and user >= bearer0_user:
                continue
            module_names = [(f'{name}_{user}_{bearer}', name)
                            for name in ul_node_names + dl_node_names]
            if bearer == 0:
                module_names.extend([(f'{names[0]}-c{conflict_id}', names[1])
                                     for names in module_names
                                     for conflict_id in range(args.conflictnum)])
            for name, w_name in module_names:
                module = Module(name, len(nodes), MODULE_WEIGHTS[w_name])
                module_map[name] = module
                nodes.append(module)

    attributes = {
        'cpu_number': args.cpunum,
        'cpu_capacity': args.cpucapacity,
    }

    flows = []
    bearer0_flows = []
    for bearer in range(bearer_num):
        for user in range(user_num):
            if bearer == 0 and user >= bearer0_user:
                continue
            for direction in ('u', 'd'):
                fname = f'{direction}l_{user}_{bearer}'
                mod_names_str = Flow.get_flow_module_names(direction,
                                                           bearer, user)
                modules = [module_map[name]
                           for name in mod_names_str.split(',')]
                flows.append(Flow(fname, modules))
                if bearer == 0:
                    bearer0_flows.append(flows[-1])  # add main flow
                    for conflict_id in range(args.conflictnum):
                        fname = f'{direction}l_{user}_{bearer}-c{conflict_id}'
                        mnames_str = Flow.get_flow_module_names(direction,
                                                                bearer, user,
                                                                str(conflict_id))
                        modules = [module_map[name]
                                   for name in mnames_str.split(',')]
                        conf_flow = Flow(fname, modules)
                        flows.append(conf_flow)
                        bearer0_flows.append(conf_flow)  # add backup flow

    arc_set = set()
    for flow in flows:
        for source, target in zip(flow.modules, flow.modules[1:]):
            source.successors.add(target)
            arc_set.add((source.label, target.label))
    arcs = list(arc_set)

    conflicts = set()
    for idx, cur_flow in enumerate(bearer0_flows):
        for module in cur_flow.modules:
            # collects flow that has same dir (encoded in their name)
            other_flows = [flow for flow in bearer0_flows[idx+1:]
                           if cur_flow.name[:2] == flow.name[:2]]
            for other_flow in other_flows:
                for other_module in other_flow.modules:
                    if module.label != other_module.label:
                        conflicts.add((module.label, other_module.label))
                        module.conflicts.add(other_module)

    return Config(nodes=nodes, arcs=arcs,
                  attributes=attributes, flows=flows,
                  conflicts=conflicts)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--usernum', '-u', type=int, default=2,
                        help='Number of users')
    parser.add_argument('--bearernum', '-b', type=int, default=2,
                        help='Number of bearers')
    parser.add_argument('--bearer0user', '-B', type=int, default=1,
                        help='Number of bearer0 users')
    parser.add_argument('--cpunum', '-c', type=int, default=5,
                        help='Number of CPUs')
    parser.add_argument('--cpucapacity', '-C', type=float, default=25,
                        help='CPU capacity')
    parser.add_argument('--conflictnum', '-l', type=int, default=0,
                        help='Number of CPU failures to cover')
    parser.add_argument('--outfile', '-o', type=str,
                        help='Outfile (LGF)')
    parser.add_argument('--quiet', '-q', action='store_true',
                        help='Do not print LGF to stdout')
    args_parsed = parser.parse_args()

    config = create_config(args_parsed)

    if not args_parsed.quiet:
        config.print_lgf()

    if args_parsed.outfile:
        config.write_lgf(args_parsed.outfile)
