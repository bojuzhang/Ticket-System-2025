#include <climits>
#include <fstream>
#include <string>

namespace sjtu {
    template <class T1, class T2> class pair {
    public:
        T1 first;
        T2 second;
        constexpr pair() : first(), second() {}
        pair(const pair &other) = default;
        pair(pair &&other) = default;
        pair(const T1 &x, const T2 &y) : first(x), second(y) {}
        template <class U1, class U2> pair(U1 &&x, U2 &&y) : first(x), second(y) {}
        template <class U1, class U2>
        pair(const pair<U1, U2> &other) : first(other.first), second(other.second) {}
        template <class U1, class U2>
        pair(pair<U1, U2> &&other) : first(other.first), second(other.second) {}
        pair &operator=(const pair &other) {
            first = other.first, second = other.second;
            return *this;
        }
        bool operator<(const pair &other) {
            if (first != other.first)
                return first < other.first;
            return second < other.second;
        }
        bool operator>(const pair &other) {
            if (first != other.first)
                return first > other.first;
            return second > other.second;
        }
        bool operator==(const pair &other) {
            return first == other.first && second == other.second;
        }
        bool operator<=(const pair &other) { return !((*this) > other); }
        bool operator>=(const pair &other) { return !((*this) < other); }
        bool operator!=(const pair &other) { return !((*this) == other); }
    };
    class exception {
    protected:
        const std::string variant = "";
        std::string detail = "";
    
    public:
        exception() {}
        exception(const exception &ec) : variant(ec.variant), detail(ec.detail) {}
        virtual std::string what() { return variant + " " + detail; }
    };
    class index_out_of_bound : public exception {
        /* __________________________ */
    };
    class runtime_error : public exception {
        /* __________________________ */
    };
    class invalid_iterator : public exception {
        /* __________________________ */
    };
    class container_is_empty : public exception {
        /* __________________________ */
    };
    
    template <class T, int max_size> class MyArray {
    protected:
        T a[max_size]{};
        int size_;
    
    public:
        MyArray() { size_ = 0; }
        MyArray(const T &);
        int size() const { return size_; }
    
        void push_back(const T &);
        int lower_bound(const T &);
        void Insert(int, const T &);
        void erase(int);
        T &operator[](int);
        const T &operator[](int) const;
        T &back() { return a[size_ - 1]; }
        const T &back() const { return a[size_ - 1]; }
        void clear() { size_ = 0; }
        bool empty() const { return size_ == 0; }
    
        MyArray &operator=(const MyArray &other) {
            for (int i = 0; i < max_size; i++)
                a[i] = other.a[i];
            size_ = other.size_;
            return *this;
        }
        MyArray(const MyArray &other) {
            for (int i = 0; i < max_size; i++)
                a[i] = other.a[i];
            size_ = other.size_;
        }
    };
    
    template <typename T, int max_size> MyArray<T, max_size>::MyArray(const T &v) {
        size_ = 1;
        a[0] = v;
    }
    template <typename T, int max_size>
    void MyArray<T, max_size>::push_back(const T &v) {
        a[size_++] = v;
    }
    
    template <typename T, int max_size> T &MyArray<T, max_size>::operator[](int pos) {
        return a[pos];
    }
    template <typename T, int max_size>
    const T &MyArray<T, max_size>::operator[](int pos) const {
        return a[pos];
    }
    template <typename T, int max_size>
    void MyArray<T, max_size>::Insert(int pos, const T &v) {
        size_++;
        for (int i = size_ - 1; i >= pos + 1; i--) {
            a[i] = a[i - 1];
        }
        a[pos] = v;
    }
    template <typename T, int max_size> void MyArray<T, max_size>::erase(int pos) {
        size_--;
        for (int i = pos; i < size_; i++) {
            a[i] = a[i + 1];
        }
    }
    template <typename T, int max_size>
    int MyArray<T, max_size>::lower_bound(const T &v) {
        int l = 0, r = size_, ans = r;
        while (l <= r) {
            int mid = (l + r) / 2;
            if (a[mid] >= v) {
                ans = mid;
                r = mid - 1;
            } else {
                l = mid + 1;
            }
        }
        return ans;
    }
    
