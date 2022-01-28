# Cotton

Cotton is a example of SPVM application on Windows.

# Compile cotton

  # Compile Cotton Web Brwoer, DEBUG
  spvmcc -o cotton -c cotton.config -I lib/SPVM Cotton::Browser::Win

# Old

  # install
  packman -S mingw-w64-i686-libpng

  # Compile Cotton Web Brwoer with libpng
  spvmcc --lddlflags '-lgdi32 -Wl,-subsystem,windows' -I lib -o Cotton Cotton::Win::Browser
  
  # Compile Cotton Web Brwoer with libpng
  spvmcc --lddlflags '-Wl,-Bstatic,-lpng,-lz,-lssp,-Bdynamic -lgdi32 -Wl,-subsystem,windows' -I lib -o Cotton Cotton::Win::Browser
