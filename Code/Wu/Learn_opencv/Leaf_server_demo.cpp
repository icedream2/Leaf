#include "Precompile.h"
#include "Leaf_identifier.h"
#include <boost/asio.hpp>
#include <memory>
#include <numeric>

using namespace boost::asio;
using boost::asio::ip::tcp;
namespace asio = boost::asio;

namespace std {
std::ostream& operator<<(
    std::ostream& out, 
    const Leaf::Leaf_identifier::feature_vector& v
    )
{
    std::for_each(v.begin(), v.end(), [&](float f){
        out << "\t" << f;
    });
    return out;
}
}

int main()
try {
    Tcp_server server;
    server.run();
    return 0;
}
catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return -1;
}