#include <iostream>
#include <fstream>
#include <iterator>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;
namespace Fs = boost::filesystem;

void do_work(const std::string& p);

int main(int argc, const char* argv[])
try {
    // pre-condition
    if (argc != 2) {
        cout << "Usage: leaf <img-path>\n"
             << "Example:\n"
             << "\t leaf C:\\img.jpg\n";
        return 1;
    }

    if (!Fs::exists(argv[1])) {
        cerr << "File don't exist!\n";
        return 2;
    }

    do_work(argv[1]);

    return 0;
}
catch (std::exception& err) {
    cerr << err.what() << endl;
    return -1;
}

//-----------------------------------------------------------
std::string identify(std::istream& in)
{
    asio::io_service io;
    tcp::socket client(io);

    client.connect(tcp::endpoint(ip::address_v4::from_string("127.0.0.1"), 54577));

    // prepare vector string
    asio::streambuf buf;
    std::ostream out(&buf);
    out << "!"
        << in.rdbuf()
        << "\r\n\r\n"
        << "{ \"contour\": [] }"
        << "\r\n\r\n";

    asio::write(client, buf, asio::transfer_all());

    boost::system::error_code ec;
    asio::streambuf recv;
    asio::read_until(client, recv, "]", ec);
    if (ec == asio::error::eof) {
        throw std::runtime_error("Bad response");
    }

    std::istream resp_in(&recv);
    return std::string(
                std::istreambuf_iterator<char>{resp_in},
                std::istreambuf_iterator<char>{}
            );
}

void do_work(const std::string& p)
{
    std::cout << "Load image from " << p << std::endl;
    std::ifstream ifs(p, std::ios::binary);
    if (!ifs) {
        throw std::invalid_argument("Bad img path");
    }

    std::cout << "Result: " << identify(ifs) << std::endl;
}
//-------------------------------------------------------------------------
// End of File
//-------------------------------------------------------------------------
