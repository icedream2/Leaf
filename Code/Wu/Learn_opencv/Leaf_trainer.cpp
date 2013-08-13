#include "Precompile.h"
#include "Leaf_identifier.h"
#include "Leaf_util.h"
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <boost/filesystem.hpp>

namespace Fs = boost::filesystem;

std::multimap<
    Leaf::Leaf_identifier::label,
    Leaf::Leaf_identifier::feature_vector
    >
    load_data(const Fs::path& p);

void usage()
{
    std::cout
        << "Usage: leaf-trainer <folder>\n"
        << "\teg. leaf-trainer Samples/\n";
}

int main(int argc, char* argv[])
try {
    if (argc != 2) {
        usage();
        return 1;
    }
    if (!Fs::exists(argv[1])) {
        std::cerr << argv[1] << " do not exist\n";
        return 2;
    }

    auto dset = load_data(argv[1]);
    assert(!dset.empty());

    auto leaf = Leaf::identifier();
    leaf->init("leaf.config");
    leaf->train(Leaf::MM_view(dset));
    std::cout << "Training finished\n"
               << "Num of samples: " << dset.size() << std::endl;

    return 0;
}
catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return -1;
}
//--------------------------------------------------------------
std::vector<Leaf::Leaf_identifier::feature_vector>
    read_vectors(const Fs::path& p)
{
    assert(Fs::exists(p));
    std::ifstream in(p.string());
    assert(in);
    std::vector<Leaf::Leaf_identifier::feature_vector> fvs;
    std::string line;
    while (std::getline(in, line)) {
        Leaf::Leaf_identifier::feature_vector fv;
        std::istringstream iss(line);
        float f = 0.f;
        while (iss >> f) {
            fv.push_back(f);
        }
        if (fv.empty()) break;
        //if (fv.size() != 14) break;
        fvs.push_back(std::move(fv));
    }
    return fvs;
}

std::multimap<
    Leaf::Leaf_identifier::label,
    Leaf::Leaf_identifier::feature_vector
    >
    load_data(const Fs::path& dir)
{
    std::multimap<
        Leaf::Leaf_identifier::label,
        Leaf::Leaf_identifier::feature_vector
    > rs;
    std::map<std::string, Leaf::Leaf_identifier::label> name2id;
    assert(Fs::is_directory(dir));

    int id = 1;
    for (auto iter = Fs::directory_iterator(dir);
        iter != Fs::directory_iterator();
        ++iter) {
            auto& path = iter->path();
            if (path.extension() == ".txt") {
                auto label = 0;
                label = id++;
                name2id[path.stem().string()] = label;
                auto v = read_vectors(path);
                for (auto& f: v) {
                    rs.insert({label, f});
                }
            }
    }

    std::ofstream ofs("name-map.map");
    assert(ofs);
    boost::archive::text_oarchive out(ofs);
    out << name2id;

    return rs;
}
