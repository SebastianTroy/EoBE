#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <vector>
#include <functional>

namespace Tril {

template <typename T>
class CircularBuffer {
public:
    CircularBuffer(size_t capacity)
        : items_(capacity, T{})
        , next_(0)
        , fill_(0)
    {
        items_.shrink_to_fit();
    }

    size_t Size() const { return fill_; }
    size_t Capacity() const { return items_.size(); }
    bool Full() const { return Size() == Capacity(); }
    bool Empty() const { return Size() == 0; }

    T Oldest() const
    {
        if (!items_.empty()) {
            if (Full()) {
                return items_[next_];
            } else {
                return items_.front();
            }
        } else {
            return {};
        }
    }

    T Newest() const
    {
        if (!items_.empty()) {
            if (next_ > 0) {
                return items_[next_ - 1];
            } else {
                return items_.back();
            }
        } else {
            return {};
        }
    }

    // TODO implement an iterator!
    void ForEach(const std::function<void(const T& item)>& action) const
    {
        auto iter = items_.cbegin();
        size_t startIndex = fill_ == items_.size() ? next_ : 0;
        std::advance(iter, startIndex);
        for (size_t count = 0; count < fill_; ++count) {
            action(*iter++);
            if (iter == items_.cend()) {
                iter = items_.cbegin();
            }
        }
    }

    void Resize(size_t size)
    {
        CircularBuffer copy(size);
        ForEach([&](const T& item)
        {
            copy.PushBack(item);
        });
        items_ = std::move(copy.items_);
        items_.resize(size);
        next_ = copy.next_;
        fill_ = copy.fill_;
    }

    void PushBack(const T& item)
    {
        if (items_.size() > 0) {
            items_[next_++] = item;
            if (next_ == items_.size()) {
                next_ = 0;
            }
            if (fill_ < items_.size()) {
                ++fill_;
            }
        }
    }

    void Clear()
    {
        std::fill(std::begin(items_), std::end(items_), T{});
        next_ = 0;
        fill_ = 0;
    }

private:
    std::vector<T> items_;
    size_t next_;
    size_t fill_;
};

} // end namespace Tril

#endif // CIRCULARBUFFER_H
