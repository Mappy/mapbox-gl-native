#include <mbgl/renderer/painter.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/line_bucket.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/style/layers/line_layer.hpp>
#include <mbgl/style/layers/line_layer_impl.hpp>
#include <mbgl/style/paint_property.hpp>
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

    auto draw = [&] (auto& program, auto&& uniformValues) {
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
            properties,
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
                 lineAtlas->getSize().width));

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
                 *posB));

    } else {
		// Mappy specific drawing on paths
		if (properties.get<LineIsMappyPath>() == true)
		{
//			Color stroke_color = {1.0, 1.0, 1.0, opacity};
//			
//			context.program = lineShader.getID();
//			
//			lineShader.u_matrix = vtxMatrix;
//			lineShader.u_linewidth = (properties.lineWidth * 3.0f) / 4.0f;
//			lineShader.u_gapwidth = properties.lineGapWidth / 2;
//			lineShader.u_antialiasing = antialiasing / 2;
//			lineShader.u_ratio = ratio;
//			lineShader.u_blur = blur;
//			lineShader.u_extra = extra;
//			lineShader.u_offset = -properties.lineOffset;
//			lineShader.u_antialiasingmatrix = antialiasingMatrix;
//			
//			lineShader.u_color = stroke_color;
//			lineShader.u_opacity = opacity;
//			
//			setDepthSublayer(0);
//			bucket.drawLines(lineShader, context, paintMode());

            LinePaintProperties::Evaluated mappyEvaluated = properties;
            mappyEvaluated.


//            LinePaintProperties mappyProperties = LinePaintProperties(layer.impl->paint);
//            const optional<std::string>& klass = {};
//
//            auto width = properties.get<LineWidth>();
////            PropertyValue<float> mappyWidth = { width * 30.0f };
//            mappyProperties.set<LineWidth>(30.0, klass);
//
//            width = mappyProperties.evaluated.get<LineWidth>();
//
//            auto color = properties.get<LineColor>();
//            DataDrivenPropertyValue<Color> mappyWhite = { Color::white() };
//            mappyProperties.set<LineColor>(mappyWhite, klass);

            draw(parameters.programs.line,
                 LineProgram::uniformValues(
                                            mappyEvaluated,
                                            tile,
                                            state,
                                            pixelsToGLUnits));
		}

//        draw(parameters.programs.line,
//             LineProgram::uniformValues(
//                                        properties,
//                                        tile,
//                                        state,
//                                        pixelsToGLUnits));
    }
}

} // namespace mbgl
