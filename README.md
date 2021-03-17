# install
  
  packman -S mingw-w64-i686-libpng

# cotton
Cotton Game and Application Browser
  
  # Compile Cotton Web Brwoer
  spvmcc --lddlflags '-Wl,-Bstatic,-lpng,-lz,-lssp,-Bdynamic -lgdi32 -Wl,-subsystem,windows' -I lib -o Cotton Cotton
