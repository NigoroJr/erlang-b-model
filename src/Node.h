#ifndef NODE_H_
#define NODE_H_

#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Node {
public:
    using id_t = unsigned;
    using wavelength_t = unsigned;
    using Wavelengths = std::unordered_set<wavelength_t>;

    /* No wavelength available */
    static const wavelength_t NONE;

    /* Constructors, Destructor, and Assignment operators {{{ */
    // Default constructor
    Node();

    /**
     * Creates a node with the given number of links.
     * With this constructor, it is assumed that the wavelengths do not
     * matter, and thus the constructor will assign arbitrary but unique
     * wavelength to each link. Note that there are links for each port.
     *
     * \param[in] num_links the number of links per port in the node.
     *
     * \param[in] has_converter whether or not this node has a wavelength
     *            converter. Defaults to false.
     */
    Node(const id_t id, const unsigned num_links, bool has_converter = false);
    Node(const id_t id, const int num_links, bool has_converter = false);

    /**
     * Creates a node with links of given wavelengths.
     *
     * \param[in] wavelengths the wavelengths of each link.
     *
     * \param[in] has_converter whether or not this node has a wavelength
     *            converter. Defaults to false.
     *
     * Note: Implementation here because of template linking error
     */
    template<typename T>
    Node(const id_t id, const T& wavelengths, bool has_converter = false)
        : id_{id}
        , wavelengths_{wavelengths.begin(), wavelengths.end()}
        , has_converter_{has_converter}
    { }

    // Copy constructor
    Node(const Node& other);

    // Move constructor
    Node(Node&& other);

    // Destructor
    ~Node();

    // Assignment operator
    Node&
    operator=(const Node& other);

    // Move assignment operator
    Node&
    operator=(Node&& other);
    /* }}} */

    id_t
    id() const;

    /**
     * Checks if the given wavelength can be used or not.
     * A wavelength cannot be used if this node does not have that wavelength
     * or if it's already used (i.e. locked).
     *
     * \return true if the wavelength can be used, false otherwise.
     */
    bool
    can_use(const wavelength_t) const;

    /**
     * Reserves a wavelength by locking it.
     *
     * \param[in] wl the wavelength to use.
     *
     * \return true if lock was successful, false otherwise.
     */
    bool
    lock(const wavelength_t wl);

    /**
     * Releases the wavelength.
     * Nothing is done if no connection currently exists.
     */
    void
    release(const wavelength_t wl);

    /**
     * \return set of all the possible wavelengths.
     */
    const Wavelengths&
    wavelengths() const;

    /**
     * \return the set of used wavelengths.
     */
    const Wavelengths&
    used_wavelengths() const;

    /**
     * \return the set of wavelengths that are not used.
     */
    Wavelengths
    available_wavelengths() const;

    /**
     * \return the number of ports that this node has.
     */
    unsigned
    num_wavelengths() const;

    /**
     * \return true if this node has wavelength conversion capability, false
     *         otherwise.
     */
    bool
    has_converter() const;

private:
    id_t id_;
    Wavelengths wavelengths_;
    Wavelengths used_;
    bool has_converter_;
};

/* Inlined methods */
inline Node::id_t
Node::id() const {
    return id_;
}

inline const Node::Wavelengths&
Node::wavelengths() const {
    return wavelengths_;
}

inline const Node::Wavelengths&
Node::used_wavelengths() const {
    return used_;
}

inline unsigned
Node::num_wavelengths() const {
    return wavelengths_.size();
}

inline bool
Node::has_converter() const {
    return has_converter_;
}

#endif /* end of include guard */
