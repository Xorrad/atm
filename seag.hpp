/****************************************************************************
*                                                                           *
*   SEAG : a Simple Engine for ASCII Game                                   *
*                                                                           *
*   Made by Xorrad <monsieurs.aymeric@gmail.com>                            *
*   This code is licensed under MIT license (see LICENSE for details)       *
*                                                                           *
****************************************************************************/

#ifndef SEAG_CPP
#define SEAG_CPP

#include <iostream>
#include <string>
#include <locale>
#include <codecvt>
#include <sstream>

#include <SFML/Graphics.hpp>

#define SEAG_VERSION "v0.1"

/* Define log macros. */
#define SEAG_LOG(msg) std::cout << "[INFO] " << __FILE__ << "(" << __LINE__ << "): " << msg << std::endl 
#define SEAG_ERROR_LOG(msg) std::cout << "[ERROR] " << __FILE__ << "(" << __LINE__ << "): " << msg << std::endl 
#define SEAG_FATAL_LOG(msg) std::cout << "[FATAL] " << __FILE__ << "(" << __LINE__ << "): " << msg << std::endl; exit(EXIT_FAILURE)

/* Define glyph rendering modifiers. */
#define SEAG_LINE_PADDING 0
#define SEAG_FONT_KERNING 1

/* Define tag characters for text style. */
#define SEAG_STYLE '#'
#define SEAG_STYLE_ESCAPE '/'
#define SEAG_STYLE_BACKGROUND '_'
#define SEAG_STYLE_RESET 'r'
#define SEAG_STYLE_BOLD 'b'
#define SEAG_STYLE_ITALIC 'i'

/* Classes and structures are declared in this scope. */
namespace seag
{
    /* Store drawing information like style and colors. */
    struct Brush
    {
        sf::Color foregroundColor;
        sf::Color backgroundColor;
        bool bold;
        bool italic;
    };

    /* Internal structures and classes. */
    namespace impl
    {
        const std::string vertexShader = \
            "uniform vec4 windowBackground;" \
            "uniform vec4 foreground;" \
            "uniform vec4 background;" \
            "uniform bool italic;" \
            "void main()" \
            "{" \
                "vec4 pos = gl_Vertex;" \
                "if(italic) pos.x -= pos.y * 0.3 - 0.0;" \
                "pos = gl_ModelViewProjectionMatrix * pos;" \
                "gl_Position = pos;" \
                "gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;" \
                "gl_FrontColor = gl_Color;" \
            "}";

        const std::string fragmentShader = \
            "uniform sampler2D texture;" \
            "uniform vec4 windowBackground;" \
            "uniform vec4 foreground;" \
            "uniform vec4 background;" \
            "uniform bool italic;" \
            "void main()" \
            "{" \
                "vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);" \
                "float t = (pixel.r + pixel.g + pixel.b)/3.0;" \
                "vec4 targetBackground = background;" \
                "vec4 targetForeground = foreground;" \
                "if(windowBackground == background) targetBackground.a = 0.0;" \
                "vec4 color = mix(targetBackground, targetForeground, t);" \
                "gl_FragColor = color;" \
            "}";

        /* Store data of a glyph like its coordinates in the atlas, origin and size. */
        struct Glyph
        {
            sf::Vector2i coords;
            sf::Vector2i origin;
            sf::Vector2f size;
        };

        /* Represent a character on the screen with its value (unicode) and its brush (color, style). */
        struct BufferCharacter
        {
            char32_t code;
            Brush brush;
        };
    }

    /* Used to load and process bitmap font atlas from file or memory. */
    class Font
    {
        public:
            Font(const std::string& file_path, int glyph_size); /* Create and load a font from a file. */
            Font(const void* data, size_t size, int glyph_size); /* Create and load a font from a memory address. */
            ~Font();

            sf::Texture& getTexture(); /* Get the SFML texture of the atlas (used to draw). */
            sf::Image& getImage(); /* Get the SFML image of the atlas (used to read/write pixels). */
            impl::Glyph getGlyph(char32_t character); /* Get the glyph data of a unicode character. */
            int getGlyphSize(); /* Get the size of the glyphs in the atlas texture. */

        private:
            sf::Image m_image;
            sf::Texture m_texture;
            impl::Glyph* m_glyphs;
            int m_glyph_size;

            void init(); /* Initialize the font: calculate all glyphs data. */
            impl::Glyph calculateGlyph(int x, int y); /* Calculate the boundaries of a glyph (used during initialization). */
    };

    /* Represent a graphical window where you can print characters. */
    class Window
    {
        public:
            Window(); /* Create a default window. */
            Window(const std::string& title, uint32_t width, uint32_t height); /* Create a window with custom title and size. */
            ~Window();

            sf::RenderWindow& getNativeWindow(); /* Get the SFML window. */

            Font* getFont(); /* Get the window font. */
            void setFont(Font* font); /* Change the window font. */

            int getFontSize(); /* Get the window font size. */
            void setFontSize(int fontSize); /* Change the window font size. */

            sf::Vector2f getCellSize(); /* Get the size of a character cell. */

            sf::Vector2f getCursor(); /* Get the window cursor position. */
            void setCursor(sf::Vector2f cursor); /* Change the window cursor position. */

            std::map<uint32_t, std::map<uint32_t, impl::BufferCharacter>>& getScreenBuffer(); /* Get the screen buffer (table of characters to draw). */
            void setScreenBuffer(std::map<uint32_t, std::map<uint32_t, impl::BufferCharacter>>& buffer); /* Change the screen buffer.*/

            std::u32string getLine(uint32_t y); /* Get a string from a line in the screen buffer (Stop at EOL character). */
            std::u32string getText(uint32_t y); /* Get a string from a line in the screen buffer. */
            impl::BufferCharacter getCharacter(uint32_t x, uint32_t y); /* Get a character in the screen buffer. */

            void print(std::u32string str); /* Print a UTF-32 string. */
            void print(std::u32string str, uint32_t y); /* Print a UTF-32 string at a specific line. */
            void print(std::u32string str, uint32_t x, uint32_t y); /* Print a UTF-32 string at a specific position. */
            
            void print(std::string str); /* Print a string. */
            void print(std::string str, uint32_t y); /* Print a string at a specific line. */
            void print(std::string str, uint32_t x, uint32_t y); /* Print a string at a specific position. */

            void setForegroundColor(sf::Color color); /* Set default text color */
            void setBackgroundColor(sf::Color color); /* Set default background color */

            void pushForegroundColor(sf::Color color); /* Set text color for next prints */
            void pushBackgroundColor(sf::Color color); /* Set background color for next prints */
            void pushBold(bool bold); /* Enable or disable bold style for next prints */
            void pushItalic(bool italic); /* Enable or disable italic style for next prints */

            void resetStyle(); /* Reset active colors and text style to default */

            bool isOpen(); /* Determine if the window is open. */

            bool waitEvent(sf::Event& event); /* Get SFML event (freeze execution). */
            bool pollEvent(sf::Event& event); /* Get SFML event. */

            void clear(); /* Clear the window and screen buffer. */
            void display(); /* Display the screen buffer to the window. */
            void close(); /* Close the window. */

        private:
            sf::RenderWindow m_window;

            Font *m_font;
            int m_fontSize;
            sf::Vector2f m_cursor;
            std::map<uint32_t, std::map<uint32_t, impl::BufferCharacter>> m_screenBuffer;
            
            sf::Shader m_shader;

            Brush m_default_brush;
            Brush m_active_brush;

    };
};

#endif
