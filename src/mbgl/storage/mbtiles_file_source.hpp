#pragma once

#include <mbgl/storage/file_source.hpp>

namespace mbgl {

namespace util {
template <typename T> class Thread;
} // namespace util

class MBTilesFileSource : public FileSource {
public:
	MBTilesFileSource();
	~MBTilesFileSource() override;

	std::unique_ptr<AsyncRequest> request(const Resource&, Callback) override;

	void setMBTilesPath(const std::string& t);

private:
	class Impl;
	std::unique_ptr<util::Thread<Impl>> impl;
};
    
} // namespace mbgl
