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
# include <tuple>

# ifndef _MKM_DEBUG_OUTPUT
#  include <ostream>
# endif

namespace mkm {
    namespace detail {
# ifndef _MKM_DEBUG_OUTPUT
        class NullBuffer: public std::streambuf {
            public:
                int overflow(int c) { return c; }
        };
        NullBuffer nul_buffer;
        std::ostream nul(&nul_buffer);
#  define _MKM_DEFAULT_DEBUG
#  define _MKM_DEBUG_OUTPUT ::mkm::detail::nul
# endif

        /**
         * @brief Remove const, volatile, and reference qualifiers
         *
         * @tparam T The type to remove the qualifiers from
         */
        template<typename T = void>
        struct RemoveCVRef {
            typedef std::remove_cv_t<std::remove_reference_t<T>> type;
        };

        /**
         * @brief Helper type for RemoveCVRef
         */
        template<typename T = void>
        using RemoveCVRef_t = typename RemoveCVRef<T>::type;

        template<typename T, typename... Ts>
        struct IndexImpl;

        template<typename T, typename... Ts>
        struct IndexImpl<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

        template<typename T, typename U, typename... Ts>
        struct IndexImpl<T, U, Ts...> : std::integral_constant<std::size_t, 1 + IndexImpl<T, Ts...>::value> {};

        /**
         * @brief Gets the index of T in Ts...
         *
         * @tparam T The type to get the index for
         * @tparam Ts The types to search through
         */
        template<typename T, typename... Ts>
        struct Index: IndexImpl<RemoveCVRef_t<T>, Ts...> {};

        /**
         * @brief Helper type for Index
         *
         * @tparam T The type to get the index for
         * @tparam Ts The types to search through
         */
        template<typename T, typename... Ts>
        constexpr std::size_t Index_v = Index<T, Ts...>::value;

        /**
         * @brief Helper function-like interface for Index
         *
         * @tparam T The type to get the index for
         * @tparam Ts The types to search through
         *
         * @return The index of T in Ts...
         */
        template<typename T, typename... Ts>
        constexpr std::size_t getIndexOfType() {
            return Index<T, Ts...>::value;
        }

        /**
         * @brief Runs a function F on every value in a tuple
         *
         * @tparam Is The indices for the tuple
         * @tparam Tuple The tuple type
         * @tparam F The function type
         *
         * @param tuple The tuple to "iterate over"
         * @param f The function to run
         */
        template<size_t... Is, typename Tuple, typename F>
        void forEach(std::index_sequence<Is...>, Tuple&& tuple, F&& f) {
            int unused[] = { 0, ( (void)f(std::get<Is>(std::forward<Tuple>(tuple))), 0 )... };
            (void)unused;
        }

        /**
         * @brief Runs a function F on every value in a tuple
         *
         * @tparam F The function type
         * @tparam Args The types that make up the tuple
         *
         * @param tuple The tuple to "iterate over"
         * @param f The function to run
         */
        template<typename F, typename... Args>
        void forEach(std::tuple<Args...>& tuple, F&& f) {
            forEach(std::make_index_sequence<sizeof...(Args)>{}, tuple, f);
        }

        /**
         * @brief Runs a function F on every value in a tuple
         *
         * @tparam F The function type
         * @tparam Args The types that make up the tuple
         *
         * @param tuple The tuple to "iterate over"
         * @param f The function to run
         */
        template<typename F, typename... Args>
        void forEach(const std::tuple<Args...>& tuple, F&& f) {
            forEach(std::make_index_sequence<sizeof...(Args)>{}, tuple, f);
        }
    }

    template<typename V, typename... Keys>
    class MultiKeyMap {
        public:
            using key_type = std::tuple<Keys...>;
            using mapped_type = V;
            using value_type = std::pair<key_type, V>;
            using size_type = std::size_t;
            using reference = value_type&;
            using const_reference = const value_type&;

        protected:
            struct Node;

