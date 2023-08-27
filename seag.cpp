#include "seag.hpp"
#include "seag_atlas.hpp"

seag::Font::Font(const std::string& file_path, int glyph_size)
{
    if(!m_texture.loadFromFile(file_path))
    {
        SEAG_ERROR_LOG("Failed to load altas from file " + file_path);
        return;
    }

    m_glyph_size = glyph_size;
    init();
}

seag::Font::Font(const void* data, size_t size, int glyph_size)
{
    if(!m_texture.loadFromMemory(data, size))
    {
        SEAG_ERROR_LOG("Failed to load altas from memory 0x" + (uint64_t) data);
        return;
    }

    m_glyph_size = glyph_size;
    init();
}

seag::Font::~Font()
{
    if(m_glyphs)
        delete[] m_glyphs;
}

sf::Texture& seag::Font::getTexture()
{
    return m_texture;
}

sf::Image& seag::Font::getImage()
{
    return m_image;
}

seag::impl::Glyph seag::Font::getGlyph(char32_t character)
{
    return m_glyphs[character];
}

int seag::Font::getGlyphSize()
{
    return m_glyph_size;
}

void seag::Font::init()
{
    //Initialize SFML texture and image.
    m_texture.setSmooth(false);
    m_image = m_texture.copyToImage();
    
    //Process each glyphs in the atlas and calculate its boundaries.
    size_t glyphs_count = (m_texture.getSize().x * m_texture.getSize().y) / m_glyph_size;
    m_glyphs = new impl::Glyph[glyphs_count];
    char32_t character = 0;

    for(uint32_t y = 0; y < m_texture.getSize().y; y += m_glyph_size)
    {
        for(uint32_t x = 0; x < m_texture.getSize().x; x += m_glyph_size)
        {
            m_glyphs[character] = calculateGlyph(x, y);
            character++;
        }
    }
}

seag::impl::Glyph seag::Font::calculateGlyph(int x, int y)
{
    impl::Glyph glyph = { { x, y }, { 0, 0 }, { m_glyph_size/2.f, (float) m_glyph_size } };

    //Define the default bounds of the glyph.
    int minX = glyph.size.x, maxX = glyph.origin.x;
    int minY = glyph.size.y, maxY = glyph.origin.y;
    bool hasPixels = false;

    //Determine the bounds of the glyph (left, top, width, height).
    for(int ly = 0; ly < m_glyph_size; ly++)
    {
        for(int lx = 0; lx < m_glyph_size; lx++)
        {
            sf::Color color = m_image.getPixel(x + lx, y + ly);

            if(color == sf::Color::Black || color.a == 0)
                continue;

            hasPixels = true;

            if(minX > lx)
                minX = lx;
            else if(maxX < lx)
                maxX = lx;

            if(minY > ly)
                minY = ly;
            else if(maxY < ly)
                maxY = ly;
        }
    }

    if(hasPixels)
    {
        glyph.origin = { minX, minY };
        glyph.size = { (float) abs(maxX - minX) + SEAG_FONT_KERNING, (float) abs(maxY - minY) + 1 };
    }

    return glyph;
}

seag::Window::Window()
    : Window("SEAG " + std::string(SEAG_VERSION), 500, 200)
{
}

seag::Window::Window(const std::string& title, uint32_t width, uint32_t height)
    : m_window(sf::VideoMode(width, height), title),
    m_font(new Font(&SEAG_CP437_ATLAS, SEAG_CP437_ATLAS_SIZE, 10)),
    m_fontSize(11),
    m_cursor(0, 0),
    m_default_brush({sf::Color::White, sf::Color::Black, false, false}),
    m_active_brush(m_default_brush)
{
    m_window.setVerticalSyncEnabled(true);

    //TODO : Move shaders to const string to avoid carrying files around.
    if(!m_shader.loadFromMemory(impl::vertexShader, impl::fragmentShader))
    {
        SEAG_FATAL_LOG("Could not load rendering shader files.");
    }
    m_shader.setUniform("texture", sf::Shader::CurrentTexture);
}

seag::Window::~Window()
{
    if(m_font)
        delete m_font;
}

sf::RenderWindow& seag::Window::getNativeWindow()
{
    return m_window;
}

seag::Font* seag::Window::getFont()
{
    return m_font;
}

void seag::Window::setFont(Font* font)
{
    m_font = font;
}

int seag::Window::getFontSize()
{
    return m_fontSize;
}

void seag::Window::setFontSize(int fontSize)
{
    m_fontSize = fontSize;
}

