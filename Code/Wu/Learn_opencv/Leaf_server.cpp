#include "Precompile.h"

#include "Leaf_identifier.h"
#include "Leaf_server.h"
#include "Leafparse.h"
#include "Fetcher.h"

#include <memory>
#include <numeric>
#include <sstream>
#include <iterator>
#include <opencv2/highgui/highgui.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

// Server Implementation
namespace Leaf { namespace Server {

namespace Detail {

std::map<int, std::string>
    init_map()
{
    namespace Fs = boost::filesystem;
    std::map<std::string, int> name_map;
    {
        assert(Fs::exists("name-map.map"));
        std::ifstream ifs("name-map.map");
        boost::archive::text_iarchive in(ifs);
        in >> name_map;
    }

    std::map<int, std::string> m;
    for (auto& v: name_map) {
        m.insert({ v.second, v.first });
    }

    return m;
}

const auto protocal_delim = "\r\n\r\n";
const auto img_map = init_map();

/*!
 * \brief   represent a complete task of recognition.
 */
class Tcp_connection:
    public std::enable_shared_from_this<Tcp_connection> {
public:
    typedef Connection_ptr pointer;

    static pointer create(io_service& io)
    {
        return pointer(new Tcp_connection(io));
    }

    tcp::socket& socket() { return socket_; }

    /*!
     * \brief   start work(read remote info).
     */
    void serve();

private:
    Tcp_connection(io_service& io)
        : socket_(io)
    {
    }

//-------------------------------------------------------------------
//! \name   read handlers
//! @{
private:
    void on_read_image_(const error_code& ec);

    void on_read_contour_(const error_code& ec);
//! @}
//------------------------------------------------------------------
//! \name   error handler
//! @{
private:
    void report_error_(const std::string& error);

//! @}
//------------------------------------------------------------------
//! \name   image process
//! @{
private:
    void recognize_image_();
//! @}

//------------------------------------------------------------------
//! \name   response
//! @{
private:
    void response_(const std::string& info);

    void report_result_(const Leaf_identifier::result_set& rs);

    // dummy function.
    // I don't care the result
    void on_response_end_(const error_code& ec) {}
//! @}

//-----------------------------------------------------------------
//! \name   representation
//! @{
private:
    tcp::socket socket_;
    std::string response_buf_;
    asio::streambuf request_buf_;

    cv::Mat img_;
    std::vector<cv::Point> contour_;
//! @}
};

}  // of namespace Leaf::Server::Detail

Tcp_server::Tcp_server()
    : io_(), acceptor_(io_, tcp::endpoint(tcp::v4(), 54577))
{
    identifier()->init("leaf.config");

    BOOST_LOG_TRIVIAL(trace) << "Init server";
}

void Tcp_server::run()
{
    BOOST_LOG_TRIVIAL(trace) << "Run server";
    start_listen_();
    io_.run();
}

void Tcp_server::start_listen_()
{
    auto con = Detail::Tcp_connection::create(io_);
    acceptor_.async_accept(
        con->socket(),
        boost::bind(&Tcp_server::handle_accept_, this, con, boost::asio::placeholders::error)
        );
}

void Tcp_server::handle_accept_(
    Detail::Connection_ptr con, const error_code& ec
    )
{
    if (!ec) {
        BOOST_LOG_TRIVIAL(trace) << "A new task is issued";
        con->serve();
    }

    start_listen_();
}

// Detail::Tcp_connection Implementation
namespace Detail {

cv::Mat read_image(std::istream& in)
{
    if (in.peek() != '!') return {};

    in.get();
    std::vector<uchar> buffer;
    std::string tmp;
    while (std::getline(in, tmp)) {
        auto last_byte = tmp[tmp.size() - 1];
        buffer.insert(buffer.end(), tmp.begin(), tmp.end());
        if (last_byte == '\r' && in.peek() == '\r') {
            //assert(in.peek() == '\r');
            in.get();   // consume '\r'
            in.get();   // consume '\n'
            break;
        }
        else {
            buffer.push_back('\n');
        }
    }

    return cv::imdecode(buffer, 1);
}

void Tcp_connection::serve()
{
    BOOST_LOG_TRIVIAL(trace) << "Read from remote";
    asio::async_read_until(
        socket_,
        request_buf_,
        protocal_delim,
        boost::bind(&Tcp_connection::on_read_image_, shared_from_this(), asio::placeholders::error)
        );
}

void Tcp_connection::on_read_image_(const error_code& ec)
{
    // check first
    if (ec) {
        report_error_("Bad image");
        return;
    }

    // retrieve image
    BOOST_LOG_TRIVIAL(trace) << "Get a image";
    std::istream img_stream(&request_buf_);
    img_ = read_image(img_stream);
    if (img_.empty()) {
        report_error_("Image format cannot be recognized");
        return;
    }

    // read contour
    BOOST_LOG_TRIVIAL(trace) << "Retrieve contour";
    asio::async_read_until(
        socket_,
        request_buf_,
        protocal_delim,
        boost::bind(&Tcp_connection::on_read_contour_, shared_from_this(), asio::placeholders::error)
        );
}

void Tcp_connection::on_read_contour_(const error_code& ec)
{
    assert(!img_.empty());
    // check first
    if (ec) {
        report_error_("Bad contour in reading");
        return;
    }

    // std::istream in(&request_buf_);
    //BOOST_LOG_TRIVIAL(trace) << "Get contour: " << &request_buf_;
    BOOST_LOG_TRIVIAL(trace) << "Get contour";
    using namespace boost::property_tree;
    std::istream in(&request_buf_);
    ptree contour;
    try {
        json_parser::read_json(in, contour);
        for (auto& elem: contour.get_child("contour")) {
            auto& value = elem.second;
            auto x = value.get<int>("x");
            auto y = value.get<int>("y");
            assert(x >= 0 && y >= 0);
            contour_.push_back({x, y});
        }
    }
    catch (ptree_error&) {
        report_error_("Bad json object of contour");
        return;
    }

    return recognize_image_();
}

void Tcp_connection::recognize_image_()
try {
    assert(!img_.empty());

    // retrieve feature vector
    BOOST_LOG_TRIVIAL(trace) << "Retrieve image feature";
    auto contour = limit_contour(img_, contour_);
    if (contour.empty()) {
        report_error_("Bad image or contour");
        return;
    }
    if (!is_leaf(img_, contour)) {
        report_error_("The image is not a leaf");
        return;
    }
    auto final_contour = refine_contour(img_, contour);
    auto fv = get_feature(img_, final_contour);

    // recognize
    BOOST_LOG_TRIVIAL(trace) << "Recognize feature vector";
    auto rs = identifier()->identify(
        Leaf_identifier::feature_vector(fv.begin(), fv.end())
        );

    // reprort result
    BOOST_LOG_TRIVIAL(trace) << "Report result";
    report_result_(rs);
}
catch (std::invalid_argument& e) {
    report_error_(e.what());
}
catch (std::exception&) {
    report_error_("Bad image or contour.");
}

void Tcp_connection::report_result_(const Leaf_identifier::result_set& rs)
{
    using result_entry = std::pair<Leaf_identifier::label, double>;
    std::vector<result_entry> ordered_rs(rs.begin(), rs.end());
    std::sort(
        ordered_rs.begin(),
        ordered_rs.end(),
        [](const result_entry& left, const result_entry& right){
            return left.second > right.second;
        }
    );

    std::ostringstream out;
    out << "[";
    int i = 0;
    for (auto& p: ordered_rs) {
        auto name = img_map.find(p.first)->second;
        auto prob = p.second;
        out << "{"
            << "\"name\":\"" << name << "\", "
            << "\"value\": " << prob
            << "}";
        if (++i < rs.size()) out << ",";
    }
    out << "]";

    BOOST_LOG_TRIVIAL(trace) << "Result: " << out.str();
    response_(out.str());
}

void Tcp_connection::report_error_(const std::string& error)
{
    response_("{ \"error\": \"" + error + "\"}");
}

void Tcp_connection::response_(const std::string& msg)
{
    response_buf_ = msg;
    asio::async_write(
            socket_,
            asio::buffer(response_buf_),
            boost::bind(&Tcp_connection::on_response_end_,
                        shared_from_this(),
                        asio::placeholders::error)
            );
}

}  // of namespace Leaf::Server::Detail

}} // of namespace Leaf::Server

//------------------------------------------------------
// End of File
//------------------------------------------------------
