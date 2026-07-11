  #pragma once
  #include <string>
  using std::string;

  // Pure pattern-vs-filename matcher. No filesystem access.
  // Handles: * ? [...] [^...] [a-z] [[:class:]]
  // Neither * nor ? nor [...] ever match '/'.
  bool matchGlob(const string &pattern, const string &name);