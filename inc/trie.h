#ifndef TRIE_H
# define TRIE_H

# include <optional>
# include <unordered_map>
# include <string>
# include <utility>
# include <vector>
# include <stack>
# include <memory>
# include <type_traits>

# include "Index.h"

namespace generic_trie {
    template<typename V, typename... Keys>
    class KTrie {
        public:
            using Key = std::tuple<Keys...>;

            using key_type = std::tuple<Keys...>;
            using mapped_type = V;
            using value_type = std::pair<Key, V>;
            using size_type = std::size_t;
            using reference = value_type&;
            using const_reference = const value_type&;

            template<typename T>
            struct Wrapper {
                using Type = std::decay_t<T>;

                Type value;
            };

            template<typename T>
            using RefWrapper = Wrapper<const T&>;

            template<typename T>
            static Wrapper<T> makeWrapper(const T& t) {
                return Wrapper<RemoveCVRef_t<T>>{t};
            }

            KTrie():
                root(new Node{})
            { }

            struct Node {
                template<typename T>
                using ChildrenType = std::unordered_map<T, std::shared_ptr<Node>>;

                // There can be a set of children for each remaining part of the key
                //   TODO: For optimization, can we skip the first part of the key types, as we will not see that again?
                //   Example (ignore V):
                //   Node<int, string, FILE, float> {
                //      int -> Node<string, FILE, float> {
                //          string -> Node<FILE, float> {
                //              FILE -> Node<float> {
                //                  float -> V
                //              },
                //              float -> Node<FILE> {
                //                  FILE -> V
                //              }
                //          },
                //          FILE -> Node<string, float> {
                //              string -> Node<float> {
                //                  float -> V
                //              },
                //              float -> Node<string> {
                //                  string -> V
                //              }
                //          },
                //          float -> Node<string, FILE> {
                //              string -> Node<FILE> {
                //                  float -> V
                //              },
                //              FILE -> Node<string> {
                //                  string -> V
                //              }
                //          },
                //      },
                //      ...
                //   }
                std::tuple< ChildrenType<Keys>... > children;

                using Data = std::pair<Key, V>;

                // Value is optional as one is only held if this is a leaf
                //   TODO: With template magic this could probably be skipped
                //     if Keys... is not empty (as if it's not empty then it's
                //       automatically not a leaf
                std::optional<Data> data;
            };

            // We do not accept partial keys for insertion
            bool insert(const Key& key, const V& value) {
                std::shared_ptr<Node> node = getNodeForPartialKey<Keys...>(key, true);

                // Only insert the value if it does not already exist
                //   Return true if we inserted the value, false otherwise
                if(node->data == std::nullopt) {
                    node->data = { key, value};
                    return true;
                } else {
                    return false;
                }
            }

            template<typename NodeType = Node>
            class IteratorImpl {
                public:
                    IteratorImpl(std::shared_ptr<NodeType> node):
                        m_nodes()
                    {
                        if(node != nullptr) {
                            m_nodes.push(node);

                            // If the top node does not have a value, advance
                            //   until one is found (For the case of partial
                            //   keys, where a node is returned that does not
                            //   hold a value because we are given one that is
                            //   part of the way through a key)
                            while(!m_nodes.empty() &&
                                  !m_nodes.top()->data.has_value())
                            {
                                advance();
                            }
                        }
                    }

                    // End iterator, cannot be moved forward any more
                    IteratorImpl<NodeType>(): m_nodes() { }

                    IteratorImpl<NodeType> operator++() {
                        // Do not attempt to increment if we are at the end
                        if(m_nodes.empty()) {
                            return *this;
                        }

                        // Advance by 1
                        advance();

                        // If the top element does not have a value, then
                        //   keep advancing until we have one that does
                        while(!m_nodes.empty() &&
                               m_nodes.top()->data == std::nullopt)
                        {
                            advance();
                        }

                        return *this;
                    }

                    bool operator!=(const IteratorImpl<NodeType>& it) const noexcept
                    {
                        return !(*this == it);
                    }

                    bool operator==(const IteratorImpl<NodeType>& it) const noexcept
                    {
                        // TODO: Convert this into a boolean expression and
                        //   remove branching
                        if(isEnd() && it.isEnd()) {
                            return true;
                        } else if(isEnd() || it.isEnd()) {
                            return false;
                        }

                        // Iterator is the same as another if they are currently
                        //  looking at the same node.
                        return m_nodes.top().get() == it.m_nodes.top().get();
                    }

                    bool isEnd() const noexcept {
                        return m_nodes.empty();
                    }