            using NodePtr = std::shared_ptr<Node>;
            using ConstNodePtr = std::shared_ptr<const Node>;

            /**
             * @brief Defines a single node in a Trie.
             * @details Each node will hold remaining parts of a key as children
             *          as well as an optional value (which should only be
             *          defined for leaf nodes, as values can only be inserted
             *          for full keys, not partial keys).
             *
             * @par Example structure for a trie with a key of
             *      <tt> {int, string, FILE, float} </tt> (let the stored value just be 'V'):
             * @code{.unparsed}
             * Node<int, string, FILE, float> {
             *    int -> Node<string, FILE, float> {
             *        string -> Node<FILE, float> {
             *            FILE -> Node<float> {
             *                float -> V
             *            },
             *            float -> Node<FILE> {
             *                FILE -> V
             *            }
             *        },
             *        FILE -> Node<string, float> {
             *            string -> Node<float> {
             *                float -> V
             *            },
             *            float -> Node<string> {
             *                string -> V
             *            }
             *        },
             *        float -> Node<string, FILE> {
             *            string -> Node<FILE> {
             *                float -> V
             *            },
             *            FILE -> Node<string> {
             *                string -> V
             *            }
             *        },
             *    },
             *    ...
             * }
             * @endcode
             */
            struct Node {
                template<typename T>
                using ChildrenType = std::unordered_map<T, NodePtr>;

                // There can be a set of children for each remaining part of the key
                std::tuple< ChildrenType<Keys>... > children;

                using Value = std::pair<key_type, mapped_type>;

                // Value is optional as one is only held if this is a leaf
                std::optional<Value> data;

                //! Parent node. Will be null for the root
                NodePtr parent;
            };

            //! Helper type alias for access a Node's children type
            template<typename T>
            using NodeChildren = typename Node::template ChildrenType<T>;

            /**
             * @brief A light-weight wrapper around a value of a given type
             *
             * @tparam T The type to wrap
             */
            template<typename T>
            struct Wrapper {
                //! The raw type being wrapped.
                using Type = std::decay_t<T>;

                //! The value being wrapped
                Type value;
            };

            /**
             * @brief Helper function to create a Wrapper object
             *
             * @tparam T The type to wrap
             * @param t The value to wrap
             *
             * @return A new Wrapper<T> object around 't'
             */
            template<typename T>
            static Wrapper<T> makeWrapper(const T& t) {
                return Wrapper<detail::RemoveCVRef_t<T>>{t};
            }

        public:
            /**
             * @brief Constructs a new empty MultiKeyMap
             */
            MultiKeyMap():
                root(new Node{}),
                m_size(0)
            { }

            template<typename InputIt>
            MultiKeyMap(InputIt first, InputIt last):
                m_size(0)
            {
                for(; first != last; ++first) {
                    insert(first->first, first->second);
                }
            }

            MultiKeyMap(std::initializer_list<value_type> init):
                MultiKeyMap(init.begin(), init.end())
            { }

            MultiKeyMap(const MultiKeyMap& rhs):
                root(new Node{}),
                m_size(0)
            {
                _MKM_DEBUG_OUTPUT << "copy ctor" << std::endl;

                for(auto it = rhs.cbegin(); it != rhs.cend(); ++it) {
                    insert(it->first, it->second);
                }
            }

            MultiKeyMap(MultiKeyMap&& rhs):
                root(std::move(rhs.root)),
                m_size(std::move(rhs.m_size))
            { }

            ~MultiKeyMap() = default;

            MultiKeyMap& operator=(const MultiKeyMap& rhs) {
                _MKM_DEBUG_OUTPUT << "operator=" << std::endl;

                new(this) MultiKeyMap(rhs);

                return *this;
            }

