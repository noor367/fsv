#include "./filtered_string_view.h"
#include <algorithm>
#include <sstream>

namespace fsv {
    filter fsv::filtered_string_view::default_predicate = [](const char&) { return true; };
    /**
        Constructors
    */
    filtered_string_view::filtered_string_view()
    : pointer_(nullptr)
    , length_(0)
    , predicate_(default_predicate) {}

    filtered_string_view::filtered_string_view(const std::string& str, filter pred)
    : pointer_(str.data())
    , length_(str.size())
    , predicate_(pred) {}

    filtered_string_view::filtered_string_view(const char* str, filter pred)
    : pointer_(str)
    , length_(std::strlen(str))
    , predicate_(pred) {}

    filtered_string_view::filtered_string_view(const filtered_string_view& other)
    : pointer_(other.pointer_)
    , length_(other.length_)
    , predicate_(other.predicate_) {}

    filtered_string_view::filtered_string_view(filtered_string_view&& other) noexcept
    : pointer_(other.pointer_)
    , length_(other.length_)
    , predicate_(std::move(other.predicate_)) {
        other.pointer_ = nullptr;
        other.length_ = 0;
        other.predicate_ = filter{};
    }

    /**
        Member operators
    */
    auto filtered_string_view::operator=(const filtered_string_view& other) -> filtered_string_view& {
        if (this != &other) {
            pointer_ = other.pointer_;
            length_ = other.length_;
            predicate_ = other.predicate_;
        }
        return *this;
    }

    auto filtered_string_view::operator=(filtered_string_view&& other) -> filtered_string_view& {
        if (this != &other) {
            pointer_ = other.pointer_;
            length_ = other.length_;
            predicate_ = std::move(other.predicate_);

            other.pointer_ = nullptr;
            other.length_ = 0;
            other.predicate_ = filter{};
        }
        return *this;
    }

    auto filtered_string_view::operator[](std::size_t n) const -> const char& {
        auto count = std::size_t{0};
        for (auto i = std::size_t{0}; i < length_; ++i) {
            if (predicate_(pointer_[i])) {
                if (count == n) {
                    return pointer_[i];
                }
                ++count;
            }
        }
        return pointer_[0];
    }

    filtered_string_view::operator std::string() {
        auto result = std::string{};
        result.reserve(length_);
        for (auto i = std::size_t{0}; i < length_; ++i) {
            if (predicate_(pointer_[i])) {
                result.push_back(pointer_[i]);
            }
        }
        return result;
    }

    auto filtered_string_view::at(std::size_t index) -> const char& {
        auto count = std::size_t{0};
        for (auto i = std::size_t{0}; i < length_; ++i) {
            if (predicate_(pointer_[i])) {
                if (count == index) {
                    return pointer_[i];
                }
                ++count;
            }
        }
        std::ostringstream oss;
        oss << "filtered_string_view::at(" << index << "): invalid index";
        throw std::domain_error(oss.str());
    }

    auto filtered_string_view::size() const -> std::size_t {
        auto count = std::size_t{0};
        for (auto i = std::size_t{0}; i < length_; ++i) {
            if (predicate_(pointer_[i])) {
                ++count;
            }
        }
        return count;
    }

    auto filtered_string_view::empty() const -> bool {
        return (size() == 0);
    }

    auto filtered_string_view::data() const -> const char* {
        return pointer_;
    }

    auto filtered_string_view::predicate() const -> const filter& {
        return predicate_;
    }

    /**
        non-member operators
    */
    auto operator==(const filtered_string_view& lhs, const filtered_string_view& rhs) -> bool {
        if (lhs.size() != rhs.size()) {
            return false;
        }
        for (auto i = std::size_t{0}; i < lhs.size(); ++i) {
            if (lhs[i] != rhs[i]) {
                return false;
            }
        }
        return true;
    }