    template <int max_size> class MyString : public MyArray<char, max_size> {
    public:
        using MyArray<char, max_size>::MyArray;
        bool operator<(const MyString<max_size> &other) const {
            auto len = std::min(this->size_, other.size_);
            for (int i = 0; i < len; i++) {
                if (this->a[i] != other.a[i]) {
                    return this->a[i] < other.a[i];
                }
            }
            return this->size_ < other.size_;
        }
        bool operator>(const MyString<max_size> &other) const {
            auto len = std::min(this->size_, other.size_);
            for (int i = 0; i < len; i++) {
                if (this->a[i] != other.a[i]) {
                    return this->a[i] > other.a[i];
                }
            }
            return this->size_ > other.size_;
        }
        bool operator==(const MyString<max_size> &other) const {
            auto len = std::min(this->size_, other.size_);
            if (this->size_ != other.size_) {
                return false;
            }
            for (int i = 0; i < len; i++) {
                if (this->a[i] != other.a[i]) {
                    return false;
                }
            }
            return true;
        }
        bool operator<=(const MyString<max_size> &other) const {
            return !((*this) > other);
        }
        bool operator>=(const MyString<max_size> &other) const {
            return !((*this) < other);
        }
        bool operator!=(const MyString<max_size> &other) const {
            return !((*this) == other);
        }
        MyString(const std::string &s) {
            this->size_ = s.size();
            for (int i = 0; i < this->size_; i++) {
                this->a[i] = s[i];
            }
        }
        friend std::ostream &operator<<(std::ostream &os, const MyString &v) {
            for (int i = 0; i < v.size_; i++) {
                os << v.a[i];
            }
            return os;
        }
        friend std::istream &operator>>(std::istream &is, const MyString &v) {
            std::string s;
            is >> s;
            v = MyString(s);
            return is;
        }
        std::string str() const {
            std::string res;
            for (int i = 0; i < this->size_; i++) {
                res += this->a[i];
            }
            return res;
        }
        operator std::string() const {
            std::string res;
            for (int i = 0; i < this->size_; i++) {
                res += this->a[i];
            }
            return res;
        }
    };
    
