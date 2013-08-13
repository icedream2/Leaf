#include "Precompile.h"
#include "Leaf_identifier.h"
#include <iterator>
#include <sstream>

std::multimap<
    Leaf::Leaf_identifier::label,
    Leaf::Leaf_identifier::feature_vector
    >
    load_data();

void test_all();

int main()
{
    test_all();
    return 0;
}

namespace Fs = boost::filesystem;
namespace Rng = boost::range;

void test_all()
try {
    auto test = load_data();

    int hit = 0;
    int partial_hit = 0;
    int miss = 0;
    int error = 0;
    auto leaf = Leaf::identifier();
    leaf->init("leaf.config");
    if (!leaf->is_ok()) {
        std::cerr << "error: classifiers have not been trained!\n";
        return;
    }

    for (auto iter = test.begin(); iter != test.end(); ++iter) {
        BOOST_LOG_TRIVIAL(debug) << "Test a new sample";
        auto ret = leaf->identify(iter->second);

        typedef decltype(*ret.begin())& value_ref;
        if (ret.empty()) {
            ++miss;
            BOOST_LOG_TRIVIAL(debug) << "No result";
        }
        else {
            auto max = std::max_element(ret.begin(), ret.end(), [](value_ref l, value_ref r){
                return l.second < r.second;
            });
            if (max->first == iter->first) ++hit;
            else if (Rng::count_if(ret, [&](value_ref v){ return v.first == iter->first; })) {
                ++partial_hit;
            }
            else {
                ++error;
                BOOST_LOG_TRIVIAL(warning) << "Failed to recogize";
            }
        }
        BOOST_LOG_TRIVIAL(debug) << "It should be " << iter->first;
    }

    auto size = test.size();
    BOOST_LOG_TRIVIAL(info) << "Total: " << size;
    BOOST_LOG_TRIVIAL(info) << "Hit: " << hit << " =>" << 100.f * hit / size << "%";
    BOOST_LOG_TRIVIAL(info) << "Partial: " << partial_hit << " =>" << 100.f * partial_hit / size << "%";
    BOOST_LOG_TRIVIAL(info) << "Can't: " << error << " =>" << 100.f * error / size << "%";
    BOOST_LOG_TRIVIAL(info) << "Miss: " << miss << " =>" << 100.f * miss / size << "%";
}
catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
}

//-----------------------------------------------------------
//-----------------------------------------------------------
std::multimap<
    Leaf::Leaf_identifier::label,
    Leaf::Leaf_identifier::feature_vector
    >
    load_data()
{
    using namespace boost::archive;

    std::ifstream ifs("leaf-samples.db");
    if (!ifs) {
        throw std::runtime_error("cannot load test samples");
    }

    text_iarchive in(ifs);
    std::multimap<
        Leaf::Leaf_identifier::label,
        Leaf::Leaf_identifier::feature_vector
        > rs;
    in >> rs;
    return rs;
}
//---------------------------------------------------------
// End of File
//---------------------------------------------------------