            /**
             * @brief Inserts a value into the map. Only full keys are allowed.
             *        Will not insert the value if something already exists for
             *        that key.
             *
             * @param key A full key to insert value at
             * @param value The value to insert
             *
             * @return True if the value was inserted successfully, false
             *         otherwise.
             */
            bool insert(const key_type& key, const V& value) {
                _MKM_DEBUG_OUTPUT << "insert" << std::endl;

                NodePtr node = getNodeForPartialKey<Keys...>(key, true);
                _MKM_DEBUG_OUTPUT << "  " << node.get() << std::endl;

                // Only insert the value if it does not already exist
                //   Return true if we inserted the value, false otherwise
                if(node->data == std::nullopt) {
                    node->data = { key, value };
                    ++m_size;
                    return true;
                } else {
                    return false;
                }
            }

            /**
             * @brief Base implementation for Iterator type.
             *
             * @tparam NodeType The type of node that can be looked at. Meant to
             *                  differentiate between <tt>Node</tt> and
             *                  <tt>const Node</tt>.
             */
            template<typename NodeType = Node>
            class IteratorImpl {
                protected:
                    //! Helper type alisas
                    using IterNodePtr = std::shared_ptr<NodeType>;

                    //! Helper type alias for m_nodes
                    using NodeStack = std::stack<IterNodePtr>;

                public:
                    /**
                     * @brief Builds a new Iterator starting at the given node
                     *
                     * @param node The node to start iterating from.
                     */
                    IteratorImpl(IterNodePtr node):
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

                    /**
                     * @brief Constructs a new iterator not pointing at any
                     *        node. Represents the end iterator.
                     */
                    IteratorImpl(): m_nodes() { }

                    /**
                     * @brief Increments the iterator to the next node that has
                     *        a value (or to the end if no remaining nodes have
                     *        values).
                     *
                     * @return This iterator
                     */
                    IteratorImpl<NodeType>& operator++() {
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

                    /**
                     * @brief Compares this iterator against another of the same
                     *        NodeType
                     *
                     * @param it The other iterator to compare against.
                     *
                     * @return True if they are not equal, false otherwise.
                     */
                    bool operator!=(const IteratorImpl<NodeType>& it) const noexcept
                    {
                        return !(*this == it);
                    }

                    /**
                     * @brief Compares this iterator against another of the same
                     *        NodeType
                     *
                     * @param it The other iterator to compare against.
                     *
                     * @return True if they are equal, false otherwise.
                     */
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

                    /**
                     * @brief Tests if this iterator is at the end.
                     *
                     * @return True if there are no nodes left, false otherwise.
                     */
                    bool isEnd() const noexcept {
                        return m_nodes.empty();
                    }

                protected:
                    /**
                     * @brief Advances the iterator by one node.
                     * @details Performs one single step of a breadth-first
                     *          search of the node tree. Will assume that
                     *          m_nodes is not empty.
                     */
                    void advance() noexcept {
                        _MKM_DEBUG_OUTPUT << "advance()" << std::endl;
                        auto node = m_nodes.top();
                        m_nodes.pop();

                        // Iterate over every possible child in the tuple
                        //   and add its children map to the stack
                        detail::forEach(node->children, [this](auto& child_map)
                        {
                            _MKM_DEBUG_OUTPUT << "  " << child_map.size() << " children" << std::endl;
                            for(auto&& [v, child] : child_map) {
                                _MKM_DEBUG_OUTPUT << "    " << v << std::endl;
                                // 'v' is stored in the child node, and can be
                                //   ignored here
                                m_nodes.push(child);
                            }
                        });
                    }

                    /**
                     * @brief Gets m_nodes.
                     */
                    NodeStack& getNodeStack() noexcept {
                        return m_nodes;
                    }

                    /**
                     * @brief Gets m_nodes.
                     */
                    const NodeStack& getNodeStack() const noexcept {
                        return m_nodes;
                    }

                private:
                    //! The stack of nodes being iterated over.
                    NodeStack m_nodes;
            };

            /**
             * @brief Non-const implementation of IteratorImpl.
             */
            struct Iterator: public IteratorImpl<Node> {
                /**
                 * @brief Dereferences iterator and returns value_type
                 *
                 * @return The currently looked at key and value.
                 */
                value_type& operator*() const noexcept {
                    return this->getNodeStack().top()->data.value();
                }

