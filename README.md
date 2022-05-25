# Cotton

Cotton is a example of SPVM application on Windows.

# Compile cotton

  # Compile Cotton - development mode
  spvmcc -o cotton -c cotton.config -I lib/SPVM Cotton::Browser::Win

  # Compile Cotton - producetion mode
  spvmcc -o cotton -c cotton.production.config -I lib/SPVM Cotton::Browser::Win

