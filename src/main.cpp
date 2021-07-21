#include <svke/svke.hpp>

int main() {
  svke::Application app {512, 512, "First Application"};
  app.Run();

  return 0;
}
