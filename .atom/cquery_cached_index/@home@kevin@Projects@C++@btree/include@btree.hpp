#pragma once

#include <cassert>
#include <array>
#include <vector>


namespace btree {

  using namespace std;

  class error_exception : exception {
  private:
    string msg;
  public:
    error_exception() = delete;
    error_exception(string msg): msg(std::move(msg)) {}

    const char* what() noexcept { return msg.c_str(); }
  };

  void assert_or_throw(bool expr, string msg) {
    if (!expr) throw error_exception(msg);
  }

  template <class Key, size_t Size>
  class Record {
  private:
    array<Key, Size> m_buf;
    size_t           m_used;

  public:
    Record(): m_buf(), m_used(0) {}
    explicit Record(const Record& record): m_buf(record.m_buf) {}

    bool add(Key key) {
      size_t idx;
      for (idx = 0; idx < m_used; ++idx)
        if (m_buf[idx] >= key) break;

      assert_or_throw(m_buf[idx] != key, "No duplicates are allowed!");
      for (size_t jdx = m_used - 1; jdx <= idx; --jdx)
        m_buf[jdx + 1] = std::move(m_buf[jdx]);

      m_buf[idx] = std::move(key);
    }

    bool rmv(const Key& key) {

    }

    const Key& find(const Key& key) {

    }
  };


  template <class Key, size_t Order>
  class BPlusTree {
  private:

    struct Node {
      Record<Key, Order - 1> m_record;
      array<Node, Order>     m_childs;
    };

    Node m_root;
  public:
    BPlusTree(): m_root(nullptr) {}
    BPlusTree(const BPlusTree& rhs) = delete;

    bool add(Key key) {
    }

    bool rmv(const Key& key) {
    }

    const Key& find(const Key& key) {
    }

  private:

  };

}