    auto operator<(const filtered_string_view& lhs, const filtered_string_view& rhs) -> bool {
        return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    auto operator>(const filtered_string_view& lhs, const filtered_string_view& rhs) -> bool {
        return rhs < lhs;
    }

    auto operator<=(const filtered_string_view& lhs, const filtered_string_view& rhs) -> bool {
        return !(rhs < lhs);
    }

    auto operator>=(const filtered_string_view& lhs, const filtered_string_view& rhs) -> bool {
        return !(lhs < rhs);
    }

    auto operator<<(std::ostream& os, const filtered_string_view& fsv) -> std::ostream& {
        for (auto it = fsv.begin(); it != fsv.end(); ++it) {
            os << *it;
        }
        return os;
    }

    /**
        iterator class
    */
    auto fsv::filtered_string_view::iter::operator++() -> iter& {
        do {
            ++pos_;
        } while (pos_ < view_->length_ && !view_->predicate_(view_->pointer_[pos_]));
        return *this;
    }

    auto fsv::filtered_string_view::iter::operator++(int) -> iter {
        iter copy = *this;
        ++(*this);
        return copy;
    }

    auto fsv::filtered_string_view::iter::operator--() -> iter& {
        do {
            --pos_;
        } while (pos_ > 0 && !view_->predicate_(view_->pointer_[pos_]));
        return *this;
    }

    auto fsv::filtered_string_view::iter::operator--(int) -> iter {
        iter copy = *this;
        --(*this);
        return copy;
    }

    /**
        non-member utility functions
    */
    auto compose(const filtered_string_view& fsv, const std::vector<filter>& filts) -> filtered_string_view {
        return filtered_string_view{fsv.data(), [filts](const char& c) {
                                        for (const auto& filt : filts) {
                                            if (!filt(c))
                                                return false;
                                        }
                                        return true;
                                    }};
    }

    auto substr(const filtered_string_view& fsv, size_t pos, std::optional<size_t> count) -> filtered_string_view {
        auto indices = std::vector<std::size_t>{};
        auto const data = fsv.data();
        for (auto i = size_t{0}; i < fsv.size(); ++i) {
            if (fsv.predicate()(data[i])) {
                indices.push_back(i);
            }
        }
        auto const filtered_size = indices.size();
        if (pos > filtered_size) {
            throw std::out_of_range{"filtered_string_view::substr(" + std::to_string(pos)
                                    + "): position out of range for filtered string of size "
                                    + std::to_string(filtered_size)};
        }
        auto end = count.has_value() ? std::min(pos + count.value(), filtered_size) : filtered_size;
        if (pos == end) {
            return filtered_string_view{fsv.data(), [](const char&) { return false; }};
        }
        auto i_start = indices[pos];
        auto i_end = indices[end - 1] + 1;

        auto new_pred = [=, pred = fsv.predicate()](const char& c) -> bool {
            auto offset = &c - data;
            return offset >= static_cast<std::ptrdiff_t>(i_start) && offset < static_cast<std::ptrdiff_t>(i_end)
                   && pred(c);
        };

        return filtered_string_view{fsv.data(), new_pred};
    }

    auto split(const filtered_string_view& fsv, const filtered_string_view& tok) -> std::vector<filtered_string_view> {
        auto fsv_filtered = std::string{};
        for (auto c : fsv)
            fsv_filtered.push_back(c);

        auto tok_filtered = std::string{};
        for (auto c : tok)
            tok_filtered.push_back(c);

        if (tok_filtered.empty() || fsv_filtered.empty()) {
            return {fsv};
        }
        auto result = std::vector<filtered_string_view>{};
        auto start = std::size_t{0};

        while (start <= fsv_filtered.size()) {
            auto pos = fsv_filtered.find(tok_filtered, start);
            if (pos == std::string::npos) {
                auto part = substr(fsv, start);
                result.push_back(part);
                break;
            }
            auto part = substr(fsv, start, pos - start);
            result.push_back(part);

            start = pos + tok_filtered.size();
        }

        return result;
    }

} // namespace fsv