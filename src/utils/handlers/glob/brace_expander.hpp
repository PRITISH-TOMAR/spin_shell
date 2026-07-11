 #pragma once                                                                                                                                                                                                  
  #include <string>                                                                                                                                                                                             
  #include <vector>                                                                                                                                                                                             
  using std::string;                                                                                                                                                                                            
  using std::vector;

  // Expands a single pattern string into all brace-expanded alternatives.

  // expandBraces("file{.txt,.md}")     → ["file.txt", "file.md"]
  // expandBraces("{a,{b,c}}")          → ["a", "b", "c"]
  // expandBraces("{1..3}")             → ["1", "2", "3"]
  // expandBraces("no_braces")          → ["no_braces"]
  vector<string> expandBraces(const string &pattern);
