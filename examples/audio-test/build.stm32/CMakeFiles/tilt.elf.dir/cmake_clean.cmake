file(REMOVE_RECURSE
  "tilt.elf.pdb"
  "tilt.elf"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/tilt.elf.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
