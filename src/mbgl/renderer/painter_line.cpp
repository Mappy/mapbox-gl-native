#include <mbgl/renderer/painter.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/line_bucket.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/style/layers/line_layer.hpp>
#include <mbgl/style/layers/line_layer_impl.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/programs/line_program.hpp>
#include <mbgl/sprite/sprite_atlas.hpp>
#include <mbgl/geometry/line_atlas.hpp>

namespace mbgl {

using namespace style;

void Painter::renderLine(PaintParameters& parameters,
                         LineBucket& bucket,
                         const LineLayer& layer,
                         const RenderTile& tile) {
    if (pass == RenderPass::Opaque) {
        return;
    }

    const LinePaintProperties::Evaluated& properties = layer.impl->paint.evaluated;

    auto draw = [&] (auto& program, auto&& uniformValues, auto& lineProperties) {
        program.draw(
            context,
            gl::Triangles(),
            depthModeForSublayer(0, gl::DepthMode::ReadOnly),
            stencilModeForClipping(tile.clip),
            colorModeForRenderPass(),
            std::move(uniformValues),
            *bucket.vertexBuffer,
            *bucket.indexBuffer,
            bucket.segments,
            bucket.paintPropertyBinders.at(layer.getID()),
            lineProperties,
            state.getZoom()
        );
    };

    if (!properties.get<LineDasharray>().from.empty()) {
        const LinePatternCap cap = bucket.layout.get<LineCap>() == LineCapType::Round
            ? LinePatternCap::Round : LinePatternCap::Square;
        LinePatternPos posA = lineAtlas->getDashPosition(properties.get<LineDasharray>().from, cap);
        LinePatternPos posB = lineAtlas->getDashPosition(properties.get<LineDasharray>().to, cap);

        lineAtlas->bind(context, 0);

        draw(parameters.programs.lineSDF,
             LineSDFProgram::uniformValues(
                 properties,
                 frame.pixelRatio,
                 tile,
                 state,
                 pixelsToGLUnits,
                 posA,
                 posB,
                 layer.impl->dashLineWidth,
                 lineAtlas->getSize().width),
             properties);

    } else if (!properties.get<LinePattern>().from.empty()) {
        optional<SpriteAtlasElement> posA = spriteAtlas->getPattern(properties.get<LinePattern>().from);
        optional<SpriteAtlasElement> posB = spriteAtlas->getPattern(properties.get<LinePattern>().to);

        if (!posA || !posB)
            return;

        spriteAtlas->bind(true, context, 0);

        draw(parameters.programs.linePattern,
             LinePatternProgram::uniformValues(
                 properties,
                 tile,
                 state,
                 pixelsToGLUnits,
                 *posA,
                 *posB),
             properties);

    } else {
		// Mappy specific drawing on paths
		if (layer.impl->isMappyPath == true) {
            const LinePaintProperties::Evaluated& mappyProperties = layer.impl->mappyPaint.evaluated;
            draw(parameters.programs.line,
                 LineProgram::uniformValues(
                                            mappyProperties,
                                            tile,
                                            state,
                                            pixelsToGLUnits),
                 mappyProperties);
		}

        draw(parameters.programs.line,
             LineProgram::uniformValues(
                                        properties,
                                        tile,
                                        state,
                                        pixelsToGLUnits),
             properties);
    }
}

} // namespace mbgl