                /**
                 * @brief Dereferences iterator and returns value_type
                 *
                 * @return The currently looked at key and value.
                 */
                value_type* operator->() const noexcept {
                    return &this->getNodeStack().top()->data.value();
                }
            };

            /**
             * @brief Const implementation of IteratorImpl.
             */
            struct ConstIterator: public IteratorImpl<const Node> {
                /**
                 * @brief Dereferences iterator and returns value_type
                 *
                 * @return The currently looked at key and value.
                 */
                const value_type& operator*() const noexcept {
                    return this->getNodeStack().top()->data.value();
                }

                /**
                 * @brief Dereferences iterator and returns value_type
                 *
                 * @return The currently looked at key and value.
                 */
                const value_type* operator->() const noexcept {
                    return &this->getNodeStack().top()->data.value();
                }
            };

            /**
             * @brief Returns a ConstIterator to the beginning of the map.
             */
            ConstIterator begin() const noexcept {
                return ConstIterator{std::const_pointer_cast<const Node>(root)};
            }

            /**
             * @brief Returns a ConstIterator to the beginning of the map.
             */
            ConstIterator cbegin() const noexcept {
                return ConstIterator{std::const_pointer_cast<const Node>(root)};
            }

            /**
             * @brief Returns a ConstIterator to the end of the map.
             */
            ConstIterator end() const noexcept {
                return ConstIterator{};
            }

            /**
             * @brief Returns a ConstIterator to the end of the map.
             */
            ConstIterator cend() const noexcept {
                return ConstIterator{};
            }

            /**
             * @brief Returns a Iterator to the beginning of the map.
             */
            Iterator begin() noexcept {
                return Iterator{root};
            }

            /**
             * @brief Returns a Iterator to the beginning of the map.
             */
            Iterator end() noexcept {
                return Iterator{};
            }

            /**
             * @brief Finds all values in the map that match the given key.
             * @details \c key can be only partially provided, so long as they
             *          are in the same order as <tt>Keys...</tt> and so long as
             *          no parts of the key are skipped.
             *
             * @par If a key is only partially given, then the iterator will
             *      provide a range of values that match the key more
             *      specifically and can be iterated for all possible matches.
             *      See TrieTests::ValidateComplexTriePartialKeyLookup for
             *      examples.
             *
             * @tparam PartialKey The types for key lookup
             * @param key The key to use for lookup
             *
             * @return An iterator to all nodes that match the given key
             */
            template<typename... PartialKey>
            Iterator find(const std::tuple<PartialKey...>& key) noexcept {
                _MKM_DEBUG_OUTPUT << "find()" << std::endl;
                NodePtr node = getNodeForPartialKey(key);
                _MKM_DEBUG_OUTPUT << "  " << node.get() << std::endl;

                return Iterator{node};
            }

            /**
             * @brief Finds all values in the map that match the given key.
             * @details \c key can be only partially provided, so long as they
             *          are in the same order as <tt>Keys...</tt> and so long as
             *          no parts of the key are skipped.
             *
             * @par If a key is only partially given, then the iterator will
             *      provide a range of values that match the key more
             *      specifically and can be iterated for all possible matches.
             *      See TrieTests::ValidateComplexTriePartialKeyLookup for
             *      examples.
             *
             * @tparam PartialKey The types for key lookup
             * @param key The key to use for lookup
             *
             * @return An iterator to all nodes that match the given key
             */
            template<typename... PartialKey>
            Iterator find(const PartialKey&... key) noexcept {
                _MKM_DEBUG_OUTPUT << "find()" << std::endl;
                std::tuple<std::decay_t<PartialKey>...> tkey = std::make_tuple(key...);

                NodePtr node = getNodeForPartialKey<std::decay_t<PartialKey>...>(tkey);
                _MKM_DEBUG_OUTPUT << "  " << node.get() << std::endl;

                return Iterator{node};
            }

