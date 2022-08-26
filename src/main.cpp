#include <iostream>
#include <vector>

#include "core/subnet.h"
#include "core/transaction.h"

std::unique_ptr<Subnet> subnet;

// Let that good boi run
int main() {
  std::signal(SIGINT, SIG_IGN);
  //std::signal(SIGTERM, SIG_IGN);
  subnet = std::make_unique<Subnet>();
  subnet->start();
  return 0;
}