    /**
     * a data container like std::vector
     * store data in a successive memory and support random access.
     */
    template <typename T> class vector {
    public:
        /**
         * a type for actions of the elements of a vector, and you should write
         *   a class named const_iterator with same interfaces.
         */
        /**
         * you can see RandomAccessIterator at CppReference for help.
         */
        class const_iterator;
        class iterator {
            // The following code is written for the C++ type_traits library.
            // Type traits is a C++ feature for describing certain properties of a type.
            // For instance, for an iterator, iterator::value_type is the type that the
            // iterator points to.
            // STL algorithms and containers may use these type_traits (e.g. the
            // following typedef) to work properly. In particular, without the following
            // code,
            // @code{std::sort(iter, iter1);} would not compile.
            // See these websites for more information:
            // https://en.cppreference.com/w/cpp/header/type_traits
            // About value_type:
            // https://blog.csdn.net/u014299153/article/details/72419713 About
            // iterator_category: https://en.cppreference.com/w/cpp/iterator
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = T;
            using pointer = T *;
            using reference = T &;
            using iterator_category = std::output_iterator_tag;
    
        private:
            pointer ptr_;
            int idx_;
            friend class const_iterator;
    
        public:
            iterator() : ptr_(nullptr), idx_(0) {}
            iterator(pointer ptr, int idx) : ptr_(ptr), idx_(idx) {}
    
            /**
             * return a new iterator which pointer n-next elements
             * as well as operator-
             */
            iterator operator+(const int &n) const {
                iterator p = *this;
                p.idx_ += n;
                return p;
            }
            iterator operator-(const int &n) const {
                iterator p = *this;
                p.idx_ += n;
                return p;
            }
            // return the distance between two iterators,
            // if these two iterators point to different vectors, throw
            // invaild_iterator.
            int operator-(const iterator &rhs) const {
                if (ptr_ != rhs.ptr_) {
                    throw invalid_iterator();
                }
                return idx_ - rhs.idx_;
            }
            iterator &operator+=(const int &n) {
                idx_ += n;
                return *this;
            }
            iterator &operator-=(const int &n) {
                idx_ -= n;
                return *this;
            }
            /**
             * iter++
             */
            iterator operator++(int) {
                iterator p = *this;
                idx_++;
                return p;
            }
            /**
             * ++iter
             */
            iterator &operator++() {
                idx_++;
                return *this;
            }
            /**
             * iter--
             */
            iterator operator--(int) {
                iterator p = *this;
                idx_--;
                return p;
            }
            /**
             * --iter
             */
            iterator &operator--() {
                idx_--;
                return *this;
            }
            /**
             * *it
             */
            T &operator*() const { return ptr_[idx_]; }
            /**
             * a operator to check whether two iterators are same (pointing to the same
             * memory address).
             */
            bool operator==(const iterator &rhs) const {
                return ptr_ == rhs.ptr_ && idx_ == rhs.idx_;
            }
            bool operator==(const const_iterator &rhs) const {
                return ptr_ == rhs.ptr_ && idx_ == rhs.idx_;
            }
            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const { return !((*this) == rhs); }
            bool operator!=(const const_iterator &rhs) const {
                return !((*this) == rhs);
            }
        };
        /**
         * has same function as iterator, just for a const object.
         */
        class const_iterator {
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = const T;
            using pointer = const T *;
            using reference = const T &;
            using iterator_category = std::output_iterator_tag;
    
        private:
            const pointer ptr_;
            int idx_;
            friend class iterator;
    
        public:
            const_iterator() : ptr_(nullptr), idx_(0) {}
            const_iterator(const pointer ptr, int idx) : ptr_(ptr), idx_(idx) {}
            /**
             * return a new iterator which pointer n-next elements
             * as well as operator-
             */
            const_iterator operator+(const int &n) const {
                const_iterator p = *this;
                p.idx_ += n;
                return p;
            }
            const_iterator operator-(const int &n) const {
                const_iterator p = *this;
                p.idx_ += n;
                return p;
            }
            // return the distance between two iterators,
            // if these two iterators point to different vectors, throw
            // invaild_iterator.
            int operator-(const const_iterator &rhs) const {
                if (ptr_ != rhs.ptr_) {
                    throw invalid_iterator();
                }
                return idx_ - rhs.idx_;
            }
            const_iterator &operator+=(const int &n) {
                idx_ += n;
                return *this;
            }
            const_iterator &operator-=(const int &n) {
                idx_ -= n;
                return *this;
            }
            /**
             * iter++
             */
            const_iterator operator++(int) {
                const_iterator p = *this;
                idx_++;
                return p;
            }
            /**
             * ++iter
             */
            const_iterator &operator++() {
                idx_++;
                return *this;
            }
            /**
             * iter--
             */
            const_iterator operator--(int) {
                const_iterator p = *this;
                idx_--;
                return p;
            }
            /**
             * --iter
             */
            const_iterator &operator--() {
                idx_--;
                return *this;
            }
            /**
             * *it
             */
            const T &operator*() const { return ptr_[idx_]; }
            /**
             * a operator to check whether two iterators are same (pointing to the same
             * memory address).
             */
            bool operator==(const iterator &rhs) const {
                return ptr_ == rhs.ptr_ && idx_ == rhs.idx_;
            }
            bool operator==(const const_iterator &rhs) const {
                return ptr_ == rhs.ptr_ && idx_ == rhs.idx_;
            }
            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const { return !((*this) == rhs); }
            bool operator!=(const const_iterator &rhs) const {
                return !((*this) == rhs);
            }
        };
        /**
         * TODO Constructs
         * At least two: default constructor, copy constructor
         */
        vector() {}
        vector(const vector &other) { *this = other; }
        /**
         * TODO Destructor
         */
        ~vector() {
            for (int i = 0; i < size_; i++) {
                data_[i].~T();
            }
            free(data_);
        }
        /**
         * TODO Assignment operator
         */
        vector &operator=(const vector &other) {
            if (this == &other) {
                return *this;
            }
            for (int i = 0; i < size_; i++) {
                data_[i].~T();
            }
            free(data_);
            size_ = other.size_;
            capacity_ = other.capacity_;
            data_ = reinterpret_cast<T *>(malloc(capacity_ * sizeof(T)));
            for (int i = 0; i < size_; i++) {
                new (data_ + i) T(other.data_[i]);
            }
            return *this;
        }
        /**
         * assigns specified element with bounds checking
         * throw index_out_of_bound if pos is not in [0, size)
         */
        T &at(const int &pos) {
            if (pos >= size_) {
                throw index_out_of_bound();
            }
            return data_[pos];
        }
        const T &at(const int &pos) const {
            if (pos >= size_) {
                throw index_out_of_bound();
            }
            return data_[pos];
        }
        /**
         * assigns specified element with bounds checking
         * throw index_out_of_bound if pos is not in [0, size)
         * !!! Pay attentions
         *   In STL this operator does not check the boundary but I want you to do.
         */
        T &operator[](const int &pos) {
            if (pos >= size_) {
                throw index_out_of_bound();
            }
            return data_[pos];
        }
        const T &operator[](const int &pos) const {
            if (pos >= size_) {
                throw index_out_of_bound();
            }
            return data_[pos];
        }
        /**
         * access the first element.
         * throw container_is_empty if size == 0
         */
        const T &front() const {
            if (size_ == 0) {
                throw container_is_empty();
            }
            return data_[0];
        }
        /**
         * access the last element.
         * throw container_is_empty if size == 0
         */
        const T &back() const {
            if (size_ == 0) {
                throw container_is_empty();
            }
            return data_[size_ - 1];
        }
        /**
         * returns an iterator to the beginning.
         */
        iterator begin() { return iterator(data_, 0); }
        const_iterator begin() const { return const_iterator(data_, 0); }
        const_iterator cbegin() const {
            return const_iterator(const_cast<const T *>(data_), 0);
        }
        /**
         * returns an iterator to the end.
         */
        iterator end() { return iterator(data_, size_); }
        const_iterator end() const { return const_iterator(data_, size_); }
        const_iterator cend() const {
            return const_iterator(const_cast<typename const_iterator::pointer>(data_),
                                                        size_);
        }
        /**
         * checks whether the container is empty
         */
        bool empty() const { return size_ == 0; }
        /**
         * returns the number of elements
         */
        int size() const { return size_; }
        int capacity() const { return capacity_; }
        /**
         * clears the contents
         */
        void clear() {
            for (int i = 0; i < size_; i++) {
                data_[i].~T();
            }
            size_ = 0;
        }
        /**
         * inserts value before pos
         * returns an iterator pointing to the inserted value.
         */
        iterator Insert(iterator pos, const T &value) {
            return Insert(pos - begin(), value);
        }
        /**
         * inserts value at index ind.
         * after inserting, this->at(ind) == value
         * returns an iterator pointing to the inserted value.
         * throw index_out_of_bound if ind > size (in this situation ind can be size
         * because after inserting the size will increase 1.)
         */
        iterator Insert(const int &ind, const T &value) {
            if (ind > size_) {
                throw index_out_of_bound();
            }
            if (size_ == capacity_) {
                reserve(capacity_ ? capacity_ * 2 : 1);
            }
            size_++;
            for (int i = size_ - 1; i > ind; i--) {
                new (data_ + i) T(std::move(data_[i - 1]));
            }
            new (data_ + ind) T(value);
            return iterator(data_, ind);
        }
        /**
         * removes the element at pos.
         * return an iterator pointing to the following element.
         * If the iterator pos refers the last element, the end() iterator is
         * returned.
         */
        iterator erase(iterator pos) { return erase(pos - begin()); }
        /**
         * removes the element with index ind.
         * return an iterator pointing to the following element.
         * throw index_out_of_bound if ind >= size
         */
        iterator erase(const int &ind) {
            if (ind >= size_) {
                throw index_out_of_bound();
            }
            data_[ind].~T();
            size_--;
            for (int i = ind; i < size_; i++) {
                new (data_ + i) T(std::move(data_[i + 1]));
            }
            return iterator(data_, ind);
        }
        /**
         * adds an element to the end.
         */
        void push_back(const T &value) {
            if (size_ == capacity_) {
                reserve(capacity_ ? capacity_ * 2 : 1);
            }
            new (data_ + size_) T(value);
            ++size_;
        }
        /**
         * remove the last element from the end.
         * throw container_is_empty if size() == 0
         */
        void pop_back() {
            if (!size_) {
                throw container_is_empty();
            }
            size_--;
            data_[size_].~T();
        }
    
    private:
        int size_ = 0;
        int capacity_ = 0;
    
        T *data_ = nullptr;
    
        void reserve(const int new_capacity) {
            capacity_ = new_capacity;
            T *new_data = reinterpret_cast<T *>(malloc(capacity_ * sizeof(T)));
            for (int i = 0; i < size_; i++) {
                new (new_data + i) T(std::move(data_[i]));
                data_[i].~T();
            }
            free(data_);
            data_ = new_data;
    
            // if (capacity_ < size_) {
            //     size_ = capacity_;
            // }
        }
    };
    