            ///////

            /**
             * @brief Finds all values in the map that match the given key.
             * @details For specific details on how this works, see non-const
             *          find(PartialKey...).
             *
             * @tparam PartialKey The types for key lookup
             * @param key The key to use for lookup
             *
             * @return An iterator to all nodes that match the given key
             */
            template<typename... PartialKey>
            ConstIterator find(const PartialKey&... key) const noexcept {
                _MKM_DEBUG_OUTPUT << "find() const" << std::endl;
                std::tuple<std::decay_t<PartialKey>...> tkey = std::make_tuple(key...);

                return find(tkey);
            }

            /**
             * @brief Finds all values in the map that match the given key.
             * @details For specific details on how this works, see non-const
             *          find(PartialKey...).
             *
             * @tparam PartialKey The types for key lookup
             * @param key The key to use for lookup
             *
             * @return An iterator to all nodes that match the given key
             */
            template<typename... PartialKey>
            ConstIterator find(const std::tuple<PartialKey...>& key) const noexcept
            {
                _MKM_DEBUG_OUTPUT << "find()" << std::endl;
                ConstNodePtr node = getNodeForPartialKey(key);
                _MKM_DEBUG_OUTPUT << "  " << node.get() << std::endl;

                return ConstIterator{node};
            }

            /**
             * @brief Erases all elements.
             */
            void clear() noexcept {
                m_size = 0;
                root.reset(new Node);
            }

            template<typename... PartialKey>
            size_type count(const std::tuple<PartialKey...>& key) const noexcept
            {
                _MKM_DEBUG_OUTPUT << "count()" << std::endl;
                size_type s = 0;
                for(auto it = find(key); it != end(); ++it, ++s)
                    _MKM_DEBUG_OUTPUT << "  ++" << std::endl;
                return s;
            }

            template<typename... PartialKey>
            size_type count(const PartialKey&... key) const noexcept
            {
                std::tuple<std::decay_t<PartialKey>...> tkey = std::make_tuple(key...);
                return count<PartialKey...>(tkey);
            }

            template<typename... PartialKey>
            bool contains(const PartialKey&... key) const noexcept {
                return find(key...) != end();
            }

            template<typename... PartialKey>
            bool contains(const std::tuple<PartialKey...>& key) const noexcept {
                return find(key) != end();
            }

            bool operator==(const MultiKeyMap& rhs) const noexcept {
                if(size() != rhs.size()) return false;

                for(auto&& [k,v] : rhs) {
                    auto it = find(k);
                    if(it == end()) return false;
                    if(it->second != v) return false;
                }

                return true;
            }

            bool operator!=(const MultiKeyMap& rhs) const noexcept {
                return !((*this) == rhs);
            }

            /**
             * @brief Returns a reference to the value found for the given key.
             *        If no such element exists, a std::out_of_range exception
             *        is thrown. 
             *
             * @param key The key to search for.
             *
             * @return A reference to the value at key
             */
            mapped_type& at(const Keys&... key) {
                _MKM_DEBUG_OUTPUT << "at()" << std::endl;
                auto it = find(key...);
                if(it == end()) {
                    throw std::out_of_range("Requested key not found.");
                }

                return it->second;
            }

            /**
             * @brief Returns a reference to the value found for the given key.
             *        If no such element exists, a std::out_of_range exception
             *        is thrown. 
             *
             * @param key The key to search for.
             *
             * @return A reference to the value at key
             */
            mapped_type& at(const key_type& key) {
                _MKM_DEBUG_OUTPUT << "at()" << std::endl;
                auto it = find(key);
                if(it == end()) {
                    throw std::out_of_range("Requested key not found.");
                }

                return it->second;
            }

