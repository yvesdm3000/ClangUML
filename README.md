# ClangUML
Generate UML diagrams using clang parser.
The output format is in PlantUML and can be edited after generation to tune the graph to your needs. It simply helps if you don't have to type all the different classes for a PlantUML file.

# Example usage
Add all the files where you want to use the classes from in 1 command:

> ClangUML source1.h source2.h source2.cpp > mydiagram.pu

Author: Yves De Muyter (yves@alfavisio.be)

## Requirements
- Linux (other platforms haven't been tested yet)
- Clang 3.6 or later
- CMake 2.8 or later
- gcc 4.8 (no other had been tested yet)

## TODO
- Aggregation
- STL storage types
- Custom storage types that should be treated as list or pointer
- Command line arguments to include or filter out classes
- Command line arguments to enable/disable various options like arrow direction, namespaces
- Add support for "compile_commands.json"
- Find some nice way to integrate with PlantUML. I currently use its watchfolder feature.