    template <class T, int info_len = 4> class MemoryRiver {
    private:
        /* your code here */
        const int sizeofsize_t = sizeof(int);
        std::fstream file;
        std::string file_name;
        int sizeofT = sizeof(T);
        int len_{};
    
    public:
        MemoryRiver() = default;
    
        MemoryRiver(const std::string &file_name) : file_name(file_name) {}
    
        ~MemoryRiver() {
            if (!file.is_open()) {
                file.open(file_name, std::ios::in | std::ios::out);
            }
            int t = len_;
            write_info(t, 1);
            file.close();
        }
    
        void initialise(std::string FN = "", bool clear_file = 0) {
            if (FN != "")
                file_name = FN;
            if (clear_file == 0) {
                file.open(file_name, std::ios::out);
                int tmp = 0;
                for (int i = 0; i < info_len; i++)
                    file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
            } else {
                file.open(file_name, std::ios::in);
                if (!file) {
                    file.open(file_name, std::ios::out);
                    int tmp = 0;
                    for (int i = 0; i < info_len; i++)
                        file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
                } else {
                    int t;
                    get_info(t, 1);
                    len_ = t;
                }
            }
            file.close();
        }
    
        // 读出第n个int的值赋给tmp，1_base
        void get_info(int &tmp, int n) {
            if (n > info_len)
                return;
            if (!file.is_open()) {
                file.open(file_name, std::ios::in | std::ios::out);
            }
            file.seekg((n - 1) * sizeofsize_t);
            file.read(reinterpret_cast<char *>(&tmp), sizeofsize_t);
        }
    
        // 将tmp写入第n个int的位置，1_base
        void write_info(int tmp, int n) {
            if (n > info_len)
                return;
            if (!file.is_open()) {
                file.open(file_name, std::ios::in | std::ios::out);
            }
            file.seekp((n - 1) * sizeofsize_t);
            file.write(reinterpret_cast<char *>(&tmp), sizeofsize_t);
        }
    
        // 在文件合适位置写入类对象t，并返回写入的位置索引index
        // 位置索引意味着当输入正确的位置索引index，在以下三个函数中都能顺利的找到目标对象进行操作
        // 位置索引index可以取为对象写入的起始位置
        int write(T &t) {
            if (!file.is_open()) {
                file.open(file_name, std::ios::in | std::ios::out);
            }
            int pos = info_len * sizeofsize_t + len_ * sizeofT;
            file.seekp(pos);
            file.write(reinterpret_cast<char *>(&t), sizeofT);
            ++len_;
            return len_ - 1;
        }
    
        // 用t的值更新位置索引index对应的对象，保证调用的index都是由write函数产生
        void update(T &t, const int index) {
            if (!file.is_open()) {
                file.open(file_name, std::ios::in | std::ios::out);
            }
            int pos = info_len * sizeofsize_t + index * sizeofT;
            file.seekp(pos);
            file.write(reinterpret_cast<char *>(&t), sizeofT);
        }
    
        // 读出位置索引index对应的T对象的值并赋值给t，保证调用的index都是由write函数产生
        void read(T &t, const int index) {
            if (!file.is_open()) {
                file.open(file_name, std::ios::in | std::ios::out);
            }
            int pos = info_len * sizeofsize_t + index * sizeofT;
            file.seekg(pos);
            file.read(reinterpret_cast<char *>(&t), sizeofT);
        }
    
        // //删除位置索引index对应的对象(不涉及空间回收时，可忽略此函数)，保证调用的index都是由write函数产生
        // void Delete(int index) {
        //     /* your code here */
        // }
    };
    
    } // namespace sjtu