                protected:
                    using NodeStack = std::stack<std::shared_ptr<NodeType>>;

                    // Will advance the nodes by 1
                    //   Pops the top node off the stack, adds all children to
                    //   the stack
                    // Assumes m_nodes is not empty
                    void advance() noexcept {
                        auto node = m_nodes.top();
                        m_nodes.pop();

                        // Iterate over every possible child in the tuple
                        //   and add its children map to the stack
                        forEach(node->children, [this](auto& child_map) {
                            for(auto&& [v, child] : child_map) {
                                // 'v' is stored in the child node, and can be
                                //   ignored here
                                m_nodes.push(child);
                            }
                        });
                    }

                    NodeStack& getNodeStack() noexcept {
                        return m_nodes;
                    }

                    const NodeStack& getNodeStack() const noexcept {
                        return m_nodes;
                    }

                private:
                    NodeStack m_nodes;
            };

            struct Iterator: public IteratorImpl<Node> {
                std::pair<const Key&, V&> operator*() const noexcept {
                    return {
                        this->getNodeStack().top()->data.value().first,
                        this->getNodeStack().top()->data.value().second
                    };
                }
            };

            struct ConstIterator: public IteratorImpl<const Node> {
                std::pair<const Key&, const V&> operator*() const noexcept {
                    return {
                        this->getNodeStack().top()->data.value().first,
                        this->getNodeStack().top()->data.value().second
                    };
                }
            };

            ConstIterator begin() const noexcept {
                return ConstIterator{root};
            }

            ConstIterator end() const noexcept {
                return ConstIterator{};
            }

            Iterator begin() noexcept {
                return Iterator{root};
            }

            Iterator end() noexcept {
                return Iterator{};
            }

            // TODO: This overload is broken. Compiler gets into an endless loop if forced.
            //   FIX THIS
            template<typename... PartialKey>
            Iterator find(const std::tuple<PartialKey...>& key) noexcept {
                std::shared_ptr<Node> node = getNodeForPartialKey(key);

                return Iterator{node};
            }

            template<typename... PartialKey>
            Iterator find(PartialKey&&... key) noexcept {
                std::tuple<std::decay_t<PartialKey>...> tkey = std::make_tuple(key...);

                std::shared_ptr<Node> node = getNodeForPartialKey<std::decay_t<PartialKey>...>(tkey);

                return Iterator{node};
            }

            ///////

            template<typename... PartialKey>
            ConstIterator find(PartialKey&&... key) const noexcept {
                std::tuple<std::decay_t<PartialKey>...> tkey = std::make_tuple(key...);

                std::shared_ptr<const Node> node = getNodeForPartialKey<std::decay_t<PartialKey>...>(tkey);

                return ConstIterator{node};
            }

            template<typename... PartialKey>
            void erase(std::tuple<PartialKey...> key) {
                std::shared_ptr<Node> node = getNodeForPartialKey(key);

                // TODO: We should probably actually return the erased values as
                //   well

                // For each part of the key, erase all children
                ([&] {
                    std::get<Keys>(node->children).clear();
                }(), ...);
            }

        protected:
            template<typename... PartialKey>
            std::shared_ptr<const Node>
                getNodeForPartialKey(const std::tuple<PartialKey...>& key) const noexcept
            {
                return getNodeForPartialKey(key, std::make_index_sequence<sizeof...(PartialKey)>{});
            }

            template<typename T, std::size_t... Indices>
            std::shared_ptr<const Node> getNodeForPartialKey(const T& key,
                                                             std::index_sequence<Indices...>) const noexcept
            {
                return getNodeForPartialKeyImpl(
                    std::make_tuple(makeWrapper( std::get<Indices>(key) )...));
            }

            template<typename... PartialKey>
            std::shared_ptr<const Node>
                getNodeForPartialKeyImpl(const std::tuple<Wrapper<PartialKey>...>& key) const noexcept
            {
                std::shared_ptr<const Node> node = root;

                std::apply([&](auto&&... args) {
                    node = getNodeForPartialKeyImpl<PartialKey...>(args...);
                }, key);

                return node;
            }

