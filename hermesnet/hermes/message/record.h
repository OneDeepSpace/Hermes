#ifndef NET_RECORD_H
#define NET_RECORD_H

#include <cstdint>
#include <vector>
#include <string>
#include <future>
#include <algorithm>
#include <fstream>
#include <iomanip>

namespace filestorage::record
{
    using byte_t = std::uint8_t;
    using record_buffer_t = std::vector<byte_t>;

    // 16 байт
    struct record_info_t
    {
        std::uint32_t hash          {0};    // хэш от размера полезной нагрузки
        std::uint32_t payload_size  {0};    // размер полезной нагрузки, в байтах
        std::uint32_t record_size   {0};    // размер всей записи(sizeof(record_info_t)+payload.size()), в байтах
        std::uint32_t io_flag       {0};    // флаг для проверки успешной операции записи/чтения

        // запись структуры record_info_t в выходной поток
        friend std::ofstream& operator << (std::ofstream& stream, record_info_t& info) {

            stream.write(reinterpret_cast<const char *>(&info), sizeof(record_info_t));
            stream.flush();

            return stream;
        }

        // чтение структуры record_info_t из входного потока
        friend std::ifstream& operator >> (std::ifstream& stream, record_info_t& info) {

            stream.read(reinterpret_cast<char *>(&info), sizeof(record_info_t));

            auto read_count = stream.gcount();

            // проверка операции чтения "заголовка". В случае успешного извлечения
            // байтов из потока ни один из ниже приведенных битов ошибок не должен
            // быть взведён, в противном случае - произошла ошибка во время чтения
            // байтов. В том числе, не должен быть достигнут конец файла, поскольку
            // после "информации" о записи следует часть с полезной нагрузкой.
            const bool io_check = !(stream.bad() && stream.fail() && stream.eof());
            info.io_flag = io_check ? 1 : 0;

            return stream;
        }
    };

    struct record_t
    {
        record_info_t   info;
        record_buffer_t payload;

        bool check_hash() noexcept {
            return true;
        }

        // запись "записи" record_t в выходной поток
        friend std::ofstream& operator << (std::ofstream& stream, record_t& rec) {

            auto write_record = [&stream, &rec]() -> bool {
                auto res = std::async(std::launch::async, [&]() -> bool {

                    if (!stream.good()) return false;
                    stream << rec.info;

                    auto data_ptr = reinterpret_cast<const char*>(rec.payload.data());
                    auto data_size = rec.payload.size() * sizeof(byte_t);
                    stream.write(data_ptr, data_size);
                    stream.flush();

                    return stream.good();
                });
                return res.get();
            };

            // если запись "записи" record_t прошла успешно -> флаги ошибок выходного потока
            // не были установлены, отразим это в поле io_flag:
            rec.info.io_flag = write_record();

            return stream;
        }

        // чтение "записи" record_t из входного потока
        friend std::ifstream& operator >> (std::ifstream& stream, record_t& rec) {

            std::size_t read_count {0};

            auto read_record = [&stream, &rec, &read_count]() -> bool {
                auto res = std::async(std::launch::async, [&]() -> bool {

                    if (!stream.good()) return false;
                    stream >> rec.info;

                    rec.payload.resize(rec.info.payload_size);

                    auto data_ptr = reinterpret_cast<char*>(rec.payload.data());
                    auto data_size = rec.info.payload_size * sizeof(byte_t);
                    stream.read(data_ptr, data_size);

                    read_count = stream.gcount();

                    return (stream.good() || stream.eof()) && read_count > 0;
                });
                return res.get();
            };

            // если чтение "записи" record_t прошло успешно, то все поля размеров и хэш от
            // размера полезной нагрузки совпадают с тем, что было в record_info_t;
            // отражаем это в поле io_flag:
            const bool read_check = read_record();
            const bool hash_check = read_check ? rec.check_hash() : false;
            const bool payload_size_check = rec.info.payload_size == rec.payload.size();
            const bool record_size_check = rec.info.record_size == sizeof(rec.info) + rec.payload.size();
            rec.info.io_flag = read_check && hash_check && payload_size_check && record_size_check;

            return stream;
        }
    };

    // fill_record
    void fill_record(record_t& record) {
        using std::begin;
        using std::end;

        record_buffer_t result;

        std::string payload {"Yoda said: Do or do not. There is no try."};
        std::copy(begin(payload), end(payload), std::back_inserter(result));

        record.payload.swap(result);
        record.info.hash = 137;
        record.info.payload_size = record.payload.size();
        record.info.record_size = sizeof(record_info_t) + record.payload.size();

        std::cout << "Check record - " << record.info.payload_size << " - " << record.info.record_size << std::endl;
    }

    // read
    bool read(std::string filename, record_t& record) {

        std::ifstream in(filename, std::ios::binary | std::ios::in);
        in >> record;
        in.close();

        return record.info.io_flag;
    }

    // write
    bool write(std::string filename, record_t& record) {

        std::ofstream out(filename, std::ios::binary | std::ios::out | std::ios::app);
        out << record;
        out.close();

        return record.info.io_flag;
    }

    void check_record_stuff() {

        const std::string filename {"data.bin"};

        {
            record_t record_out;
            fill_record(record_out);

            std::cout << "Try to save record info file " << std::quoted(filename) << ".. " << std::flush;
            const auto check = write(filename, record_out);
            std::cout << (check ? "success" : "failed") << std::endl;
        }
        {
            record_t record_in;
            constexpr std::size_t chunk_size {1024};
            record_in.payload.reserve(chunk_size);

            std::cout << "Try to restore record from file " << std::quoted(filename) << ".. " << std::flush;
            const auto check = read(filename, record_in);
            std::cout << (check ? "success" : "failed") << std::endl;
            if (check) {
                std::string msg {std::begin(record_in.payload), std::end(record_in.payload)};
                std::cout << "restored message is " << std::quoted(msg) << "\n";
            } else {
                std::cout << "ooops, something went wrong...\n";
            }
        }
    }
}

#endif //NET_RECORD_H
