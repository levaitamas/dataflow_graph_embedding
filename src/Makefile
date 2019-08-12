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

LEMON_DIR=/opt/lemon
GUROBI_DIR=/opt/gurobi/linux64
GUROBI_LIB_VER=81

WARNINGS=-Wall
#OPT_LVL=-Og -g
OPT_LVL=-O3
MARCH=native

INCLUDES=-I$(LEMON_DIR) -I$(GUROBI_DIR)/include
CXX=g++
CXXFLAGS= $(WARNINGS) $(OPT_LVL) -march=$(MARCH) $(INCLUDES)

LIB_DIRS=-L$(LEMON_DIR)/lemon -L$(GUROBI_DIR)/lib
LIBS=$(LIB_DIRS) -lemon -lgurobi$(GUROBI_LIB_VER)


PROG=dfg-embed
OBJS=dfg-embed.o
HEADS=cpu.h flow.h module.h utils.h
HEADS+=embed-common.h embed-random.h embed-roundrobin.h embed-bestfitdec.h
HEADS+=embed-greedy.h embed-ilp.h

$(PROG): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIBS)

$(OBJS): $(HEADS)

.PHONY: clean purge

clean:
	$(RM) $(OBJS)

purge:
	$(RM) $(OBJS)
	$(RM) $(PROG)