            /**
             * @brief Returns a reference to the value found for the given key.
             *        If no such element exists, a std::out_of_range exception
             *        is thrown. 
             *
             * @param key The key to search for.
             *
             * @return A reference to the value at key
             */
            const mapped_type& at(const Keys&... key) const {
                auto it = find(key...);
                if(it == end()) {
                    throw std::out_of_range("Requested key not found.");
                }

                return it->second;
            }

            /**
             * @brief Returns a reference to the value found for the given key.
             *        If no such element exists, a std::out_of_range exception
             *        is thrown. 
             *
             * @param key The key to search for.
             *
             * @return A reference to the value at key
             */
            const mapped_type& at(const key_type& key) const {
                auto it = find(key);
                if(it == end()) {
                    throw std::out_of_range("Requested key not found.");
                }

                return it->second;
            }

            /**
             * @brief Returns a reference to the value found for a given key.
             *        If no such element exists, then a new one is inserted.
             *
             * @param key The key to search for.
             *
             * @return A reference to the value at key.
             */
            mapped_type& operator[](const key_type& key) {
                _MKM_DEBUG_OUTPUT << "operator[]()" << std::endl;

                NodePtr node = getNodeForPartialKey<Keys...>(key, true);
                _MKM_DEBUG_OUTPUT << "  " << node.get() << std::endl;

                // Only insert the value if it does not already exist
                //   Return true if we inserted the value, false otherwise
                if(node->data == std::nullopt) {
                    node->data = { key, mapped_type{} };
                    ++m_size;
                }

                // By this point we should be guaranteed for node->data to hold
                //   a value
                return node->data->second;
            }

            /**
             * @brief Returns a reference to the value found for a given key.
             *        If no such element exists, then a new one is inserted.
             *
             * @param key The key to search for.
             *
             * @return A reference to the value at key.
             */
            mapped_type& operator[](key_type&& key) {
                _MKM_DEBUG_OUTPUT << "operator[]()" << std::endl;

                NodePtr node = getNodeForPartialKey<Keys...>(key, true);
                _MKM_DEBUG_OUTPUT << "  " << node.get() << std::endl;

                // Only insert the value if it does not already exist
                //   Return true if we inserted the value, false otherwise
                if(node->data == std::nullopt) {
                    node->data = { key, mapped_type{} };
                    ++m_size;
                }

                return node->data.second;
            }

            /**
             * @brief Returns the number of elements in the map.
             *
             * @return The number of elements in the map
             */
            size_type size() const noexcept {
                return m_size;
            }

            /**
             * @brief Returns true if the number of elements is 0.
             */
            bool empty() const noexcept {
                return m_size == 0;
            }


            /**
             * @brief Erases all values that match the given key.
             *
             * @tparam PartialKey The types used to build the key.
             * @param key The key used to lookup the value to erase
             */
            template<typename... PartialKey>
            void erase(std::tuple<PartialKey...> key) {
                _MKM_DEBUG_OUTPUT << "erase()" << std::endl;
                NodePtr node = getNodeForPartialKey(key);
                _MKM_DEBUG_OUTPUT << "  node=" << node.get() << std::endl;

                auto c = 0;
                // Decrease the size of this element by the number of values
                //   that are getting removed
                for(auto it = Iterator{node}; it != end(); ++it)
                {
# ifndef _MKM_DEFAULT_DEBUG
                    ++c;
# endif
                    --m_size;
                }
                _MKM_DEBUG_OUTPUT << "  Dec size by " << c << std::endl;

                // For each part of the key, erase all children
                ([&] {
                    _MKM_DEBUG_OUTPUT << "  Clear node child type " << typeid(Keys).name() << std::endl;
                    getChildrenTypeFromTuple<Keys>(node->children).clear();
                }(), ...);

                if(node->parent != nullptr) {
                    constexpr std::size_t last_idx = sizeof...(PartialKey) - 1;
                    std::get<last_idx>(node->parent->children).erase(std::get<last_idx>(key));
                }
            }

            void swap(MultiKeyMap& rhs) noexcept {
                std::swap(m_size, rhs.m_size);
                std::swap(root, rhs.root);
            }

