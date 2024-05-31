#include "render.hpp"
#include "bsl/log.hpp"

DEFINE_LOG_CATEGORY(Render)

namespace Render {

    GeometryBatch::GeometryBatch():
        m_Vertices(sf::PrimitiveType::Triangles),
        m_Buffer(sf::PrimitiveType::Triangles, sf::VertexBuffer::Usage::Stream)
    {}

    void GeometryBatch::Rect(const sf::FloatRect& rect, const sf::Color& color, const sf::Transform &transform) {
        sf::Vertex v1(transform.transformPoint({rect.left, rect.top}), color);
        sf::Vertex v2(transform.transformPoint({rect.left + rect.width, rect.top}), color);
        sf::Vertex v3(transform.transformPoint({rect.left + rect.width, rect.top + rect.height}), color);
        sf::Vertex v4(transform.transformPoint({rect.left, rect.top + rect.height}), color);

        m_Vertices.append(v1);
        m_Vertices.append(v2);
        m_Vertices.append(v3);
        m_Vertices.append(v1);
        m_Vertices.append(v3);
        m_Vertices.append(v4);
    }

    void GeometryBatch::Circle(const sf::Vector2f& center, float radius, const sf::Color& color, int pointCount) {
        float angleStep = 2 * 3.141592653589793f / pointCount;

        for (int i = 0; i < pointCount; ++i) {
            float angle1 = i * angleStep;
            float angle2 = (i + 1) * angleStep;

            sf::Vector2f point1({center.x + std::cos(angle1) * radius, center.y + std::sin(angle1) * radius});
            sf::Vector2f point2({center.x + std::cos(angle2) * radius, center.y + std::sin(angle2) * radius});

            sf::Vertex v1(center, color);
            sf::Vertex v2(point1, color);
            sf::Vertex v3(point2, color);

            m_Vertices.append(v1);
            m_Vertices.append(v2);
            m_Vertices.append(v3);
        }
    }

    void GeometryBatch::Clear() {
        m_Vertices.clear();
    }

    void GeometryBatch::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        if(!m_Vertices.getVertexCount())
            return;

        if (m_Buffer.getVertexCount() != m_Vertices.getVertexCount()){
            LogRender(Info, "BatchBuffer Resized");
            m_Buffer.create(m_Vertices.getVertexCount());
        }

		m_Buffer.update(&m_Vertices[0]);

        LogRender(Info, "Submited % vertices batch", m_Vertices.getVertexCount());
        states.transform *= getTransform();
        target.draw(m_Buffer, states);
    }

    std::size_t s_DrawcallsCount = 0;

	static sf::Font MakeFont(const char *path) {
		auto font = sf::Font::loadFromFile(path);
		assert(font.has_value());

		font.value().setSmooth(true);
		return std::move(font.value());
	}
	
	const sf::Font &GetFont(){
		static sf::Font font = MakeFont(R"(X:\User\Fonts\Montserrat\Montserrat-Medium.ttf)");
		return font;
	}
}