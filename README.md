# Probabilistic Fast Downward
<b>Probabilistic Fast Downward</b> (probFD) is an extension of the Fast Downward
planning system (release 22.12) for fully observable probabilistic planning
domains.

For more information about Fast Downward, see the bottom of this README.

## Building the Planner
In contrast to Fast Downward, you need at least C++ 17 to build the planner.
On Windows, the build script generates build files for Ninja instead of NMake,
since nmake does not support build parallelism. If you want to use another build
system, you need to invoke cmake manually (note that the CMakeLists.txt is in 
the root directory and not the src/ directory as in Fast Downward):
```
mkdir -p builds/mycustombuild
cd builds/mycustombuild
cmake ../.. CMAKE_OPTIONS
```

The build instructions are otherwise equivalent to those of Fast Downward. For
more information, see
<https://www.fast-downward.org/ObtainingAndRunningFastDownward>.

## Building the Documentation
To build the documentation, you need to have Doxygen installed. In the `docs/`
directory, execute doxygen to generate the documentation.

## Features
The problem description language assumed by probFD is PPDDL.

### Algorithms
probFD supports a variety of different search algorithms for Generalized
Stochastic Shortest-Path Problems (GSSPs), for example:

- Topological Value Iteration
- i-Dual and i²-Dual
- Labelled real-time dynamic programming (LRTDP)
- Improved LAO* (iLAO*)

To handle zero-reward cycles, probFD implements both end component decomposition
and the FRET framework to eliminate these cycles and to guarantee convergence
of the algorithms against the optimal value function. As a special case in the 
GSSP framework, probFD also supports MaxProb analysis.

### Heuristics
The planner currently implements the following families of heuristics:

- Determinization-based heuristics by usage of a classical heuristic on the
all-outcomes determinization.
- Occupation measure heuristics: The regrouped operator counting heuristic
h<sup>roc</sup> and the projection occupation measure heuristic h<sup>pom</sup>
- Pattern database heuristics for SSPs. Multiple pattern generators are
supported, including classical pattern generators that operator on the
all-outcomes determinization, as well as adaptations of the CEGAR and
hillclimbing pattern generation algorithms for probabilistic planning tasks.
Notably also supports unavoidable dead-ends and zero-reward cycles.

## Contributors
The following list includes all people that actively contributed to
Probabilistic Fast Downward.

- 2019-2022 Marcel Steinmetz
- 2019-2022 Thorsten Klößner
- 2022 Thomas Keller


# Fast Downward

Fast Downward is a domain-independent classical planning system.

Copyright 2003-2022 Fast Downward contributors (see below).

For further information:
- Fast Downward website: <https://www.fast-downward.org>
- Report a bug or file an issue: <https://issues.fast-downward.org>
- Fast Downward mailing list: <https://groups.google.com/forum/#!forum/fast-downward>
- Fast Downward main repository: <https://github.com/aibasel/downward>


## Tested software versions

This version of Fast Downward has been tested with the following software versions:

| OS           | Python | C++ compiler                                                     | CMake |
| ------------ | ------ | ---------------------------------------------------------------- | ----- |
| Ubuntu 22.04 | 3.10   | GCC 11, GCC 12, Clang 14                                         | 3.22  |
| Ubuntu 20.04 | 3.8    | GCC 9, GCC 10, Clang 10, Clang 11                                | 3.16  |
| macOS 12     | 3.10   | AppleClang 14                                                    | 3.24  |
| macOS 11     | 3.8    | AppleClang 13                                                    | 3.24  |
| Windows 10   | 3.8    | Visual Studio Enterprise 2019 (MSVC 19.29) and 2022 (MSVC 19.31) | 3.22  |

We test LP support with CPLEX 12.9, SoPlex 3.1.1 and Osi 0.107.9.
On Ubuntu, we test both CPLEX and SoPlex. On Windows, we currently
only test CPLEX, and on macOS, we do not test LP solvers (yet).


## Contributors

The following list includes all people that actively contributed to
Fast Downward, i.e. all people that appear in some commits in Fast
Downward's history (see below for a history on how Fast Downward
emerged) or people that influenced the development of such commits.
Currently, this list is sorted by the last year the person has been
active, and in case of ties, by the earliest year the person started
contributing, and finally by last name.

- 2003-2022 Malte Helmert
- 2008-2016, 2018-2022 Gabriele Roeger
- 2010-2022 Jendrik Seipp
- 2010-2011, 2013-2022 Silvan Sievers
- 2012-2022 Florian Pommerening
- 2013, 2015-2022 Salomé Eriksson
- 2015, 2021-2022 Thomas Keller
- 2018-2022 Patrick Ferber
- 2021-2022 Clemens Büchner
- 2021-2022 Dominik Drexler
- 2022 Remo Christen
- 2016-2020 Cedric Geissmann
- 2017-2020 Guillem Francès
- 2018-2020 Augusto B. Corrêa
- 2020 Rik de Graaff
- 2015-2019 Manuel Heusner
- 2017 Daniel Killenberger
- 2016 Yusra Alkhazraji
- 2016 Martin Wehrle
- 2014-2015 Patrick von Reth
- 2009-2014 Erez Karpas
- 2014 Robert P. Goldman
- 2010-2012 Andrew Coles
- 2010, 2012 Patrik Haslum
- 2003-2011 Silvia Richter
- 2009-2011 Emil Keyder
- 2010-2011 Moritz Gronbach
- 2010-2011 Manuela Ortlieb
- 2011 Vidal Alcázar Saiz
- 2011 Michael Katz
- 2011 Raz Nissim
- 2010 Moritz Goebelbecker
- 2007-2009 Matthias Westphal
- 2009 Christian Muise


## History

The current version of Fast Downward is the merger of three different
projects:

- the original version of Fast Downward developed by Malte Helmert
  and Silvia Richter
- LAMA, developed by Silvia Richter and Matthias Westphal based on
  the original Fast Downward
- FD-Tech, a modified version of Fast Downward developed by Erez
  Karpas and Michael Katz based on the original code

In addition to these three main sources, the codebase incorporates
code and features from numerous branches of the Fast Downward codebase
developed for various research papers. The main contributors to these
branches are Malte Helmert, Gabi Röger and Silvia Richter.


## License

The following directory is not part of Fast Downward as covered by
this license:

- ./src/search/ext

For the rest, the following license applies:

```
Fast Downward is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

Fast Downward is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
```