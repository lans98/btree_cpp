#include <BTree.hpp>

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <fmt/ostream.h>

int main() {
  btree::Page<int, 8> page;
  page.add(3);
  page.add(2);
  page.add(9);
  page.add(1);
  page.add(5);

  fmt::print("page: {}\n", page);

  fmt::print("Searching: 5 {{ index: {:<4}, found: {:<5} }}\n", page.find(5).index, page.find(5).found);
  fmt::print("Searching: 0 {{ index: {:<4}, found: {:<5} }}\n", page.find(0).index, page.find(0).found);
  fmt::print("Searching: 8 {{ index: {:<4}, found: {:<5} }}\n", page.find(8).index, page.find(8).found);
  fmt::print("Searching: 9 {{ index: {:<4}, found: {:<5} }}\n", page.find(9).index, page.find(9).found);
  fmt::print("Searching: 10 {{ index: {:<4}, found: {:<5} }}\n", page.find(10).index, page.find(10).found);

  {
    btree::Page<int, 8> l;
    btree::Page<int, 8> r;
    auto mid = page.split(l, r, false);
    fmt::print("Without consuming middle key {{ {} }} :\n", mid);
    fmt::print("l: {}\n",l);
    fmt::print("r: {}\n",r);
  }

  {
    btree::Page<int, 8> l;
    btree::Page<int, 8> r;
    auto mid = page.split(l, r, true);
    fmt::print("Consuming middle key {{ {} }} :\n", mid);
    fmt::print("l: {}\n",l);
    fmt::print("r: {}\n",r);
  }
}
