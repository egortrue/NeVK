#include <iostream>
#include "app.h"

int main() {
  try {
    Application app;
    app.run();
  } catch (const std::exception& error) {
    std::cerr << error.what() << std::endl;
  }
  return 0;
}