sf::Vector2f seag::Window::getCellSize()
{
    return { (float) m_fontSize, m_fontSize * 2.f };
}

sf::Vector2f seag::Window::getCursor()
{
    return m_cursor;
}

void seag::Window::setCursor(sf::Vector2f cursor)
{
    m_cursor = cursor;
}

std::map<uint32_t, std::map<uint32_t, seag::impl::BufferCharacter>>& seag::Window::getScreenBuffer()
{
    return m_screenBuffer;
}

void seag::Window::setScreenBuffer(std::map<uint32_t, std::map<uint32_t, seag::impl::BufferCharacter>>& buffer)
{
    m_screenBuffer = buffer;
}

std::u32string seag::Window::getLine(uint32_t y)
{
    std::u32string line = U"";
    int previousX = -1;

    for(auto &[x,bufferChar] : m_screenBuffer.at(y))
    {
        if(previousX != -1 && x - previousX > 1)
            break;

        if(bufferChar.code == 0)
            break;

        line += bufferChar.code;
        previousX = x;

        if(bufferChar.code == U'\n')
            break;
    }

    return line;
}

std::u32string seag::Window::getText(uint32_t y)
{
    std::u32string line = U"";
    bool exit = false;
    int previousX = -1;

    do
    {
        for(auto &[x,bufferChar] : m_screenBuffer.at(y))
        {
            if(previousX != -1 && x - previousX > 1)
            {
                exit = true;
                break;
            }

            if(bufferChar.code == 0)
            {
                exit = true;
                break;
            }

            line += bufferChar.code;
            previousX = x;

            if(bufferChar.code == U'\n')
            {
                y++;
                break;
            }
        }
    }
    while(!exit);

    return line;
}

seag::impl::BufferCharacter seag::Window::getCharacter(uint32_t x, uint32_t y)
{
    return m_screenBuffer.at(y).at(x);
}

void seag::Window::print(std::u32string str)
{
    bool escaped = false;

    for (uint32_t i = 0; i < str.size(); i++)
    {
        if(str[i] == SEAG_STYLE_ESCAPE)
        {
            escaped = true;
            continue;
        }

        if(!escaped)
        {
            if(str[i] == SEAG_STYLE)
            {
                if(str.length() - i > 0)
                {
                    if(str[i+1] == SEAG_STYLE_RESET)
                        resetStyle();
                    else if(str[i+1] == SEAG_STYLE_BOLD)
                        pushBold(!m_active_brush.bold);
                    else if(str[i+1] == SEAG_STYLE_ITALIC)
                        pushItalic(!m_active_brush.italic);
                    else
                        goto After;

                    i++;
                    continue;
                }

                After:

                int codeLength = (str.length() - i > 6 && str[i+1] == SEAG_STYLE_BACKGROUND) ? 7 : 6;
                if(str.length() - i < codeLength)
                    continue;

                std::u32string c = str.substr(i + 1, codeLength);
                std::string code = std::string(c.begin(), c.end());

                if(code[0] == SEAG_STYLE_BACKGROUND)
                {
                    //Change active brush background color.
                    unsigned int hex;
                    std::stringstream ss;
                    ss << std::hex << (code.substr(1, code.length() - 1) + "FF");
                    ss >> hex;
                    pushBackgroundColor(sf::Color(hex));
                }
                else
                {
                    //Change active brush foreground color.
                    unsigned int hex;
                    std::stringstream ss;
                    ss << std::hex << (code + "FF");
                    ss >> hex;
                    pushForegroundColor(sf::Color(hex));
                }

                i += codeLength;
                continue;
            }
        }

        impl::BufferCharacter bufferCharacter = { str[i], m_active_brush };

        if(bufferCharacter.code == U'\t')
        {
            bufferCharacter = impl::BufferCharacter{ U' ', m_active_brush };
            for(int i = 0; i < 4; i++)
                m_screenBuffer[m_cursor.y][m_cursor.x+i] = bufferCharacter;
            m_cursor.x += 4;
        }
        else if(bufferCharacter.code == U'\n')
        {
            m_cursor = { 0, m_cursor.y + 1 };
        }
        else
        {
            m_screenBuffer[m_cursor.y][m_cursor.x] = bufferCharacter;
            m_cursor.x++;
        }

        escaped = false;
    }   
}

void seag::Window::print(std::u32string str, uint32_t y)
{
    print(str, 0, y);
}