            void merge(MultiKeyMap& rhs) noexcept {
                for(auto&& [k, v] : rhs) {
                    if(insert(k, v)) {
                        rhs.erase(k);
                    }
                }
            }

        protected:
            /**
             * @brief Gets the node for the given partial key.
             *
             * @tparam PartialKey The types for the partial key
             * @param key The key to search for
             *
             * @return A shared_ptr to the node for the given key.
             */
            template<typename... PartialKey>
            ConstNodePtr getNodeForPartialKey(const std::tuple<PartialKey...>& key) const noexcept
            {
                _MKM_DEBUG_OUTPUT << "getNodeForPartialKey() const" << std::endl;
                return getNodeForPartialKey(key, std::make_index_sequence<sizeof...(PartialKey)>{});
            }

            /**
             * @brief Gets the node for the given partial key. Wraps each key
             *        part in Wrapper.
             *
             * @tparam T The tuple type for key
             * @tparam Indices
             * @param key The key to search for
             * @param std::index_sequence
             *
             * @return A shared_ptr to the node for the given key.
             */
            template<typename T, std::size_t... Indices>
            ConstNodePtr getNodeForPartialKey(const T& key,
                                              std::index_sequence<Indices...>) const noexcept
            {
                _MKM_DEBUG_OUTPUT << "getNodeForPartialKey<T, I...>() const" << std::endl;
                return getNodeForPartialKeyImpl(
                    std::make_tuple(makeWrapper( std::get<Indices>(key) )...));
            }

            /**
             * @brief Implementation detail for getNodeForPartialKey
             *
             * @tparam PartialKey The types for the partial key
             * @param key A partial key.
             *
             * @return  A shared_ptr to the node for the given key.
             */
            template<typename... PartialKey>
            ConstNodePtr getNodeForPartialKeyImpl(const std::tuple<Wrapper<PartialKey>...>& key) const noexcept
            {
                _MKM_DEBUG_OUTPUT << "getNodeForPartialKeyImpl() const" << std::endl;
                ConstNodePtr node = root;

                std::apply([&](auto&&... args) {
                    node = getNodeForPartialKeyImpl<PartialKey...>(args...);
                }, key);

                return node;
            }

            /**
             * @brief Gets the node for the given partial key.
             *
             * @tparam PartialKey The types for the partial key
             * @param key The key to search for
             *
             * @return A shared_ptr to the node for the given key.
             */
            template<typename... PartialKey>
            ConstNodePtr getNodeForPartialKeyImpl(const Wrapper<PartialKey>&... key) const noexcept
            {
                _MKM_DEBUG_OUTPUT << "getNodeForPartialKeyImpl(Wrapper...) const" << std::endl;
                ConstNodePtr node = root;

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

                    _MKM_DEBUG_OUTPUT << "  key=" << key.value << std::endl;

                    // Stop early if node is null
                    if(node == nullptr) {
                        _MKM_DEBUG_OUTPUT << "  node is null" << std::endl;
                        return false;
                    }

                    // Find the correct 'children' for this type
                    // Children are stored in a tuple of hashmap<T, shared_ptr<Node> >
                    const auto& children = getChildrenTypeFromTuple<RawType>(node->children);

                    _MKM_DEBUG_OUTPUT << "  children[" << children.size() << " elements]" << std::endl;

                    // If the node for this key value doesn't exist, then set
                    //  'node' to null to denote a lookup failure
                    if(children.count(key.value) == 0) {
                        _MKM_DEBUG_OUTPUT << "  children does not contain " << key.value << std::endl;
                        node = nullptr;
                        return false;
                    } else {
                        node = children.at(key.value);
                    }

                    return true;
                }() && ...);

                return node;
            }

            ////////