            template<typename... PartialKey>
            std::shared_ptr<const Node> getNodeForPartialKeyImpl(const Wrapper<PartialKey>&... key) const noexcept
            {
                std::shared_ptr<const Node> node = root;

                // If this is the first time ever trying to get a node, make
                //   sure that we either return early or initialize root
                if(node == nullptr) {
                    return nullptr;
                }

                // for every part of the key
                // Note: we can skip the std::apply if we don't accept the key
                //   as a tuple
                // We may want to only accept by tuple though, to make the
                //   API simpler
                // Return a boolean to allow us to use short-circuiting to skip
                //   any further calls to this function if we need to stop early
                ([&] {
                    using ArgType = decltype(key);
                    using RawType = typename std::decay_t<ArgType>::Type;

                    // Stop early if node is null
                    if(node == nullptr) { return false; }

                    // Find the correct 'children' for this type
                    // Children are stored in a tuple of hashmap<T, shared_ptr<Node> >
                    const auto& children = getChildrenTypeFromTuple<RawType>(node->children);

                    // If the node for this key value doesn't exist, then set
                    //  'node' to null to denote a lookup failure
                    if(children.count(key.value) == 0) {
                        return false;
                    } else {
                        node = children.at(key.value);
                    }

                    return true;
                }() && ...);

                return node;
            }

            ////////

            template<typename... PartialKey>
            std::shared_ptr<Node> getNodeForPartialKey(const std::tuple<PartialKey...>& key,
                                                        bool createIfKeyDoesNotExist = false) noexcept
            {
                return getNodeForPartialKey(key, std::make_index_sequence<sizeof...(PartialKey)>{}, createIfKeyDoesNotExist);
            }

            template<typename T, std::size_t... Indices>
            std::shared_ptr<Node> getNodeForPartialKey(const T& key,
                                                       std::index_sequence<Indices...>,
                                                       bool createIfKeyDoesNotExist = false) noexcept
            {
                return getNodeForPartialKeyImpl(
                    std::make_tuple(makeWrapper( std::get<Indices>(key) )...),
                    createIfKeyDoesNotExist);
            }

            template<typename... PartialKey>
            std::shared_ptr<Node> getNodeForPartialKeyImpl(
                    const std::tuple<Wrapper<PartialKey>...>& key,
                    bool createIfKeyDoesNotExist = false)
            {
                std::shared_ptr<Node> node = root;

                std::apply([&](auto&&... args) {
                    node = getNodeForPartialKeyImpl<PartialKey...>(args..., createIfKeyDoesNotExist);
                }, key);

                return node;
            }

            template<typename... PartialKey>
            std::shared_ptr<Node> getNodeForPartialKeyImpl(const Wrapper<PartialKey>&... key,
                                                           bool createIfKeyDoesNotExist = false)
            {
                std::shared_ptr<Node> node = root;

                // If this is the first time ever trying to get a node, make
                //   sure that we either return early or initialize root
                if(node == nullptr) {
                    if(createIfKeyDoesNotExist) {
                        root = node = std::make_shared<Node>();
                    } else {
                        return nullptr;
                    }
                }

                // for every part of the key
                // Note: we can skip the std::apply if we don't accept the key
                //   as a tuple
                // We may want to only accept by tuple though, to make the
                //   API simpler
                // Return a boolean to allow us to use short-circuiting to skip
                //   any further calls to this function if we need to stop early
                ([&] {
                    using ArgType = decltype(key);
                    using RawType = typename std::decay_t<ArgType>::Type;

                    // Stop early if node is null
                    if(node == nullptr) { return false; }

                    // Find the correct 'children' for this type
                    // Children are stored in a tuple of hashmap<T, shared_ptr<Node> >
                    auto& children = getChildrenTypeFromTuple<RawType>(node->children);

                    // If the node for this key value doesn't exist, then create
                    //  it if we are asked to, otherwise, set 'node' to null to
                    //  denote a lookup failure
                    if(children.count(key.value) == 0) {
                        if(createIfKeyDoesNotExist) {
                            node = children[key.value] = std::make_shared<Node>();
                        } else {
                            node = nullptr;
                        }
                    } else {
                        node = children[key.value];
                    }

                    return true;
                }() && ...);

                return node;
            }

            template<typename T, typename... Types>
            auto getChildrenTypeFromTuple(std::tuple<Types...>& t) noexcept
                -> typename Node::template ChildrenType<T>&
            {
                constexpr std::size_t Index = getIndexOfType<typename Node::template ChildrenType<T>, Types...>();
                return std::get<Index>(t);
            }

            template<typename T, typename... Types>
            auto getChildrenTypeFromTuple(const std::tuple<Types...>& t) const noexcept
                -> const typename Node::template ChildrenType<T>&
            {
                constexpr std::size_t Index = getIndexOfType<typename Node::template ChildrenType<T>, Types...>();
                return std::get<Index>(t);
            }

        private:
            std::shared_ptr<Node> root;
    };
}

#endif