void seag::Window::print(std::u32string str, uint32_t x, uint32_t y)
{
    m_cursor = { (float) x, (float) y };
    print(str);
}

void seag::Window::print(std::string str)
{
    print(std::u32string(str.begin(), str.end()));
}

void seag::Window::print(std::string str, uint32_t y)
{
    print(std::u32string(str.begin(), str.end()), y);
}

void seag::Window::print(std::string str, uint32_t x, uint32_t y)
{
    print(std::u32string(str.begin(), str.end()), x, y);
}

void seag::Window::setForegroundColor(sf::Color color)
{
    //Change active color to new color if it wasn't changed.
    if(m_active_brush.foregroundColor == m_default_brush.foregroundColor)
        m_active_brush.foregroundColor = color;

    m_default_brush.foregroundColor = color;
}

void seag::Window::setBackgroundColor(sf::Color color)
{
    //Change active color to new color if it wasn't changed.
    if(m_active_brush.backgroundColor == m_default_brush.backgroundColor)
        m_active_brush.backgroundColor = color;

    m_default_brush.backgroundColor = color;
}

void seag::Window::pushForegroundColor(sf::Color color)
{
    m_active_brush.foregroundColor = color;
}

void seag::Window::pushBackgroundColor(sf::Color color)
{
    m_active_brush.backgroundColor = color;
}

void seag::Window::pushBold(bool bold)
{
    m_active_brush.bold = bold;
}

void seag::Window::pushItalic(bool italic)
{
    m_active_brush.italic = italic;
}

void seag::Window::resetStyle()
{
    m_active_brush = m_default_brush;
}

bool seag::Window::isOpen()
{
    return m_window.isOpen();
}

bool seag::Window::waitEvent(sf::Event& event)
{
    return m_window.waitEvent(event);
}

bool seag::Window::pollEvent(sf::Event& event)
{
    return m_window.pollEvent(event);
}

void seag::Window::clear()
{
    //Clear SFML window.
    m_window.clear(m_default_brush.backgroundColor);

    //Clear screen buffer and reset cursor position.
    m_screenBuffer.clear();
    m_cursor = { 0, 0 };

    //Reset brush to default.
    m_active_brush = m_default_brush;
}

void seag::Window::display()
{
    sf::Vector2f cellSize = getCellSize();
    int offset = 0;

    for(auto &[y,value] : m_screenBuffer)
    {
        for(auto &[x,bufferChar] : value)
        {
            sf::Vector2f pos = { x * cellSize.x, y * cellSize.y };
            impl::Glyph glyph = m_font->getGlyph(bufferChar.code);

            sf::RectangleShape quad;
            float scale = (cellSize.x / (float) m_font->getGlyphSize())*2;
            quad.setSize({ glyph.size.x * scale, glyph.size.y * scale });
            quad.setTexture(&m_font->getTexture());
            quad.setTextureRect({ glyph.coords.x + glyph.origin.x, glyph.coords.y + glyph.origin.y, (int) glyph.size.x, (int) glyph.size.y });
            quad.setFillColor(bufferChar.brush.foregroundColor);

            sf::Vector2f quadPos = { 
                pos.x + cellSize.x/2 - quad.getGlobalBounds().width/2,
                pos.y + glyph.origin.y*scale
            };
            quad.setPosition(quadPos);

            sf::RectangleShape backgroundQuad;
            backgroundQuad.setSize(cellSize);
            backgroundQuad.setPosition(pos);
            backgroundQuad.setFillColor(bufferChar.brush.backgroundColor);

            m_window.draw(backgroundQuad);
            
            m_shader.setUniform("windowBackground", sf::Glsl::Vec4(m_default_brush.backgroundColor));
            m_shader.setUniform("foreground", sf::Glsl::Vec4(bufferChar.brush.foregroundColor));
            m_shader.setUniform("background", sf::Glsl::Vec4(bufferChar.brush.backgroundColor));
            // m_shader.setUniform("bold", bufferChar.brush.bold);
            m_shader.setUniform("italic", bufferChar.brush.italic);

            //Italic characters are handled in vertex shader.
            m_window.draw(quad, &m_shader);

            //If character is bold, we draw it again twice with a 1px offset
            if(bufferChar.brush.bold)
            {
                for(int i = 1; i < 3; i++)
                {
                    quad.setPosition(quad.getPosition() + sf::Vector2f(1, 0));
                    m_window.draw(quad, &m_shader);
                }
            }
        }
    }

    m_window.display();
}

void seag::Window::close()
{
    m_window.close();
}