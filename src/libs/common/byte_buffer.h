
#pragma once
#include "common/common.h"
#include "log/log.h"
	
namespace P2pClouds {

    class ByteBuffer
    {
	public:
		class Exception
		{
		public:
			Exception(bool _add, size_t _pos, size_t _esize, size_t _size)
				: _m_add(_add), _m_pos(_pos), _m_esize(_esize), _m_size(_size)
			{
				PrintPosError();
			}

			void PrintPosError() const
			{
				LOG_ERROR("Attempted to {} in ByteBuffer (pos:{}  size: {}).\n",
					(_m_add ? "put" : "get"), _m_pos, _m_size);
			}

		private:
			bool 		_m_add;
			size_t 		_m_pos;
			size_t 		_m_esize;
			size_t 		_m_size;
		};

    public:
        const static size_t DEFAULT_SIZE = 0x100;
        const static size_t MAX_SIZE = 10000000;

        ByteBuffer(): 
            rpos_(0), wpos_(0)
        {
            data_.reserve(DEFAULT_SIZE);
        }

        ByteBuffer(size_t res): 
            rpos_(0), wpos_(0)
        {
            if(res > 0)
            data_.reserve(res);
        }

        ByteBuffer(const ByteBuffer &buf): 
        rpos_(buf.rpos_), wpos_(buf.wpos_), data_(buf.data_) 
        { }
        
        ~ByteBuffer()
        {
            clear(true);
        }
        
        void clear(bool clearData)
        {
            if(clearData)
            data_.clear();

            rpos_ = wpos_ = 0;
        }

        template <typename T> void append(T value)
        {
            EndianConvert(value);
            append((uint8 *)&value, sizeof(value));
        }

        template <typename T> void put(size_t pos,T value)
        {
            EndianConvert(value);
            put(pos,(uint8 *)&value,sizeof(value));
        }
        
        void swap(ByteBuffer & s)
        {
            size_t rpos = s.rpos(), wpos = s.wpos();
            std::swap(data_, s.data_);
            s.rpos((int)rpos_);
            s.wpos((int)wpos_);
            rpos_ = rpos;
            wpos_ = wpos;
        }

        ByteBuffer &operator<<(uint8 value)
        {
            append<uint8>(value);
            return *this;
        }

        ByteBuffer &operator<<(uint16 value)
        {
            append<uint16>(value);
            return *this;
        }

        ByteBuffer &operator<<(uint32 value)
        {
            append<uint32>(value);
            return *this;
        }

        ByteBuffer &operator<<(uint64 value)
        {
            append<uint64>(value);
            return *this;
        }

        ByteBuffer &operator<<(int8 value)
        {
            append<int8>(value);
            return *this;
        }

        ByteBuffer &operator<<(int16 value)
        {
            append<int16>(value);
            return *this;
        }

        ByteBuffer &operator<<(int32 value)
        {
            append<int32>(value);
            return *this;
        }

        ByteBuffer &operator<<(int64 value)
        {
            append<int64>(value);
            return *this;
        }

        ByteBuffer &operator<<(float value)
        {
            append<float>(value);
            return *this;
        }

        ByteBuffer &operator<<(double value)
        {
            append<double>(value);
            return *this;
        }

        ByteBuffer &operator<<(const std::string &value)
        {
            append((uint8 const *)value.c_str(), value.length());
            append((uint8)0);
            return *this;
        }

        ByteBuffer &operator<<(const char *str)
        {
            append((uint8 const *)str, str ? strlen(str) : 0);
            append((uint8)0);
            return *this;
        }

        ByteBuffer &operator<<(bool value)
        {
            append<int8>(value);
            return *this;
        }

        ByteBuffer &operator>>(bool &value)
        {
            value = read<char>() > 0 ? true : false;
            return *this;
        }

        ByteBuffer &operator>>(uint8 &value)
        {
            value = read<uint8>();
            return *this;
        }

        ByteBuffer &operator>>(uint16 &value)
        {
            value = read<uint16>();
            return *this;
        }

        ByteBuffer &operator>>(uint32 &value)
        {
            value = read<uint32>();
            return *this;
        }

        ByteBuffer &operator>>(uint64 &value)
        {
            value = read<uint64>();
            return *this;
        }

        ByteBuffer &operator>>(int8 &value)
        {
            value = read<int8>();
            return *this;
        }

        ByteBuffer &operator>>(int16 &value)
        {
            value = read<int16>();
            return *this;
        }

        ByteBuffer &operator>>(int32 &value)
        {
            value = read<int32>();
            return *this;
        }

        ByteBuffer &operator>>(int64 &value)
        {
            value = read<int64>();
            return *this;
        }

        ByteBuffer &operator>>(float &value)
        {
            value = read<float>();
            return *this;
        }

