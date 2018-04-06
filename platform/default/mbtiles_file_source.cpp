#include <mbgl/storage/mbtiles_file_source.hpp>
#include <mbgl/storage/file_source_request.hpp>
#include <mbgl/storage/response.hpp>
#include <mbgl/util/compression.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/string.hpp>
#include <mbgl/util/thread.hpp>
#include <mbgl/util/url.hpp>
#include <mbgl/util/util.hpp>

#include "sqlite3.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>
#include <iostream>

namespace mbgl {

    class MBTilesFileSource::Impl {
    public:
        Impl(ActorRef<Impl>) {
            
        }

        void request(const Resource& resource, ActorRef<FileSourceRequest> req) {
            Response response;
            if ( ! resource.url.empty()) {
                //
                if (resource.kind == Resource::Kind::Tile) {
                    assert(resource.tileData);
                    response = getTile(*resource.tileData);
                }
            }

            // response.error = std::make_unique<Response::Error>(Response::Error::Reason::NotFound);
            //response.data = std::make_shared<std::string>(util::read_file(path));

            req.invoke(&FileSourceRequest::setResponse, response);
        }

        void setMBTilesPath(const std::string &t) {
            path = t;
            ensureDB();
        }

        void ensureDB() {
            try {
                connect(mapbox::sqlite::ReadOnly);
            } catch (mapbox::sqlite::Exception& ex) {
                Log::Error(Event::Database, "Unexpected error connecting to database: %s", ex.what());
//                throw;
            }
        }

        void connect(int flags) {
            db = std::make_unique<mapbox::sqlite::Database>(path.c_str(), flags);
            db->setBusyTimeout(Milliseconds::max());
        }

        Response getTile(const Resource::TileData& tile) {
            if (statement == nullptr) {
                // NSString *query = @"SELECT tile_data FROM tiles WHERE tile_column = ? AND tile_row = ? AND zoom_level = ?";

                // clang-format off
                statement = getStatement(
                                         "SELECT tile_data "
                                         "FROM tiles "
                                         "WHERE tile_column = ?1 "
                                         "AND tile_row = ?2 "
                                         "AND zoom_level = ?3 "
                                         );
                // clang-format on
            }
            else
            {
                statement->reset();
            }

            statement->bind(1, tile.x);
            statement->bind(2, flipY(tile.z, tile.y));
            statement->bind(3, tile.z);

            Response response;
            if (!statement->run()) {
                std::cout << "getTile: run failed " << (int32_t)tile.z << "/" << tile.x << "/" << tile.y << std::endl;
                response.noContent = true;
                return response;
            }

            uint64_t size = 0;
            
//            response.etag     = statement->get<optional<std::string>>(0);
//            response.expires  = statement->get<optional<Timestamp>>(1);
//            response.modified = statement->get<optional<Timestamp>>(2);

            optional<std::string> data = statement->get<optional<std::string>>(0);
            if (!data) {
                std::cout << "getTile: no data" << std::endl;
                response.noContent = true;
            } else {
                response.data = std::make_shared<std::string>(util::decompress_gzip(*data));
                size = response.data->length();
                std::cout << "getTile: success" << (int32_t)tile.z << "/" << tile.x << "/" << tile.y << std::endl;
            }
            
            return response;
        }

        std::unique_ptr<::mapbox::sqlite::Statement> getStatement(const char * sql) {
            return std::make_unique<mapbox::sqlite::Statement>(db->prepare(sql));
        }

        int32_t flipY(int32_t zoom, int32_t y) {
            return std::pow(2,zoom) - 1 - y;
        }

    private:
        std::string path;
        std::unique_ptr<::mapbox::sqlite::Database> db;
        std::unique_ptr<::mapbox::sqlite::Statement> statement;
    };

    MBTilesFileSource::MBTilesFileSource()
	: impl(std::make_unique<util::Thread<Impl>>("MBTilesFileSource")) {
    }

    MBTilesFileSource::~MBTilesFileSource() = default;

    std::unique_ptr<AsyncRequest> MBTilesFileSource::request(const Resource& resource, Callback callback) {
		auto req = std::make_unique<FileSourceRequest>(std::move(callback));

		impl->actor().invoke(&Impl::request, resource, req->actor());

		return std::move(req);
    }

    void MBTilesFileSource::setMBTilesPath(const std::string &t) {
        impl->actor().invoke(&Impl::setMBTilesPath, t);
    }

} // namespace mbgl
