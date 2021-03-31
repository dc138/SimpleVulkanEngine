#include <svke/svke.hpp>
using namespace svke;

int main() {
  Application app {512, 512, "First Application"};
  app.Run();

  return 0;
}