        ByteBuffer &operator>>(double &value)
        {
            value = read<double>();
            return *this;
        }

        ByteBuffer &operator>>(std::string& value)
        {
            value.clear();
            while (length() > 0)
            {
                char c = read<char>();
                if (c == 0 || !isascii(c))
                    break;

                value += c;
            }
            
            return *this;
        }

        ByteBuffer &operator>>(char *value)
        {
            while (length() > 0)
            {
                char c = read<char>();
                if (c == 0 || !isascii(c))
                    break;

                *(value++) = c;
            }

            *value = '\0';
            return *this;
        }

        uint8 operator[](size_t pos) const
        {
            return read<uint8>(pos);
        }

        size_t rpos() const { return rpos_; }

        size_t rpos(int rpos)
        {
            if(rpos < 0)
                rpos = 0;

            rpos_ = rpos;
            return rpos_;
        }

        size_t wpos() const { return wpos_; }

        size_t wpos(int wpos)
        {
            if(wpos < 0)
                wpos = 0;

            wpos_ = wpos;
            return wpos_;
        }

        template<typename T>
        void read_skip() { read_skip(sizeof(T)); }

        void read_skip(size_t skip)
        {
            if(skip > length())
                throw ByteBuffer::Exception(false, rpos_, skip, length());

            rpos_ += skip;
        }

        template <typename T> T read()
        {
            T r = read<T>(rpos_);
            rpos_ += sizeof(T);
            return r;
        }

        template <typename T> T read(size_t pos) const
        {
            if(sizeof(T) > length())
                throw ByteBuffer::Exception(false, pos, sizeof(T), length());

            T val = *((T const*)&data_[pos]);
            EndianConvert(val);
            return val;
        }

        void read(uint8 *dest, size_t len)
        {
            if(len > length())
            throw ByteBuffer::Exception(false, rpos_, len, length());

            memcpy(dest, &data_[rpos_], len);
            rpos_ += len;
        }

        uint8 *data() { return &data_[0]; }
        const uint8 *data() const { return &data_[0]; }
        
        size_t size() const { return data_.size(); }
        bool empty() const { return data_.empty(); }
        size_t length() const { return rpos() >= wpos() ? 0 : wpos() - rpos(); }
        size_t space() const { return wpos() >= size() ? 0 : size() - wpos(); }

        void done() { read_skip(length()); }

        void resize(size_t newsize)
        {
            assert(newsize <= 1310700);
            data_.resize(newsize);
            rpos_ = 0;
            wpos_ = size();
        }

        void data_resize(size_t newsize)
        {
			assert(newsize <= 1310700);
            data_.resize(newsize);
        }

        void reserve(size_t ressize)
        {
			assert(ressize <= 1310700);

            if (ressize > size())
                data_.reserve(ressize);
        }

        void append(const std::string& str)
        {
            append((uint8 const*)str.c_str(), str.size() + 1);
        }

        void append(const char *src, size_t cnt)
        {
            return append((const uint8 *)src, cnt);
        }

        template<class T> void append(const T *src, size_t cnt)
        {
            return append((const uint8 *)src, cnt * sizeof(T));
        }

        void append(const uint8 *src, size_t cnt)
        {
            if (!cnt)
                return;

            assert(size() < MAX_SIZE);

            if (data_.size() < wpos_ + cnt)
                data_.resize(wpos_ + cnt);

            memcpy(&data_[wpos_], src, cnt);
            wpos_ += cnt;
        }

        void append(const ByteBuffer& buffer)
        {
            if(buffer.wpos()){
                append(buffer.data() + buffer.rpos(), buffer.length());
            }
        }

        void insert(size_t pos, const uint8 *src, size_t cnt)
        {
            data_.insert(data_.begin() + pos, cnt, 0);
            memcpy(&data_[pos], src, cnt);
            wpos_ += cnt;
        }

        void put(size_t pos, const uint8 *src, size_t cnt)
        {
            if(pos + cnt > size())
            throw ByteBuffer::Exception(true, pos, cnt, size());

            memcpy(&data_[pos], src, cnt);
        }

        void print_storage() const
        {
            char buf[1024];
            std::string fbuffer;
            size_t trpos = rpos_;

            p2pclouds_snprintf(buf, 1024, "STORAGE_SIZE: %lu, rpos=%lu.\n", (unsigned long)wpos(), (unsigned long)rpos());
            fbuffer += buf;

            for(size_t i = rpos(); i < wpos(); ++i)
            {
                p2pclouds_snprintf(buf, 1024, "%u ", read<uint8>(i));
                fbuffer += buf;
            }

            fbuffer += " \n";
            LOG_DEBUG(fbuffer.c_str());

            rpos_ = trpos;
        }

