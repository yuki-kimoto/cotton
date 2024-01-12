# Cotton

Cotton is a portable application example using SPVM::Eg.

# Required Modules Installation

  cpanm --installdeps .

# Executable File Generating

  # Compile Engine - development mode
  spvmcc -o cotton -c cotton.config -I lib/SPVM Cotton

  # Compile Engine - producetion mode
  spvmcc -o cotton -c cotton.production.config -I lib/SPVM Cotton

# Run Application

  ./cotton

