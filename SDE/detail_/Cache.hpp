#pragma once
#include <algorithm>
#include <iostream>
#include <string>

#include "SDE/detail_/SDEAny.hpp"

namespace SDE {
using namespace detail_;

/// Class that caches stuff
class Cache {
public:
    /// Type that hashes are stored as
    using hash_type = const std::string;

    /// Type of an iterator over cache entries
    using iterator_type =
      std::map<const hash_type, std::shared_ptr<SDEAny>>::iterator;

    /// Type of a read-only iterator over cache entries
    using const_iterator_type =
      std::map<const hash_type, std::shared_ptr<SDEAny>>::const_iterator;

    /// Type of the graph stored in the cache (deprecated)
    using graph_type = std::map<hash_type, std::multimap<hash_type, hash_type>>;

    /// Makes a cache
    Cache() = default;

    /// Destroys the cache (don't worry your bank account will stay the same)
    ~Cache() = default;

    /// Inserts a hash-data pair into results map
    template<typename T>
    void insert(hash_type& key, T&& data) {
        _results[key] = std::make_shared<SDEAny>(std::move(data));
    }

    /// Computes hash value, and inserts hash-data pair into results map
    template<typename T>
    void insert(T&& data, bphash::HashType htype = bphash::HashType::Hash128) {
        bphash::Hasher h(htype);
        h(data);
        hash_type key = bphash::hash_to_string(h.finalize());
        _results[key] = std::make_shared<SDEAny>(std::move(data));
    }

    /** Returns a shared_ptr to the value held in an SDEAny instance stored in
     * the result map, index specified by hash key
     */
    template<typename T>
    std::shared_ptr<T> at(hash_type& key) const {
        auto rv = _results.at(key);
        // Aliasing constructor is used so the returned pointer reference count
        // is linked to the reference count of the _results shared_ptr
        return std::shared_ptr<T>(rv, &(SDEAnyCast<T&>(*rv)));
    }

    /** Returns a shared_ptr to the value held in an SDEAny instance stored in
     * the result map, index specified by iterator
     */
    template<typename T>
    std::shared_ptr<T> at(iterator_type it) const {
        auto rv = _results.at(it->first);
        // Aliasing constructor is used so the returned pointer reference count
        // is linked to the reference count of the _results shared_ptr
        return std::shared_ptr<T>(rv, &(SDEAnyCast<T&>(*rv)));
    }

    /** Returns a shared_ptr to the value held in an SDEAny instance stored in
     * the result map, index specified by const iterator
     */
    template<typename T>
    std::shared_ptr<T> at(const_iterator_type it) const {
        auto rv = _results.at(it->first);
        // Aliasing constructor is used so the returned pointer reference count
        // is linked to the reference count of the _results shared_ptr
        return std::shared_ptr<T>(rv, &(SDEAnyCast<T&>(*rv)));
    }

    /** Returns the use_count() of the shared_ptr<SDEAny> stored in the result
     * map, index specified by hash key.
     */
    size_t get_use_count(hash_type& key) { return _results[key].use_count(); }

    ///@{
    /** @name Exposing various iterator member functions
     *
     *
     */
    inline const_iterator_type begin() const noexcept {
        return _results.begin();
    }
    inline iterator_type begin() noexcept { return _results.begin(); }
    inline const_iterator_type end() const noexcept { return _results.end(); }
    inline iterator_type end() noexcept { return _results.end(); }
    inline iterator_type find(hash_type& key) { return _results.find(key); }
    inline const_iterator_type find(hash_type& key) const {
        return _results.find(key);
    }
    ///@}

    /// Erase element at position specified by iterator
    iterator_type erase(iterator_type pos) { return _results.erase(pos); }

    /// Erase element at position specified by const iterator
    iterator_type erase(const_iterator_type pos) { return _results.erase(pos); }

    /// Erase range of elements with range specified by iterators
    iterator_type erase(const_iterator_type first, const_iterator_type last) {
        return _results.erase(first, last);
    }

    /// Erase element at position specified by hash key
    size_t erase(hash_type& key) { return _results.erase(key); }

    /// Get total number of elements stored in the _results map
    size_t size() const noexcept { return _results.size(); }

    /** Returns the number of stored elements indexed with the specified hash
     * key. Returns 1 or 0 since std::map does not allow duplicates
     */
    inline size_t count(hash_type& key) const noexcept {
        return _results.count(key);
    }

    /** Checks for entries in an external Cache that don't exist in our
     * _results map. Any such entries are added to our _results map.
     */
    void synchronize(Cache& ref) {
        std::copy_if(
          ref._results.begin(), ref._results.end(),
          std::inserter(_results, _results.end()), [this](auto any_pair) {
              return (_results.find(any_pair.first) == _results.end());
          });
    }

    /** Check for equality with another Cache. Two Caches are equal if they
     * have their result maps contain the same number of entries, with the exact
     * same hash keys.
     */
    bool operator==(const Cache& ref) const noexcept {
        if(_results.size() != ref._results.size()) return false;
        for(auto const& it : _results)
            if(ref._results.count(it.first) == 0) return false;
        return true;
    }

    /// Negates operator==
    bool operator!=(const Cache& ref) const noexcept {
        return _results != ref._results;
    }

    /// Adds a node
    void add_node(std::string parent_valKey,
                  std::pair<std::string, std::string> submod_node) {
        _graph[parent_valKey].insert(submod_node);
    }

    /**
     * @brief     Returns a shared_ptr to the value held in an SDEAny instance
     * stored in the result map. This function is used when the desired value is
     * an intermediate result computed by a submodule, for which the associated
     * input key is unknown. The result is instead specified by providing the
     * hash value of the parent module, and the hash value of the submodule
     * state (i.e. the return value of ModuleBase::Memoize() with no arguments).
     *
     * This function currently has two major limitations:
     *  1) It will only search to a nested call depth of one
     * (Module->Submodule). 2) Will always return the last result computed by
     * the submodule. No option to change this behavior.
     */

    template<typename T>
    std::shared_ptr<T> at_path(hash_type& parent_valKey,
                               hash_type& daughter_modKey) const {
        auto mm = _graph.at(parent_valKey);
        for(auto i = mm.end(); i != mm.begin(); i--)
            if(i->second == daughter_modKey) return this->at<T>(i->first);
        throw std::out_of_range(
          "Specified Path Not Found in Module Invocation Graph");
    }

private:
    /// Stores the results
    std::map<hash_type, std::shared_ptr<SDEAny>> _results;

    /// The graph
    graph_type _graph;
};
} // End namespace SDE
