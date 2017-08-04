#pragma once

#include "common.h"

#include <boost/filesystem/path.hpp>

namespace particle {

class MsgIndex {
public:
    MsgIndex(boost::filesystem::path srcFile, boost::filesystem::path destFile);

private:
    boost::filesystem::path srcFile_, destFile_;

    static std::string stripFmtStr(const std::string& fmt);
};

} // namespace particle

inline particle::MsgIndex::MsgIndex(boost::filesystem::path srcFile, boost::filesystem::path destFile) :
        srcFile_(std::move(srcFile)),
        destFile_(std::move(destFile)) {
}
