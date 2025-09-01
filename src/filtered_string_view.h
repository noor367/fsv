#ifndef COMP6771_ASS2_FSV_H
#define COMP6771_ASS2_FSV_H

#include <compare>
#include <cstring>
#include <functional>
#include <iterator>
#include <optional>
#include <string>

namespace fsv {
    using filter = std::function<bool(const char&)>;

    class filtered_string_view {
        class iter {
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = char;
            using difference_type = std::ptrdiff_t;
            using reference = const char&;
            using pointer = void;

            iter() = default;

            auto operator*() const noexcept -> reference {
                return view_->pointer_[pos_];
            }

            auto operator->() const noexcept -> pointer;

            auto operator++() -> iter&;
            auto operator++(int) -> iter;
            auto operator--() -> iter&;
            auto operator--(int) -> iter;

            friend auto operator==(const iter& a, const iter& b) noexcept -> bool {
                return a.view_ == b.view_ && a.pos_ == b.pos_;
            }

            friend auto operator!=(const iter& a, const iter& b) noexcept -> bool {
                return !(a == b);
            }

        private:
            friend class filtered_string_view;
            iter(const filtered_string_view* view, std::size_t pos) noexcept
            : view_(view)
            , pos_(pos) {}
            /* Implementation-specific private members */
            const filtered_string_view* view_ = nullptr;
            std::size_t pos_ = 0;
            /* Implementation-specific helper functions*/
        };

    public:
        static filter default_predicate;

        /**
            Constructors
        */
        filtered_string_view();
        filtered_string_view(const std::string& str, filter pred = default_predicate);
        filtered_string_view(const char* str, filter pred = default_predicate);

        filtered_string_view(const filtered_string_view& other);
        filtered_string_view(filtered_string_view&& other) noexcept;

        ~filtered_string_view() noexcept = default;

        /**
            member operators
        */
        auto operator=(const filtered_string_view& other) -> filtered_string_view&;
        auto operator=(filtered_string_view&& other) -> filtered_string_view&;
        auto operator[](size_t n) const -> const char&;
        explicit operator std::string();

        /**
            member functions
        */
        auto at(size_t index) -> const char&;
        auto size() const -> std::size_t;
        auto empty() const -> bool;
        auto data() const -> const char*;
        auto predicate() const -> const filter&;

        using iterator = iter;
        using const_iterator = iter;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        auto begin() const -> iterator {
            return iterator(this, first_valid(0));
        }
        auto end() const -> iterator {
            return iterator(this, length_);
        }
        auto cbegin() const -> const_iterator {
            return const_iterator(this, first_valid(0));
        }
        auto cend() const -> const_iterator {
            return const_iterator(this, length_);
        }
        auto rbegin() -> reverse_iterator {
            return reverse_iterator(end());
        }
        auto rend() -> reverse_iterator {
            return reverse_iterator(begin());
        }
        auto crbegin() const -> const_reverse_iterator {
            return const_reverse_iterator(cend());
        }
        auto crend() const -> const_reverse_iterator {
            return const_reverse_iterator(cbegin());
        }

    private:
        /* Implementation-specific helper functions*/
        auto first_valid(std::size_t start) const noexcept -> std::size_t {
            while (start < length_ && !predicate_(pointer_[start])) {
                ++start;
            }
            return start;
        }

        /* Implementation-specific private members */
        const char* pointer_;
        std::size_t length_;
        filter predicate_;
    };

    /**
        non-member operators
    */
    auto operator==(const filtered_string_view& lhs, const filtered_string_view& rhs) -> bool;

    auto operator>(const filtered_string_view& lhs, const filtered_string_view& rhs) -> bool;
    auto operator<(const filtered_string_view& lhs, const filtered_string_view& rhs) -> bool;
    auto operator>=(const filtered_string_view& lhs, const filtered_string_view& rhs) -> bool;
    auto operator<=(const filtered_string_view& lhs, const filtered_string_view& rhs) -> bool;

    auto operator<<(std::ostream& os, const filtered_string_view& fsv) -> std::ostream&;

    /**
        non-member utility functions
    */
    auto compose(const filtered_string_view& fsv, const std::vector<filter>& filts) -> filtered_string_view;
    auto substr(const filtered_string_view& fsv, size_t pos = 0, std::optional<size_t> count = std::nullopt)
        -> filtered_string_view;
    auto split(const filtered_string_view& fsv, const filtered_string_view& tok) -> std::vector<filtered_string_view>;

} // namespace fsv

#endif // COMP6771_ASS2_FSV_H