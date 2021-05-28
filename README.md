# Cotton

Cotton is Game and Application Browser

Currently I'm try to implement DOM Rendering Engine.

<image src="/cotton_image.png">

# install
  
  packman -S mingw-w64-i686-libpng

# cotton

Cotton Game and Application Browser

  # Compile Cotton Web Brwoer, DEBUG
  spvmcc --ccflags '-DUNICODE' --lddlflags '-lgdi32 -lstdc++' -I lib -o Cotton Cotton::Browser::Win

# memo

  # Compile Cotton Web Brwoer with libpng
  spvmcc --lddlflags '-lgdi32 -Wl,-subsystem,windows' -I lib -o Cotton Cotton::Win::Browser
  
  # Compile Cotton Web Brwoer with libpng
  spvmcc --lddlflags '-Wl,-Bstatic,-lpng,-lz,-lssp,-Bdynamic -lgdi32 -Wl,-subsystem,windows' -I lib -o Cotton Cotton::Win::Browser


