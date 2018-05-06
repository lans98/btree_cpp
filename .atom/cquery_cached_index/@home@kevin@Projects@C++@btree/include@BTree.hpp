#pragma once

#include <cassert>
#include <array>
#include <vector>
#include <iostream>
#include <algorithm>
#include <exception>
#include <utility>
#include <cmath>

#define print_iterable(iterable) \
  for (auto& e: iterable) \
    std::cout << e << ' '; \

#define deref(node) (*node)

namespace btree {

  using namespace std;

  class error_exception : public exception {
  private:
    string msg;
  public:
    error_exception() = delete;
    error_exception(string msg): msg(std::move(msg)) {}

    const char* what() const noexcept { return msg.c_str(); }
  };

  void expect(bool expr, string msg) {
    if (!expr) throw error_exception(msg);
  }

  struct SearchResult {
    size_t index;
    bool   found;
  };

  template <class Key, size_t Size>
  class Page {
  private:
    array<Key, Size> m_buf;
    size_t           m_used;

  public:
    Page(): m_buf(), m_used(0) {}
    explicit Page(const Page& other): m_buf(other.m_buf) {}

    template <size_t OSize>
    explicit Page(const Page<Key, OSize>& other): m_buf() {
      expect(OSize >= Size, "Can't copy to a tinier page");
      for (auto i = 0; i < Size; ++i)
        m_buf[i] = other.m_buf[i];
    }

    size_t add(const Key& key) {
      expect(m_used < Size, "Buf full");

      size_t index = 0;
      try {
        index = this->find(key).index;
      } catch (const std::exception&) { }
      expect(m_buf[index] != key, "No duplicates are allowed!");

      if (m_used != 0) {
        for (size_t j = m_used - 1; j >= index; --j) {
          m_buf[j + 1] = m_buf[j];
          if (j == 0) break; // as we are using unsigned longs
        }
      }

      m_buf[index] = key;
      m_used += 1;
      return index;
    }

    size_t rmv(const Key& key) {
      expect(m_used != 0, "Buf empty");

      auto search_result = this->find(key);
      expect(search_result.found, "Key isn't here");

      for (auto j = search_result.index; j != m_used; ++j)
        m_buf[j] = m_buf[j + 1];

      m_used -= 1;
      return search_result.index;
    }

    SearchResult find(const Key& key) {
      expect(m_used != 0, "Buf empty");

      // Aliases
      auto beg = m_buf.begin();
      auto end = beg + m_used;

      // Searching
      auto iterator = lower_bound(beg, end, key); // this is binary search
      bool found    = !(iterator == end) && !(key < *iterator);
      size_t index  = *iterator < key ? m_used : iterator - beg;

      return SearchResult { .index = index, .found = found };
    }

    const Key& operator[](size_t idx) {
      expect(idx < m_used, "Out of bound");
      return m_buf[idx];
    }

    size_t& used() { return m_used; }
    void clear() { m_used = 0; }
    bool is_full() { return m_used == Size; }

    Key split(Page& left, Page& right, bool consume_mid) {
      auto middle = Size / 2UL;
      for (auto i = 0; i < middle; ++i)
        left.m_buf[i] = m_buf[i];

      for (auto i = consume_mid ? middle + 1 : middle; i < Size; ++i)
        right.m_buf[i] = m_buf[i];

      left.m_used  = middle;
      right.m_used = consume_mid? Size - middle - 1 : Size - middle;

      return m_buf[middle];
    }

    friend ostream& operator<<(ostream& out, const Page& page) {
      for (auto& e : page.m_buf)
        out << e << ' ';
    }
  };


  template <class Key, size_t Order>
  class BPlusTree {
  private:

    class Node {
    private:
      Node*                m_parent;
      Page<Key, Order - 1> m_page;
      array<Node*, Order>  m_childs;

    public:
      Node(): m_parent(nullptr), m_page(), m_childs() {
        for (auto& n : m_childs)
          n = nullptr;
      }

      Node*&               parent() { return m_parent; }
      Page<Key, Order-1>&  page() { return m_page; }
      array<Node*, Order>& childs() { return m_childs; }
      Node*&               next() { return m_childs[Order - 1]; }

      bool is_leaf() {
        for (auto i = 0; i < Order - 1; ++i)
          if (m_childs[i]) return false;
        return true;
      }
    };

    Node* m_root;

  public:
    BPlusTree(): m_root(nullptr) {}
    BPlusTree(const BPlusTree& rhs) = delete;

    bool add(Key key) {
      Node** node;
      Node*  parent;
      size_t index;
      if (find(key, node, parent, index))
        return false; // no duplicates allowed

      if (!*node) {
        (*node) = new Node();
        (*node)->parent() = parent;
      }

      // If there is space in the page, add the key
      if (!(*node)->page().is_full())
        return (*node)->add(key);

      // If the page is full, do some work to balance it
      balance_adding(key, node);
      return true;
    }

    bool rmv(const Key& key) {
      // TODO
    }

    const Key& find(const Key& key) {
      Node** node;
      Node*  parent;
      size_t index;
      expect(find(key, node, parent, index), "Key isn't here");
      return (*node)->page()[index];
    }

  private:

    void balance_adding(const Key& key, Node** node) {
      Page<Key, Order> temp_buf((*node)->page());
      temp_buf.add(key);

      auto left  = new Node();
      auto right = new Node();
      left->parent()  = *node;
      right->parent() = *node;

      auto mid = temp_buf.split(left->page(), right->page(), (*node)->is_leaf());
    }

    bool find(const Key& key, Node**& node, Node*& parent, size_t& index) {
      node   = &m_root;
      parent = nullptr;
      while (*node) {
        auto search_result = (*node)->page().find(key);
        if (search_result.found && (*node)->is_leaf()) {
          // We have found the key, we are pointing to the node where it
          // resides, we can end our search
          index = search_result.index; // tell me where did you find it
          return true; // okay now I handle this
        }

        if (search_result.found && !(*node)->is_leaf()) {
          // this is easy to handle, we have found the key but we aren't
          // in a node leaf, so we go to the child next to this key
          parent = *node;
          node   = &(*node)->childs()[search_result.index + 1];
          continue; // okay now do your job, continue searching
        }

        if (!search_result.found && (*node)->is_leaf()) {
          // we are already on a leaf and we didn't find the key, good news
          // we are pointing to the node where it should go and the index
          // where the key should go too, return them
          index = search_result.index; // tell me where it should go
          return false; // we didn't find the key
        }

        if (!search_result.found && !(*node)->is_leaf()) {
          // Ok, we are in a internal node and we didn't find the key, no worries
          // we can still search more deep, but where should we go now ?
          // this is kinda tricky, for more info look at Page::find()
          parent = *node;
          node   = &(*node)->childs()[search_result.index];
          continue; // continue searching, please
        }
      }
      return false;  // this is likely going to happen when the root doesn't exist yet
    }
  };

}
