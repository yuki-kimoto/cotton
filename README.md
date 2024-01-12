# Engine

Engine is a example of SPVM application on Windows.

# Required Modules Installation

  cpanm --installdeps .

# Executable File Generating

  # Compile Engine - development mode
  spvmcc -o cotton -c cotton.config -I lib/SPVM Engine

  # Compile Engine - producetion mode
  spvmcc -o cotton -c cotton.production.config -I lib/SPVM Engine

# Run Application

  ./cotton