            /**
             * @brief Gets the node for the given partial key.
             *
             * @tparam PartialKey The types for the partial key
             * @param key The key to search for
             *
             * @return A shared_ptr to the node for the given key.
             */
            template<typename... PartialKey>
            NodePtr getNodeForPartialKey(const std::tuple<PartialKey...>& key,
                                         bool createIfKeyDoesNotExist = false) noexcept
            {
                return getNodeForPartialKey(key, std::make_index_sequence<sizeof...(PartialKey)>{}, createIfKeyDoesNotExist);
            }

            /**
             * @brief Gets the node for the given partial key. Wraps each key
             *        part in Wrapper.
             *
             * @tparam T The tuple type for key
             * @tparam Indices
             * @param key The key to search for
             * @param std::index_sequence
             *
             * @return A shared_ptr to the node for the given key.
             */
            template<typename T, std::size_t... Indices>
            NodePtr getNodeForPartialKey(const T& key,
                                         std::index_sequence<Indices...>,
                                         bool createIfKeyDoesNotExist = false) noexcept
            {
                return getNodeForPartialKeyImpl(
                    std::make_tuple(makeWrapper( std::get<Indices>(key) )...),
                    createIfKeyDoesNotExist);
            }

            /**
             * @brief Implementation detail for getNodeForPartialKey
             *
             * @tparam PartialKey The types for the partial key
             * @param key A partial key.
             *
             * @return  A shared_ptr to the node for the given key.
             */
            template<typename... PartialKey>
            NodePtr getNodeForPartialKeyImpl(const std::tuple<Wrapper<PartialKey>...>& key,
                                             bool createIfKeyDoesNotExist = false)
            {
                NodePtr node = root;

                std::apply([&](auto&&... args) {
                    node = getNodeForPartialKeyImpl<PartialKey...>(args..., createIfKeyDoesNotExist);
                }, key);

                return node;
            }

            /**
             * @brief Gets the node for the given partial key.
             *
             * @tparam PartialKey The types for the partial key
             * @param key The key to search for
             *
             * @return A shared_ptr to the node for the given key.
             */
            template<typename... PartialKey>
            NodePtr getNodeForPartialKeyImpl(const Wrapper<PartialKey>&... key,
                                             bool createIfKeyDoesNotExist = false)
            {
                NodePtr node = root;
                NodePtr parent = nullptr;

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
                            node->parent = parent;
                            parent = node;
                        } else {
                            node = nullptr;
                        }
                    } else {
                        node = children[key.value];
                        parent = node;
                    }

                    return true;
                }() && ...);

                return node;
            }

            /**
             * @brief Gets the specific Children map from the children tuple
             *        based on the provided key part type
             *
             * @tparam T The key part to look for
             * @tparam Types All types that are in the children tuple
             * @param t The tuple of child maps to get a children map from
             *
             * @return A specific children map from 't' corresponding to the type T
             */
            template<typename T, typename... Types>
            NodeChildren<T>& getChildrenTypeFromTuple(std::tuple<Types...>& t) noexcept
            {
                constexpr std::size_t Index = detail::getIndexOfType<NodeChildren<T>, Types...>();
                return std::get<Index>(t);
            }

            /**
             * @brief Gets the specific Children map from the children tuple
             *        based on the provided key part type
             *
             * @tparam T The key part to look for
             * @tparam Types All types that are in the children tuple
             * @param t The tuple of child maps to get a children map from
             *
             * @return A specific children map from 't' corresponding to the type T
             */
            template<typename T, typename... Types>
            const NodeChildren<T>& getChildrenTypeFromTuple(const std::tuple<Types...>& t) const noexcept
            {
                constexpr std::size_t Index = detail::getIndexOfType<NodeChildren<T>, Types...>();
                return std::get<Index>(t);
            }

        private:
            //! The root node
            NodePtr root;

            size_type m_size;
    };
}

namespace std {
    template<typename V, typename... Keys>
    void swap(mkm::MultiKeyMap<V, Keys...>& lhs,
              mkm::MultiKeyMap<V, Keys...>& rhs) noexcept
    {
        lhs.swap(rhs);
    }
}

#endif