        void textlike() const
        {
            char buf[1024];
            std::string fbuffer;
            size_t trpos = rpos_;

            p2pclouds_snprintf(buf, 1024, "STORAGE_SIZE: %lu, rpos=%lu.\n", (unsigned long)wpos(), (unsigned long)rpos());
            fbuffer += buf;

            for (size_t i = rpos(); i < wpos(); ++i)
            {
                p2pclouds_snprintf(buf, 1024, "%c", read<uint8>(i));
                fbuffer += buf;
            }

            fbuffer += " \n";
			LOG_DEBUG(fbuffer.c_str());

            rpos_ = trpos;
        }

        void hexlike() const
        {
            uint32 j = 1, k = 1;
            char buf[1024];
            std::string fbuffer;
            size_t trpos = rpos_;

            p2pclouds_snprintf(buf, 1024, "STORAGE_SIZE: %lu, rpos=%lu.\n", (unsigned long)wpos(), (unsigned long)rpos());
            fbuffer += buf;
            
            uint32 i = 0;
            for (size_t idx = rpos(); idx < wpos(); ++idx)
            {
                ++i;
                if ((i == (j * 8)) && ((i != (k * 16))))
                {
                    if (read<uint8>(idx) < 0x10)
                    {
                        p2pclouds_snprintf(buf, 1024, "| 0%X ", read<uint8>(idx));
                        fbuffer += buf;
                    }
                    else
                    {
                        p2pclouds_snprintf(buf, 1024, "| %X ", read<uint8>(idx));
                        fbuffer += buf;
                    }
                    ++j;
                }
                else if (i == (k * 16))
                {
                    if (read<uint8>(idx) < 0x10)
                    {
                        p2pclouds_snprintf(buf, 1024, "\n0%X ", read<uint8>(idx));
                        fbuffer += buf;
                    }
                    else
                    {
                        p2pclouds_snprintf(buf, 1024, "\n%X ", read<uint8>(idx));
                        fbuffer += buf;
                    }

                    ++k;
                    ++j;
                }
                else
                {
                    if (read<uint8>(idx) < 0x10)
                    {
                        p2pclouds_snprintf(buf, 1024, "0%X ", read<uint8>(idx));
                        fbuffer += buf;
                    }
                    else
                    {
                        p2pclouds_snprintf(buf, 1024, "%X ", read<uint8>(idx));
                        fbuffer += buf;
                    }
                }
            }

            fbuffer += "\n";

			LOG_DEBUG(fbuffer.c_str());

            rpos_ = trpos;
        }

    protected:
        mutable size_t rpos_, wpos_;
        std::vector<uint8> data_;
    };


    template <typename T>
    inline ByteBuffer &operator<<(ByteBuffer &b, std::vector<T> v)
    {
        uint32 vsize = v.size();
        b << vsize;

        for (typename std::vector<T>::iterator i = v.begin(); i != v.end(); ++i)
        {
            b << *i;
        }

        return b;
    }

    template <typename T>
    inline ByteBuffer &operator>>(ByteBuffer &b, std::vector<T> &v)
    {
        uint32 vsize;
        b >> vsize;
        v.clear();

        while(vsize--)
        {
            T t;
            b >> t;
            v.push_back(t);
        }

        return b;
    }

    template <typename T>
    inline ByteBuffer &operator<<(ByteBuffer &b, std::list<T> v)
    {
		uint32 vsize = v.size();
        b << vsize;

        for (typename std::list<T>::iterator i = v.begin(); i != v.end(); ++i)
        {
            b << *i;
        }

        return b;
    }

    template <typename T>
    inline ByteBuffer &operator>>(ByteBuffer &b, std::list<T> &v)
    {
		uint32 vsize;
        b >> vsize;
        v.clear();

        while(vsize--)
        {
            T t;
            b >> t;
            v.push_back(t);
        }

        return b;
    }

    template <typename K, typename V>
    inline ByteBuffer &operator<<(ByteBuffer &b, std::map<K, V> &m)
    {
		uint32 vsize = m.size();
        b << vsize;

        for (typename std::map<K, V>::iterator i = m.begin(); i != m.end(); ++i)
        {
            b << i->first << i->second;
        }

        return b;
    }

    template <typename K, typename V>
    inline ByteBuffer &operator>>(ByteBuffer &b, std::map<K, V> &m)
    {
        uint32 msize;
        b >> msize;
        m.clear();

        while(msize--)
        {
            K k;
            V v;
            b >> k >> v;
            m.insert(make_pair(k, v));
        }

        return b;
    }

    template<>
    inline void ByteBuffer::read_skip<char*>()
    {
        std::string temp;
        *this >> temp;
    }

    template<>
    inline void ByteBuffer::read_skip<char const*>()
    {
        read_skip<char*>();
    }

    template<>
    inline void ByteBuffer::read_skip<std::string>()
    {
        read_skip<char*>();
    }

}

