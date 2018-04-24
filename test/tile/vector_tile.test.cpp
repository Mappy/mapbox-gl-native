#include <mbgl/test/util.hpp>
#include <mbgl/test/fake_file_source.hpp>
#include <mbgl/tile/vector_tile.hpp>
#include <mbgl/tile/tile_loader_impl.hpp>

#include <mbgl/util/default_thread_pool.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/map/transform.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/style/layers/symbol_layer.hpp>
#include <mbgl/renderer/tile_parameters.hpp>
#include <mbgl/renderer/buckets/symbol_bucket.hpp>
#include <mbgl/renderer/query.hpp>
#include <mbgl/geometry/feature_index.hpp>
#include <mbgl/annotation/annotation_manager.hpp>
#include <mbgl/renderer/image_manager.hpp>
#include <mbgl/text/glyph_manager.hpp>

#include <memory>

using namespace mbgl;

class VectorTileTest {
public:
    FakeFileSource fileSource;
    TransformState transformState;
    util::RunLoop loop;
    ThreadPool threadPool { 1 };
    style::Style style { loop, fileSource, 1 };
    AnnotationManager annotationManager { style };
    ImageManager imageManager;
    GlyphManager glyphManager { fileSource };
    Tileset tileset { { "https://example.com" }, { 0, 22 }, "none" };

    TileParameters tileParameters {
        1.0,
        MapDebugOptions(),
        transformState,
        threadPool,
        fileSource,
        MapMode::Continuous,
        annotationManager,
        imageManager,
        glyphManager,
        0
    };
};

TEST(VectorTile, setError) {
    VectorTileTest test;
    VectorTile tile(OverscaledTileID(0, 0, 0), "source", test.tileParameters, test.tileset);
    tile.setError(std::make_exception_ptr(std::runtime_error("test")));
    EXPECT_FALSE(tile.isRenderable());
    EXPECT_TRUE(tile.isLoaded());
    EXPECT_TRUE(tile.isComplete());
}

TEST(VectorTile, onError) {
    VectorTileTest test;
    VectorTile tile(OverscaledTileID(0, 0, 0), "source", test.tileParameters, test.tileset);
    tile.onError(std::make_exception_ptr(std::runtime_error("test")), 0);

    EXPECT_FALSE(tile.isRenderable());
    EXPECT_TRUE(tile.isLoaded());
    EXPECT_TRUE(tile.isComplete());
}

TEST(VectorTile, Issue8542) {
    VectorTileTest test;
    VectorTile tile(OverscaledTileID(0, 0, 0), "source", test.tileParameters, test.tileset);

    // Query before data is set
    std::vector<Feature> result;
    tile.querySourceFeatures(result, { { {"layer"} }, {} });
}
